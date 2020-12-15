#include "tabproperties.hpp"
#include "ui_tabproperties.h"
#include <src/ui/mainwindow.hpp>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/data/sprite/models/sprite_props_model.hpp>
#include <src/data/sprite/models/sprite_props_delegate.hpp>
#include <src/utility/file_support.hpp>
#include <src/utility/palette_helpers.hpp>

#include <QFileDialog>


//const QVector<QString> PaletteFileFilter
//{
//    "Microsoft RIFF palette (*.pal)",
//    "Text file hex color list (*.txt)",
//    "GIMP GPL palette (*.gpl)",
//    "Palette from a PNG image (*.png)",
//};

TabProperties::TabProperties(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabProperties)
{
    ui->setupUi(this);
}

TabProperties::~TabProperties()
{
    delete ui;
}

//const QString TabProperties::PaletteFilterString()
//{
//    static const QString filter = GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal) +
//                                  ";;" +
//                                  GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal) +
//                                  ";;" +
//                                  GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL);
//    return filter;
//}

void TabProperties::OnShowTab(QPersistentModelIndex element)
{
    qDebug() << "MainWindow::DisplayPropertiesPage(): Showing properties tab!\n";
    ContentManager & manager = ContentManager::Instance();
    Sprite * pspr = dynamic_cast<Sprite*>(manager.getOwnerNode(element));
    Q_ASSERT(pspr);

    //display properties
    m_propHandler.reset(new SpritePropertiesHandler(pspr));
    ui->tblProperties->setModel(m_propHandler->model());
    ui->tblProperties->setItemDelegate(m_propHandler->delegate());

    //Setup stats
    m_overviewModel.reset(new SpriteOverviewModel(pspr));
    ui->tblOverview->setModel(m_overviewModel.data());

    //display preview image and palette only if we have image data!
    UpdatePreview();

    BaseSpriteTab::OnShowTab(element);
}

void TabProperties::OnHideTab()
{
    //Clear previews
    ui->lblPropPreview->setPixmap(m_pmainwindow->getDefaultImage());
    ui->lbl_test_palette->setPixmap(m_pmainwindow->getDefaultImage());

    //Clear overview
    ui->tblOverview->setModel(nullptr);

    //Clear properties
    ui->tblProperties->setItemDelegate(nullptr);
    ui->tblProperties->clearSelection();
    ui->tblProperties->setModel(nullptr);

    m_propHandler.reset();
    m_overviewModel.reset();

    BaseSpriteTab::OnHideTab();
}

void TabProperties::OnDestruction()
{
    OnHideTab();
    BaseSpriteTab::OnDestruction();
}

void TabProperties::PrepareForNewContainer()
{
    OnHideTab();
    ui->tblProperties->reset();
}

void TabProperties::OnItemRemoval(const QModelIndex &item)
{
    TreeNode * te = reinterpret_cast<TreeNode*>(item.internalPointer());
    if(te == currentSprite())
    {
        //Removing the sprite, so we clear everything
        PrepareForNewContainer();
    }
    else if(te && (te->nodeDataTy() == eTreeElemDataType::image || te->nodeDataTy() == eTreeElemDataType::frame || te->nodeDataTy() == eTreeElemDataType::framepart))
    {
        //Regenerate the previews if we changed the images, in case the image used in the preview was modified!
        UpdatePreview();
    }
    BaseSpriteTab::OnItemRemoval(item);
}

// *********************************
//  Properties Tab
// *********************************
void TabProperties::on_btnImportPalette_clicked()
{
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprite to import to!")) );
        return;
    }

    QString selectedfilter;
    ePaletteDumpType ftype;
    QString filename = QFileDialog::getOpenFileName(this,
                                                    QString(tr("Import Palette File")),
                                                    QString(),
                                                    AllSupportedImportPaletteFilesFilter(),
                                                    &selectedfilter );
    if(filename.isNull())
        return;

    ftype = FilterStringToPaletteType(selectedfilter);

    try
    {
        ImportPalette(spr, filename, ftype);
    }
    catch(const std::exception & e)
    {
        ShowStatusErrorMessage(QString(tr("Error: %1")).arg(e.what()) );
        return;
    }
    catch(...)
    {
        std::rethrow_exception(std::current_exception());
        return;
    }

    ShowStatusMessage( QString(tr("Palette imported!")) );

    //Refresh property page
    UpdatePreview();
}

void TabProperties::on_btnExportPalette_clicked()
{
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprites to dump from!")) );
        return;
    }

    try
    {
        QString selectedfilter = SupportedExportPaletteFilesFilters[FileExtTXTPAL];
        qInfo("MainWindow::on_btnExportPalette_clicked(): Exporting palette!");
        QString filename = QFileDialog::getSaveFileName(this,
                            QString(tr("Save Palette Dump As")),
                            QString(),
                            AllSupportedExportPaletteFilesFilter(),
                            &selectedfilter);

        if(filename.isNull())
            return;

        qDebug() << QString("Palette Length to export is %1").arg(spr->getPalette().size());

        ePaletteDumpType ftype = FilterStringToPaletteType(selectedfilter);
        DumpPalette(spr, filename, ftype);
    }
    catch(const std::exception & e)
    {
        ShowStatusErrorMessage(QString(tr("Error: %1")).arg(e.what()) );
        return;
    }
    catch(...)
    {
        std::rethrow_exception(std::current_exception());
        return;
    }

    ShowStatusMessage( QString(tr("Palette dumped!")) );
}

QPixmap TabProperties::GenerateSpritePreviewImage()
{
    Sprite * pspr = currentSprite();
    if(!pspr || (pspr && !pspr->hasImageData()))
        return m_pmainwindow->getDefaultImage();
     return pspr->MakePreviewFrame().scaled( ui->lblPropPreview->size(), Qt::KeepAspectRatio);
}

QPixmap TabProperties::GenerateSpritePreviewPalette()
{
    Sprite * pspr = currentSprite();
    if(!pspr || (pspr && !pspr->hasImageData()))
        return m_pmainwindow->getDefaultImage();
    return pspr->MakePreviewPalette();
}

void TabProperties::UpdatePreview()
{
    ui->lblPropPreview->setPixmap(GenerateSpritePreviewImage());
    ui->lbl_test_palette->setPixmap(GenerateSpritePreviewPalette());
}
