#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QTreeWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMutex>
#include <QMutexLocker>
#include <QDataWidgetMapper>
#include <QMenu>
#include <QPersistentModelIndex>
#include <QSettings>
#include <QCloseEvent>
#include <QUndoStack>

#include "src/data/sprite/spritemanager.hpp"
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/dialogprogressbar.hpp>
#include <atomic>

#include <src/ui/editor/frame/frame_editor.hpp>

class BaseSpriteTab;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class TVSpritesContextMenu;
    friend class BaseSpriteTab;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Settings
    void writeSettings();
    void readSettings();

    //Internal processing stuff
    void HideAllTabs();
    void ShowATab(QWidget * ptab);
    void ShowATab(BaseSpriteTab *ptab, Sprite * pspr, const QModelIndex & element);

    void DisplayStartScreen();
    void DisplayPropertiesPage(Sprite * spr);
    void DisplayMFramePage(Sprite * spr, const QModelIndex & index);
    void DisplayAnimSequencePage(Sprite * spr, const QModelIndex & index);
//    void DisplayAnimSequencePage(QPersistentModelIndex spr, QPersistentModelIndex aniseq);
    void DisplayAnimTablePage(Sprite * spr, const QModelIndex & index);
    void DisplayEffectsPage(Sprite * spr, const QModelIndex & index);
    void DisplayImageListPage(Sprite * spr, const QModelIndex & index);

    //Hiding stuff
    void HideAnimSequencePage();

    //Basically do house keeping stuff when we load something new in the app
    void PrepareForNewContainer();

    void LoadContainer( const QString & path );
    void SaveContainer( const QString & path );
    void ExportContainer( const QString & path );
    void ImportContainer( const QString & path );
    void CloseContainer();

    void OnActionAddSprite();
    void OnActionRemSprite();

    //Show a warning message box with the specified content
    void Warn(const QString & title, const QString & text);

    void ShowStatusMessage(const QString & msg);
    void ShowStatusErrorMessage(const QString & msg);


    void HandleItemRemoval(QModelIndex spriteidx);

    void setSelectedTreeViewIndex(const QModelIndex &index);
    const QPixmap & getDefaultImage()const {return m_imgNoImg;}
private:
    QPixmap RenderNoImageSvg();

    void updateActions();

    spr_manager::SpriteContainer * getContainer();
    spr_manager::SpriteManager & getManager();

    //Tweak the list view to better display either single sprites or pack files!!
    void setupListView();
    void SetupUIForNewContainer(spr_manager::SpriteContainer * sprcnt);

    void setupFrameEditPageForPart( MFrame * frm,  MFramePart * part );

    void SaveAs( const QString & path );

    //Show a message box asking if we should save the changes, and return the button pressed!
    int AskSaveChanges();

    void PushUndoAction(QUndoCommand * cmd);
//    void InitAnimScene();
//    void ConnectSceneRenderer();
//    void DisconnectSceneRenderer();

    QMenu *makeSpriteContextMenu(QModelIndex entry);

    Sprite      * currentSprite();
    QModelIndex currentSpriteModelIndex();
    MFrame      * currentFrame();
    Image       * currentImage();
    AnimSequence* currentAnimSequence();
    eTreeElemDataType currentEntryType();
    //For specific sub-tables
//    MFramePart  * currentTblFrameParts();
//    Image       * currentTblImages();

//    static QString GetPaletteImportFiterString();

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override
    {
//        if (userReallyWantsToQuit())
//        {
            writeSettings();
            event->accept();
//        }
//        else
//        {
//            event->ignore();
//        }
    }
private slots:
    void on_tv_sprcontent_expanded(const QModelIndex &index);
    void on_tv_sprcontent_clicked(const QModelIndex &index);
    void on_tv_sprcontent_customContextMenuRequested(const QPoint &pos);
    void on_tv_sprcontent_activated(const QModelIndex &index);

    void on_actionNewSprite_triggered();
    void on_actionNewSprite_Pack_File_triggered();
    void on_action_Save_triggered();
    void on_actionSave_As_triggered();
    void on_action_About_triggered();
    void on_action_Quit_triggered();
    void on_action_Open_triggered();



    void ShowProgressDiag(QFuture<void> & task);

//    void on_tblviewImages_clicked(const QModelIndex &index);
//    void on_btnImagesExport_clicked();
//    void on_btnImagesImport_clicked();
//    void on_spbimgunk2_valueChanged(int arg1);
//    void on_spbimgunk14_valueChanged(int arg1);
//    void on_btnImageCrop_clicked();

//    void on_tblframeparts_clicked(const QModelIndex &index);
//    void on_btnFrmExport_clicked();
//    void on_btnFrmRmPart_clicked();
//    void on_btnFrmAdPart_clicked();
//    void on_btnFrmMvUp_clicked();
//    void on_btnFrmMvDown_clicked();
//    void on_btnFrmDup_clicked();
//    void on_cmbFrmQuickPrio_currentIndexChanged(int index);


//    void on_btnExportPalette_clicked();
//    void on_btnImportPalette_clicked();



signals:

private:
    Ui::MainWindow          *ui;
    QScopedPointer<QLabel>  m_pStatusFileType;
    QScopedPointer<QLabel>  m_pStatusError;
    QScopedPointer<QAction> m_pActionAddSprite;
    QScopedPointer<QAction> m_pActionRemSprite;
    QPixmap                 m_imgNoImg;             //Image displayed when no image can be displayed in a view!
    DialogAbout             m_aboutdiag;
    DialogProgressBar       m_progress;
    QString                 m_lastSavePath;
    QPersistentModelIndex   m_cursprite;
    QSettings               m_settings;
    QUndoStack              m_undoStack;



    static const QString & WanFileFilter()
    {
        static const QString filter(tr("WAN Sprite (*.wan)"));
        return filter;
    }
    static const QString & WatFileFilter()
    {
        static const QString filter(tr("WAT Sprite (*.wat)"));
        return filter;
    }
    static const QString & PACKFileFilter()
    {
        static const QString filter(tr("Pack Files (*.bin)"));
        return filter;
    }

    static const QString & AllFileFilter()
    {
        static const QString filter(tr("All supported formats (*.bin *.wan *.wat *.pkdpx)"));
        return filter;
    }
    void forEachTab(std::function<void (BaseSpriteTab *)> fun);
};


#endif // MAINWINDOW_H
