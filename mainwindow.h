#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTreeWidget>
#include "src/spritemanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Internal processing stuff
    void HideAllTabs();
    void ShowATab(QWidget * ptab);
    void DisplayPropertiesPage();
    void DisplayAnimFramePage();
    void DisplayAnimSequencePage();
    void DisplayAnimTablePage();
    void DisplayPalettePage();
    void DisplayEffectsPage();

    void LoadContainer( const QString & path );
    void SaveContainer( const QString & path );
    void ExportContainer( const QString & path );
    void ImportContainer( const QString & path );


private slots:
    void on_action_Open_triggered();

    void on_action_Quit_triggered();

    void on_tv_sprcontent_itemSelectionChanged();

    void on_tv_sprcontent_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_tv_sprcontent_expanded(const QModelIndex &index);

    void on_tv_sprcontent_itemClicked(QTreeWidgetItem *item, int column);

    void on_action_Save_triggered();

    void on_actionSave_As_triggered();

    void on_action_Export_triggered();

    void on_actionSprite_triggered();

    void on_actionSprite_Pack_File_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_action_Settings_triggered();

    void on_action_About_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
