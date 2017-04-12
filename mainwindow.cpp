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
}

void MainWindow::SaveContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().SaveContainer(path);
}

void MainWindow::ExportContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().ExportContainer(path);
}

void MainWindow::ImportContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().ImportContainer(path);
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

        //Fill the treeview!


        //Fill the properties page!

        //Display!
        DisplayPropertiesPage();
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
