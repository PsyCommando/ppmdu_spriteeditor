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

QString MainWindow::GetPaletteImportFiterString()
{
    using namespace spr_manager;
    static const QString pimportstr = QString("%1;;%2").arg( PaletteFilterString(),
                                      GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL)); //allow loading a PNG for its palette!
    return pimportstr;
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
