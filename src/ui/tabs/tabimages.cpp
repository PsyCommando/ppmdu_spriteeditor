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

void TabImages::OnShowTab(QPersistentModelIndex element)
{
    Sprite * pspr = currentSprite();
    Q_ASSERT(pspr);
    qDebug() << "MainWindow::DisplayImageListPage(): Displaying images list page!\n";
    SetupMappings(pspr);
    qDebug() << "MainWindow::DisplayImageListPage(): Model set!\n";
    SetupImage(element);
    BaseSpriteTab::OnShowTab(element);
}

void TabImages::OnHideTab()
{
    //Clear table view
    ui->tblviewImages->clearSelection();
    ui->tblviewImages->setModel(nullptr);

    //Clear mapper
    ClearMappings();

    //Clear current image
    ClearImage();
    BaseSpriteTab::OnHideTab();
}

void TabImages::SetupMappings(Sprite * spr)
{
    //Create the model
    m_imgListModel.reset(new ImageListModel(&spr->getImages(), spr));
    ui->spbimgunk2->setRange (0, std::numeric_limits<uint32_t>::max());
    ui->spbimgunk14->setRange(0, std::numeric_limits<uint16_t>::max());

    ui->tblviewImages->setModel(m_imgListModel.data());
    ui->tblviewImages->resizeRowsToContents();
    ui->tblviewImages->resizeColumnsToContents();

    //Map model's columns to some of the controls
    m_imgdatmapper.reset(new QDataWidgetMapper);
    m_imgdatmapper->setModel(m_imgListModel.data());
    m_imgdatmapper->addMapping(ui->spbimgunk2,  static_cast<int>(Image::eColumnType::direct_Unk2) );
    m_imgdatmapper->addMapping(ui->spbimgunk14,  static_cast<int>(Image::eColumnType::direct_Unk14) );
    //m_imgdatmapper->toFirst();
    connect(ui->tblviewImages, &QTableView::clicked, m_imgdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}

void TabImages::ClearMappings()
{
    if(m_imgdatmapper)
    {
        disconnect(ui->tblviewImages, &QTableView::clicked, m_imgdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
        m_imgdatmapper->clearMapping();
    }
    m_imgdatmapper.reset();
    m_imgListModel.reset();
}

void TabImages::SetupImage(QPersistentModelIndex index)
{
    Image * img = static_cast<Image *>(index.internalPointer());
    if(!index.isValid()|| !img)
    {
        ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());
        ClearImage();
        return;
    }
    ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(m_imgListModel->getOwnerSprite()->getPalette()), ui->lbl_imgpreview->size()));
    m_currentImage = index;
    ui->tblviewImages->setCurrentIndex(m_currentImage);
    m_imgdatmapper->setCurrentModelIndex(m_currentImage);
}

void TabImages::ClearImage()
{
    ui->tblviewImages->clearSelection();
    m_currentImage = QModelIndex();
    ui->lbl_imgpreview->setPixmap(m_pmainwindow->getDefaultImage());
    m_imgdatmapper->setCurrentModelIndex(QModelIndex());
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
        ClearImage();
    }
    BaseSpriteTab::OnItemRemoval(item);
}

void TabImages::on_tblviewImages_clicked(const QModelIndex &index)
{
    SetupImage(index);
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

void TabImages::on_btnAdd_clicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this,
                                tr("Import Images"),
                                QString(),
                                "PNG image (*.png)");

    if(filenames.empty())
        return;

    for(const QString & path : filenames)
    {
        TryImportImage(path);
    }
}

