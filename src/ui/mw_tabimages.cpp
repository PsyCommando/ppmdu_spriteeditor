#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>
#include <QSpinBox>
#include <QTimer>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/paletteeditor.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

bool IsImageResolutionValid( QSize imgres )
{
    for(const auto & supported : fmt::FrameResValues)
    {
        if( supported.first  == imgres.width() &&
            supported.second == imgres.height() )
            return true;
    }
    return false;
}

QPixmap ImageToPixmap( QImage && img, const QSize & sz )
{
    return qMove(QPixmap::fromImage(img, Qt::ImageConversionFlag::AvoidDither | Qt::ImageConversionFlag::ColorOnly).scaled( sz, Qt::KeepAspectRatio));
}



void MainWindow::DisplayImageListPage(Sprite *spr, ImageContainer *pimgs, Image *img)
{
    Q_ASSERT(spr && pimgs);
    qDebug() << "MainWindow::DisplayImageListPage(): Displaying images list page!\n";
    ui->tblviewImages->setModel(pimgs->getModel());
    qDebug() << "MainWindow::DisplayImageListPage(): Model set!\n";
    ShowATab(ui->tabImages);
    ui->tblviewImages->resizeRowsToContents();
    ui->tblviewImages->resizeColumnsToContents();

    //Map model's columns to some of the controls
    m_imgdatmapper.reset(new QDataWidgetMapper);
    m_imgdatmapper->setModel(pimgs->getModel());
    m_imgdatmapper->addMapping(ui->spbimgunk2,  static_cast<int>(Image::eColumnType::direct_Unk2) );
    m_imgdatmapper->addMapping(ui->spbimgunk14,  static_cast<int>(Image::eColumnType::direct_Unk14) );
    //m_imgdatmapper->toFirst();
    connect(ui->tblviewImages, &QTableView::clicked, m_imgdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);

    if(img)
    {
        //select specified image!
        //spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
        QModelIndex ind = ui->tblviewImages->model()->index(pimgs->indexOfNode(img), 0);
        ui->tblviewImages->setCurrentIndex(ind);
        on_tblviewImages_clicked(ind);
    }
}

// *********************************
//  Image Tab
// *********************************
void MainWindow::on_btnImageCrop_clicked()
{
    Image * pimg = static_cast<Image*>(ui->tv_sprcontent->currentIndex().internalPointer());

    if(pimg)
    {
        DiagSingleImgCropper cropper(this, pimg);
        cropper.setModal(true);
        cropper.show();
    }
    else
    {
        QApplication::beep();
        ui->statusBar->setStatusTip("Can't crop! No valid image selected!");
        qWarning("MainWindow::on_btnImageCrop_clicked(): Crop clicked, but no valid images was selected!");
    }
}

void MainWindow::on_tblviewImages_clicked(const QModelIndex &index)
{
    Image * img = static_cast<Image *>(index.internalPointer());
    if(!index.internalPointer() || !img)
    {
        ui->lbl_imgpreview->setPixmap(m_imgNoImg);
        return;
    }
    ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(img->parentSprite()->getPalette()), ui->lbl_imgpreview->size()));
}

void MainWindow::on_btnImagesExport_clicked()
{
    Image * pimg = nullptr;
    if(ui->tblviewImages->currentIndex().isValid())
        pimg = static_cast<Image*>( ui->tblviewImages->currentIndex().internalPointer() );

    if(!pimg)
    {
        ShowStatusErrorMessage(tr("Error: No image selected for export!"));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;

    QImage img = qMove( pimg->makeImage(pimg->parentSprite()->getPalette()) );

    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported image to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}


void MainWindow::on_btnImagesImport_clicked()
{
    //Get current selection if there is one
    Image * pimg = nullptr;
    if(!ui->tblviewImages->selectionModel()->selectedIndexes().empty())
    {
        pimg = static_cast<Image*>( ui->tblviewImages->selectionModel()->selectedIndexes().first().internalPointer() );
    }

    QString filename = QFileDialog::getOpenFileName(this,
                        tr("Import Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;

    //Load image
    QImage img(filename, "PNG");

    //Validate image
    if(img.format() != QImage::Format_Indexed8)
    {
        Warn("Invalid image format!", "This PNG image is not 8 bits per pixel (aka 256 colors). Make sure the image to import is saved as a PNG 8bpp, 256 colors!");
        return;
    }

    //Check if the resolution is one of the allowed ones.
    //Otherwise, warn the user
    QSize imgres = img.size();
    if(!IsImageResolutionValid(imgres))
    {
        QString desc = "Images imported this way must fit one of the supported resolution:\n";

        for(const auto & res : fmt::FrameResValues )
            desc+= QString("%1x%2\n").arg(res.first, res.second);

        Warn("Invalid image resolution!",
             desc);
        return;
    }

    //Convert the QImage to raw
    fmt::ImageDB::img_t newimgdata;
    newimgdata.unk2 = 0;
    newimgdata.unk14 = 0;
//    std::copy(img.bits(),
//              img.bits() + img.byteCount(),
//              std::back_inserter(newimgdata.data));

    //All image data is stored as 8bpp
    newimgdata.data = qMove(utils::ImgToRaw(img));

    //If we replace an existing image, or if we insert a new one!
    if(pimg)
            pimg->importImage8bpp(newimgdata, img.width(), img.height(), false);
    else
    {
        //Insert at the end of the list
        bool succ = ui->tblviewImages->model()->insertRow( ui->tblviewImages->model()->rowCount());

        if(succ)
        {
            QModelIndex newidx = ui->tblviewImages->model()->index(ui->tblviewImages->model()->rowCount(), 0);
            Image * newimg = static_cast<Image *>(newidx.internalPointer());
            Q_ASSERT(newimg);

            //Replace selected image
            newimg->importImage8bpp(newimgdata, img.width(), img.height(), false);
        }
        else
            Warn("Import Failed!","Couldn't create a new row at the end of the list! Qt refused!");
    }
}


void MainWindow::on_spbimgunk2_valueChanged(int arg1)
{
//    Image * pimg = this->currentImage();
//    if(pimg)
//        pimg->setUnk2(static_cast<uint32_t>(arg1));
}

void MainWindow::on_spbimgunk14_valueChanged(int arg1)
{
//    Image * pimg = this->currentImage();
//    if(pimg)
//        pimg->setUnk14(static_cast<uint16_t>(arg1));
}
