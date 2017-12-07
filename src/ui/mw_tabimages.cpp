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

    //#TODO: Update image details if needed
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


