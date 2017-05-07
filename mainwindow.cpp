#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_pStatusFileType.reset(new QLabel("     "));
    ui->setupUi(this);
    HideAllTabs();
    ui->tv_sprcontent->setModel( & spr_manager::SpriteManager::Instance() );
    ui->statusBar->addPermanentWidget(m_pStatusFileType.data());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::HideAllTabs()
{
    //Hide all
    ui->tabMain->setUpdatesEnabled(false);
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabanims));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabeffects));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabframeseditor));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabpal));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabseq));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabproperties));

    ui->tabanims->hide();
    ui->tabeffects->hide();
    ui->tabframeseditor->hide();
    ui->tabpal->hide();
    ui->tabseq->hide();
    ui->tabproperties->hide();

    ui->tabMain->setUpdatesEnabled(true);
}

void MainWindow::ShowATab(QWidget *ptab)
{
    HideAllTabs();
    ui->tabMain->insertTab(0, ptab, ptab->windowTitle() );
    ptab->show();
    ptab->setFocus();
}

void MainWindow::DisplayPropertiesPage()
{
    ShowATab(ui->tabproperties);
}

void MainWindow::DisplayAnimFramePage()
{
    ShowATab(ui->tabframeseditor);
}

void MainWindow::DisplayAnimSequencePage()
{
    ShowATab(ui->tabseq);
}

void MainWindow::DisplayAnimTablePage()
{
    ShowATab(ui->tabanims);
}

void MainWindow::DisplayPalettePage()
{
    ShowATab(ui->tabpal);
}

void MainWindow::DisplayEffectsPage()
{
    ShowATab(ui->tabeffects);
}

void MainWindow::LoadContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().OpenContainer(path);
    updateActions();
}

void MainWindow::SaveContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().SaveContainer(path);
    updateActions();
}

void MainWindow::ExportContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().ExportContainer(path);
    updateActions();
}

void MainWindow::ImportContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().ImportContainer(path);
    updateActions();
}

void MainWindow::on_action_Open_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            tr("Pack Files (*.bin);;WAN Sprite (*.wan *.wat)"));

    if (!fileName.isEmpty())
    {
        //Check if a sprite was open and ask to save changes!
        //DO OPEN FILE HERE!
        if( sprman.IsContainerLoaded() )
        {
            //Ask to save or discard changes!!
        }

        //Open
        sprman.OpenContainer(fileName);
        //sprman.submit();


        //Fill the properties page!

        //Display!
        DisplayPropertiesPage();
        setupListView();
        updateActions();
    }
}

void MainWindow::on_action_Quit_triggered()
{
    QApplication::instance()->exit();
}

void MainWindow::on_tv_sprcontent_itemSelectionChanged()
{

}

void MainWindow::on_tv_sprcontent_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{

}

void MainWindow::on_tv_sprcontent_expanded(const QModelIndex &index)
{

}

void MainWindow::on_tv_sprcontent_itemClicked(QTreeWidgetItem *item, int column)
{

}

void MainWindow::on_action_Save_triggered()
{

}

void MainWindow::on_actionSave_As_triggered()
{

}

void MainWindow::on_action_Export_triggered()
{

}

void MainWindow::on_actionSprite_triggered()
{

}

void MainWindow::on_actionSprite_Pack_File_triggered()
{

}

void MainWindow::on_actionUndo_triggered()
{

}

void MainWindow::on_actionRedo_triggered()
{

}

void MainWindow::on_action_Settings_triggered()
{

}

void MainWindow::on_action_About_triggered()
{

}

void MainWindow::updateActions()
{
//    bool hasSelection = ! ui->tv_sprcontent->selectionModel()->selection().isEmpty();
//    removeRowAction->setEnabled(hasSelection);
//    removeColumnAction->setEnabled(hasSelection);

//    bool hasCurrent = view->selectionModel()->currentIndex().isValid();
//    insertRowAction->setEnabled(hasCurrent);
//    insertColumnAction->setEnabled(hasCurrent);

//    if (hasCurrent) {
//        view->closePersistentEditor(view->selectionModel()->currentIndex());

//        int row = view->selectionModel()->currentIndex().row();
//        int column = view->selectionModel()->currentIndex().column();
//        if (view->selectionModel()->currentIndex().parent().isValid())
//            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
//        else
//            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
//    }

    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    m_pStatusFileType->setText(sprman.getContentShortName());


}

void MainWindow::on_actionNewSprite_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::WAN );
    //sprman.submit();

    //Display!
    DisplayPropertiesPage();
    updateActions();
    setupListView();
}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::PACK );
    //sprman.submit();

    //Display!
    DisplayPropertiesPage();
    updateActions();
    setupListView();
}

void MainWindow::setupListView()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    if( sprman.ContainerIsSingleSprite() )
    {
        ui->tv_sprcontent->setRootIsDecorated(false);
        ui->tv_sprcontent->expandToDepth(1);
    }
    else
    {
        ui->tv_sprcontent->setRootIsDecorated(true);
        ui->tv_sprcontent->collapseAll();
    }
}

void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    TreeElement * pcur = static_cast<TreeElement*>(index.internalPointer());
    if(!pcur)
        return;

    pcur->OnClicked();

    switch( pcur->getDataTy() )
    {
        //Open the appropriate tab
    case eTreeElemDataType::sprite:
        {
            Sprite * spr = static_cast<Sprite*>(pcur);

            //QPixmap mypix = spr->MakePreviewFrame();
            //QGraphicsPixmapItem * ppix = m_pPreviewScene->add(mypix);
            ui->lblPropPreview->setPixmap(spr->MakePreviewFrame().scaled( ui->lblPropPreview->size(), Qt::KeepAspectRatio) );


            ui->lbl_test_palette->setPixmap(spr->MakePreviewPalette());
            DisplayPropertiesPage();
            break;
        }
    case eTreeElemDataType::palette:
        {
            DisplayPalettePage();
            break;
        }
    case eTreeElemDataType::effectOffsets:
        {
            DisplayEffectsPage();
            break;
        }
    case eTreeElemDataType::animTable:
        {
            DisplayAnimTablePage();
            break;
        }
    case eTreeElemDataType::frame:
        {
            DisplayAnimFramePage();
            break;
        }
    case eTreeElemDataType::animSequence:
        {
            DisplayAnimSequencePage();
            break;
        }
    default:
        HideAllTabs();
    };
}

void MainWindow::on_tv_sprcontent_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex entry = ui->tv_sprcontent->indexAt(pos);
    if( entry.isValid() )
    {
        //This is triggered when clicking on an item
        //remove/copy/duplicate/etc
    }
    else
    {
        //This is triggered when clicking in the white unused space
        //Add sprite/frame/sequence/image/etc
    }
}
