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

#include "src/spritemanager.hpp"
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/dialogprogressbar.hpp>
#include <atomic>

#include <src/frameeditor.hpp>

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
    void ShowATab(BaseSpriteTab *ptab);

    void DisplayStartScreen();
    void DisplayPropertiesPage(Sprite * spr);
    void DisplayMFramePage(Sprite * spr, MFrame *frm);
//    void DisplayAnimSequencePage(Sprite * spr, AnimSequence *aniseq);
//    void DisplayAnimSequencePage(QPersistentModelIndex spr, QPersistentModelIndex aniseq);
//    void DisplayAnimTablePage(Sprite * spr);
    void DisplayEffectsPage(Sprite * spr);
    void DisplayImageListPage(Sprite * spr, ImageContainer *pimgs, Image * img = nullptr );

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
    MFramePart  * currentTblFrameParts();
    Image       * currentTblImages();

    static QString GetPaletteImportFiterString();

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

    void on_btnImageCrop_clicked();

    void ShowProgressDiag(QFuture<void> & task);

    void on_tblviewImages_clicked(const QModelIndex &index);
    void on_btnImagesExport_clicked();

    void on_tblframeparts_clicked(const QModelIndex &index);
    void on_btnFrmExport_clicked();
    void on_btnFrmRmPart_clicked();
    void on_btnFrmAdPart_clicked();
    void on_btnFrmMvUp_clicked();
    void on_btnFrmMvDown_clicked();
    void on_btnFrmDup_clicked();
    void on_cmbFrmQuickPrio_currentIndexChanged(int index);


    void on_btnExportPalette_clicked();
    void on_btnImportPalette_clicked();

//    void on_btnSeqExport_clicked();
//    void on_btnSeqAddFrm_clicked();
//    void on_btnSeqRemFrm_clicked();
//    void on_btnSeqMvUp_clicked();
//    void on_btnSeqMvDown_clicked();
//    void on_btnSeqDup_clicked();





    void on_btnImagesImport_clicked();

    void on_spbimgunk2_valueChanged(int arg1);

    void on_spbimgunk14_valueChanged(int arg1);

    //void on_tblseqfrmlst_activated(const QModelIndex &index);

    //Animation Preview stuff
//    void OnPreviewRangeChanged(int length);
//    void OnPreviewFrameChanged(int curfrm, QRectF area);

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
    QScopedPointer<FrameEditor> m_frmeditor;

    //SpriteScene             m_previewrender;
    QString                 m_lastSavePath;
    QPersistentModelIndex   m_cursprite;
    QScopedPointer<QDataWidgetMapper> m_frmdatmapper;
    QScopedPointer<QDataWidgetMapper> m_imgdatmapper;
    QSettings               m_settings;


    static const QString PaletteFilterString()
    {
        static const QString filter = spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::RIFF_Pal) +
                                      ";;" +
                                      spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::TEXT_Pal) +
                                      ";;" +
                                      spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::GIMP_PAL);
        return filter;
    }

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
};


#endif // MAINWINDOW_H
