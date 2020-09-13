#include "tabimages.hpp"
#include "ui_tabimages.h"

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>

#include <src/ui/mainwindow.hpp>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/data/sprite/imagescontainer.hpp>


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

//#MOVEME: move to utilities!
QPixmap ImageToPixmap( QImage && img, const QSize & sz )
{
    return qMove(QPixmap::fromImage(img, Qt::ImageConversionFlag::AvoidDither | Qt::ImageConversionFlag::ColorOnly).scaled( sz, Qt::KeepAspectRatio));
}

//-----------------------------------------------------------------
TabImages::TabImages(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabImages)
{
    ui->setupUi(this);
}

TabImages::~TabImages()
{
    delete ui;
}


void TabImages::OnShowTab(Sprite *pspr, QPersistentModelIndex element)
{
    Q_ASSERT(pspr);

    ImageContainer * pimgs = &(pspr->getImages());

    qDebug() << "MainWindow::DisplayImageListPage(): Displaying images list page!\n";
    ui->tblviewImages->setModel(pimgs->getModel());
    qDebug() << "MainWindow::DisplayImageListPage(): Model set!\n";

    if(element.isValid())
        m_currentImage = element;
    else
    {
        //When we open the tab with an invalid model index, we just pick the first avaialable image
        // if there is an image to pick from at all
//        Image * pimg = pimgs->getImage(0);
//        if(pimg)
//            m_currentImage = pimg;
//        else
        m_currentImage = QModelIndex();
    }

    ui->spbimgunk2->setRange (0, std::numeric_limits<uint32_t>::max());
    ui->spbimgunk14->setRange(0, std::numeric_limits<uint16_t>::max());

    ui->tblviewImages->resizeRowsToContents();
    ui->tblviewImages->resizeColumnsToContents();

    //Map model's columns to some of the controls
    m_imgdatmapper.reset(new QDataWidgetMapper);
    m_imgdatmapper->setModel(pimgs->getModel());
    m_imgdatmapper->addMapping(ui->spbimgunk2,  static_cast<int>(Image::eColumnType::direct_Unk2) );
    m_imgdatmapper->addMapping(ui->spbimgunk14,  static_cast<int>(Image::eColumnType::direct_Unk14) );
    //m_imgdatmapper->toFirst();
    connect(ui->tblviewImages, &QTableView::clicked, m_imgdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);

    if(m_currentImage.isValid())
    {
        const Image * img = static_cast<Image*>(m_currentImage.internalPointer());
        if(img)
        {
            //select specified image!
            //spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
            //QModelIndex ind = ui->tblviewImages->model()->index(pimgs->indexOfNode(img), 0);
            ui->tblviewImages->setCurrentIndex(m_currentImage);
            on_tblviewImages_clicked(m_currentImage);
            ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(img->parentSprite()->getPalette()), ui->lbl_imgpreview->size()));
        }
        else
            ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());
    }
    BaseSpriteTab::OnShowTab(pspr, element);
}

void TabImages::OnHideTab()
{
    //Clear mapper
    ClearMappings();

    //Clear table view
    ui->tblviewImages->clearSelection();
    ui->tblviewImages->setModel(nullptr);

    //Clear preview
    ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());

    //Clear current image
    m_currentImage = QModelIndex();

    BaseSpriteTab::OnHideTab();
}

void TabImages::ClearMappings()
{
    if(m_imgdatmapper)
    {
        disconnect(ui->tblviewImages, &QTableView::clicked, m_imgdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
        m_imgdatmapper->clearMapping();
    }
    m_imgdatmapper.reset();
}

void TabImages::OnDestruction()
{
    PrepareForNewContainer();
    BaseSpriteTab::OnDestruction();
}

void TabImages::PrepareForNewContainer()
{
    OnHideTab();
    ui->tblviewImages->reset();
}

void TabImages::OnItemRemoval(const QModelIndex &item)
{
    if(item.internalPointer() == currentSprite())
        PrepareForNewContainer(); //Clear everything!
    if(item == m_currentImage)
    {
        //If we deleted the image we're viewing, clear our selection and the preview window!
        ui->tblviewImages->clearSelection();
        ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());
        ClearMappings();
        m_currentImage = QModelIndex();
    }
    BaseSpriteTab::OnItemRemoval(item);
}

// *********************************
//  Image Tab
// *********************************
void TabImages::on_btnImageCrop_clicked()
{
    Image * pimg = static_cast<Image*>(m_currentImage.internalPointer());
    if(pimg)
    {
        DiagSingleImgCropper cropper(this, pimg);
        cropper.setModal(true);
        cropper.show();
    }
    else
    {
        QApplication::beep();
        ShowStatusErrorMessage("Can't crop! No valid image selected!");
        qWarning("MainWindow::on_btnImageCrop_clicked(): Crop clicked, but no valid images was selected!");
    }
}

void TabImages::on_tblviewImages_clicked(const QModelIndex &index)
{
    Image * img = static_cast<Image *>(index.internalPointer());
    if(!index.internalPointer() || !img)
    {
        ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());
        m_currentImage = QModelIndex();
        return;
    }
    ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(img->parentSprite()->getPalette()), ui->lbl_imgpreview->size()));
    m_currentImage = index;
}

void TabImages::on_btnImagesExport_clicked()
{
    Image * pimg = nullptr;
    if(m_currentImage.isValid())
        pimg = static_cast<Image*>(m_currentImage.internalPointer());

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

    QImage img = pimg->makeImage(pimg->parentSprite()->getPalette());

    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported image to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}


void TabImages::on_btnImagesImport_clicked()
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
    newimgdata.data = utils::ImgToRaw(img);

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


void TabImages::on_spbimgunk2_valueChanged(int arg1)
{
//    Image * pimg = this->currentImage();
//    if(pimg)
//        pimg->setUnk2(static_cast<uint32_t>(arg1));
}

void TabImages::on_spbimgunk14_valueChanged(int arg1)
{
//    Image * pimg = this->currentImage();
//    if(pimg)
//        pimg->setUnk14(static_cast<uint16_t>(arg1));
}
