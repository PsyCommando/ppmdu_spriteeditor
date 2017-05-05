#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    HideAllTabs();
    ui->tv_sprcontent->setModel( & spr_manager::SpriteManager::Instance() );
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
    //Show properties
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
        sprman.submit();

        //Fill the properties page!

        //Display!
        DisplayPropertiesPage();

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
}

void MainWindow::on_actionNewSprite_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::WAN );
    sprman.submit();

    //Display!
    DisplayPropertiesPage();
    updateActions();
}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::PACK );
    sprman.submit();

    //Display!
    DisplayPropertiesPage();
    updateActions();
}
