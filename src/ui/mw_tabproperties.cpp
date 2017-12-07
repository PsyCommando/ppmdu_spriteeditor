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



void MainWindow::DisplayPropertiesPage(Sprite * spr)
{
    Q_ASSERT(spr);
    qDebug() << "MainWindow::DisplayPropertiesPage(): Showing properties tab!\n";

    spr_manager::SpriteContainer * pcnt = spr_manager::SpriteManager::Instance().getContainer();
    ui->tv_sprcontent->setCurrentIndex(pcnt->index(pcnt->indexOfNode(spr), 0, QModelIndex(), &spr_manager::SpriteManager::Instance() ));
    if( !spr->wasParsed() )
        spr->ParseSpriteData();

    //display preview only if we have image data!
    if( spr->hasImageData() )
        ui->lblPropPreview->setPixmap(spr->MakePreviewFrame().scaled( ui->lblPropPreview->size(), Qt::KeepAspectRatio) );
    else
        ui->lblPropPreview->setPixmap(m_imgNoImg);

    ui->tblProperties->setModel(spr->propHandler()->model());
    ui->tblProperties->setItemDelegate(spr->propHandler()->delegate());

    //display palette preview
    ui->lbl_test_palette->setPixmap(spr->MakePreviewPalette());

    //Setup stats
    ui->tblOverview->setModel(spr->overviewModel());

    ShowATab(ui->tabProperties);
}



// *********************************
//  Properties Tab
// *********************************
void MainWindow::on_btnImportPalette_clicked()
{
    using namespace spr_manager;
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprite to import to!")) );
        return;
    }

    QString selectedfilter;
    ePaletteDumpType type;
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Import Palette File"),
                                                    QString(),
            PaletteFilterString() + ";;" + GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL), //allow loading a PNG for its palette!
                                                    &selectedfilter );
    if(filename.isNull())
        return;

    if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal))
        type = ePaletteDumpType::RIFF_Pal;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal))
        type = ePaletteDumpType::TEXT_Pal;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL))
        type = ePaletteDumpType::GIMP_PAL;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL))
        type = ePaletteDumpType::PNG_PAL;
    Q_ASSERT(type < ePaletteDumpType::INVALID);

    try
    {
        spr_manager::SpriteManager::Instance().ImportPalette(spr, filename, type);
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
    DisplayPropertiesPage(spr);
}

void MainWindow::on_btnExportPalette_clicked()
{
    using namespace spr_manager;
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprites to dump from!")) );
        return;
    }

    QString selectedfilter;
    ePaletteDumpType type;
    qInfo("MainWindow::on_btnExportPalette_clicked(): Exporting palette!");
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Save Palette Dump As"),
                        QString(),
                        PaletteFilterString(),
                        &selectedfilter);

    if(filename.isNull())
        return;

    if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal) &&
       !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal), Qt::CaseInsensitive))
    {
        //filename.append(".pal");
        type = ePaletteDumpType::RIFF_Pal;
    }
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal) &&
            !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal), Qt::CaseInsensitive))
    {
        //filename.append(".txt");
        type = ePaletteDumpType::TEXT_Pal;
    }
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL) &&
            !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL), Qt::CaseInsensitive))
    {
        //filename.append(".gpl");
        type = ePaletteDumpType::GIMP_PAL;
    }

    Q_ASSERT(type < ePaletteDumpType::INVALID);

    try
    {
        spr_manager::SpriteManager::Instance().DumpPalette(spr, filename, type);
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
