#include "tabproperties.hpp"
#include "ui_tabproperties.h"

#include <src/ui/mainwindow.hpp>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>

#include <QFileDialog>

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

QString TabProperties::GetPaletteImportFiterString()
{
    using namespace spr_manager;
    static const QString pimportstr = QString("%1;;%2").arg( PaletteFilterString(),
                                      GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL)); //allow loading a PNG for its palette!
    return pimportstr;
}

const QString TabProperties::PaletteFilterString()
{
    static const QString filter = spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::RIFF_Pal) +
                                  ";;" +
                                  spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::TEXT_Pal) +
                                  ";;" +
                                  spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::GIMP_PAL);
    return filter;
}

void TabProperties::OnShowTab(Sprite *pspr, QPersistentModelIndex element)
{
    Q_ASSERT(pspr);
    qDebug() << "MainWindow::DisplayPropertiesPage(): Showing properties tab!\n";

    spr_manager::SpriteContainer * pcnt = spr_manager::SpriteManager::Instance().getContainer();
    m_pmainwindow->setSelectedTreeViewIndex(pcnt->index(pcnt->indexOfNode(pspr), 0, QModelIndex(), &spr_manager::SpriteManager::Instance()));
    if( !pspr->wasParsed() )
        pspr->ParseSpriteData();

    //display preview image and palette only if we have image data!
    UpdatePreview();

    //display properties
    ui->tblProperties->setModel(pspr->propHandler()->model());
    ui->tblProperties->setItemDelegate(pspr->propHandler()->delegate());

    //Setup stats
    ui->tblOverview->setModel(pspr->overviewModel());
    BaseSpriteTab::OnShowTab(pspr, element);
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
    TreeElement * te = static_cast<TreeElement*>(item.internalPointer());
    if(te == currentSprite())
    {
        //Removing the sprite, so we clear everything
        PrepareForNewContainer();
    }
    else if(te && (te->getNodeDataTy() == eTreeElemDataType::image || te->getNodeDataTy() == eTreeElemDataType::frame || te->getNodeDataTy() == eTreeElemDataType::framepart))
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
    using namespace spr_manager;
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
                                                    GetPaletteImportFiterString(),
                                                    &selectedfilter );
    if(filename.isNull())
        return;

    ftype = FilterStringToPaletteType(selectedfilter);

    try
    {
        spr_manager::SpriteManager::Instance().ImportPalette(spr, filename, ftype);
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
    using namespace spr_manager;
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprites to dump from!")) );
        return;
    }

    try
    {
        QString selectedfilter = GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal);
        qInfo("MainWindow::on_btnExportPalette_clicked(): Exporting palette!");
        QString filename = QFileDialog::getSaveFileName(this,
                            QString(tr("Save Palette Dump As")),
                            QString(),
                            PaletteFilterString(),
                            &selectedfilter);

        if(filename.isNull())
            return;

        qDebug() << QString("Palette Length to export is %1").arg(spr->getPalette().size());

        ePaletteDumpType ftype = FilterStringToPaletteType(selectedfilter);
        SpriteManager::Instance().DumpPalette(spr, filename, ftype);
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