void TabImages::on_btnRemove_clicked()
{
    QModelIndexList selected = getSelectedIndices();
    if(selected.isEmpty())
    {
        ShowStatusErrorMessage(tr("No image selected!"));
        return;
    }
    Sprite * spr = m_imgListModel->getOwnerSprite();
    bool removedrefs = spr->getFrames().ClearImageReferences(selected); //set references to the removed images to -1 to preserve framepart data
    if(m_imgListModel->removeRows(selected))
        ShowStatusMessage(QString(tr("Removed %1 image(s)%2")).arg(selected.size()).arg(removedrefs? tr(", and cleared references to images from frame data.") : ""));
    else
        ShowStatusErrorMessage(tr("Couldn't remove all images!"));
}

QModelIndexList TabImages::getSelectedIndices()const
{
    QItemSelectionModel * model = ui->tblviewImages->selectionModel();
    return model->selectedRows();
}

void TabImages::TryImportImage(const QString &path)
{
    //Load image
    QImage img(path, "PNG");

    //Validate image
    if(img.format() != QImage::Format_Indexed8)
    {
        Warn(tr("Invalid image format!"), QString(tr("%1 is not 8 bits per pixel (aka 256 colors). Make sure the image to import is saved as a PNG 8bpp, 256 colors!")).arg(path));
        return;
    }

    //Check if the resolution is one of the allowed ones.
    //Otherwise, warn the user
    QSize imgres = img.size();
    if(!IsImageResolutionValid(imgres))
    {
        QString desc = tr("Images imported this way must fit one of the supported resolution:\n");

        for(const auto & res : fmt::FrameResValues )
            desc+= QString("%1x%2\n").arg(res.first, res.second);
        desc += tr("You can use the auto-import function instead to import irregular sized images and chop them automatically and generate frames from those.");
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

    //Insert at the end of the list
    bool succ = ui->tblviewImages->model()->insertRow( ui->tblviewImages->model()->rowCount());

    if(succ)
    {
        QModelIndex newidx = ui->tblviewImages->model()->index(ui->tblviewImages->model()->rowCount()-1, 0);
        Image * newimg = static_cast<Image *>(newidx.internalPointer());
        Q_ASSERT(newimg);

        //Replace selected image
        newimg->importImage8bpp(newimgdata, img.width(), img.height(), false);
    }
    else
        Warn(tr("Import Failed!"),tr("Couldn't create a new row at the end of the list! Qt refused!"));
}

void TabImages::on_btnExport_clicked()
{
    QModelIndexList selected = getSelectedIndices();
    if(selected.isEmpty())
    {
        ShowStatusErrorMessage(tr("No image selected!"));
        return;
    }

    for(const QModelIndex & idx : selected)
    {
        if(!idx.isValid())
            return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image(s)"),
                        QString(),
                        "PNG image (*.png)");
    if(filename.isNull())
        return;

    int successcnt = 0;
    QList<QString> failed;
    bool bmultiple = selected.size() > 1;
    for(const QModelIndex& imgidx : selected)
    {
        const Image * pimg = dynamic_cast<const Image *>(m_imgListModel->getItem(imgidx));
        Q_ASSERT(pimg);
        QImage img = pimg->makeImage(m_imgListModel->getOwnerSprite()->getPalette());
        QFileInfo finfo(filename);
        QString newpath = QString("%1/%2%3.%4") .arg(finfo.absolutePath()) .arg(finfo.baseName());
        newpath = (bmultiple? newpath.arg(successcnt) : newpath.arg(""));
        newpath = newpath.arg(finfo.completeSuffix());

        if(img.save(newpath, "PNG"))
            ++successcnt;
        else
            failed.append(newpath);
    }

    QFileInfo basepath(filename);
    if(failed.empty())
        ShowStatusMessage(QString(tr("Exported %1 image(s) to %2!")).arg(successcnt).arg(basepath.absolutePath()));
    else
    {
        QString failedlist;
        for(QString & f : failed)
        {
            if(failedlist.size() > 0)
                failedlist.append(QString(", %1").arg(f));
            else
                failedlist.append(f);
        }
        Warn(tr("Export partially failed"), tr("Failed to save the following files:\n") + failedlist);
    }
}
