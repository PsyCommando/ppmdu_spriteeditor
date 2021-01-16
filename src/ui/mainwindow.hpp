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
#include <src/ui/editor/frame/frame_editor.hpp>
#include <src/data/content_manager.hpp>
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/ui/windows/dialogabout.hpp>
#include <src/ui/windows/dialogprogressbar.hpp>
#include <atomic>

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
    void writeSettings(QSettings & settings);
    void readSettings(QSettings & settings);

    //Internal processing stuff
    void HideAllTabs();
    void ShowATab(QWidget * ptab);
    void ShowATab(BaseSpriteTab *ptab, const QModelIndex &element);

    void DisplayTabForElement(const QModelIndex & index);
    void DisplayTabForElement(TreeNode * item);
    void DisplayStartScreen();

    //Basically do house keeping stuff when we load something new in the app
    void PrepareForNewContainer();
    void SetupContent();
    void SetupMenu();
    void SetupStatusBar();

    void LoadContainer(const QString & path);
    void SaveContainer(const QString & path);
    void ExportContainer(const QString & path, const QString &exportType);
    void ImportContainer(const QString & path);
    void CloseContainer();
    void NewContainer(QString cnttype = QString());

    //Show a warning message box with the specified content
    void Warn(const QString & title, const QString & text);

    void ShowStatusMessage(const QString & msg);
    void ShowStatusErrorMessage(const QString & msg);

    void HandleItemRemoval(QModelIndex spriteidx);

    void selectTreeViewNode(const TreeNode * node);
    void selectTreeViewNode(const QModelIndex &index);
    const QPixmap & getDefaultImage()const {return m_imgNoImg;}

public slots:
    void ShowProgressDiag(QFuture<void> & task);
    void updateCoordinateBar(const QPointF &pos);
    void updateCurrentTab();

    void OnContentRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last); //Called when the rows of the model are being changed
    void OnContentRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row); //Called when the rows of the model are being changed
    void OnRowInserted(const QModelIndex &parent, int first, int last);
//    void OnActionAddTopItem();
//    void OnActionRemTopItem();

private:
    void forEachTab(std::function<void (BaseSpriteTab *)> fun);

    QPixmap RenderNoImageSvg();

    void updateActions();
    void addMultiItemActions(const QString &itemname);
    void remMultiItemActions();

    BaseContainer * getContainer();
    ContentManager & getManager();

    //Tweak the list view to better display either single sprites or pack files!!
    void setupListView();
    void SetupUIForNewContainer(BaseContainer * sprcnt);

    void setupFrameEditPageForPart(MFrame * frm,  MFramePart * part);

    void SaveAs(const QString & path);

    //Show a message box asking if we should save the changes, and return the button pressed!
    // if dosave == true means we actually run the save code if the user picked save
    int AskSaveChanges(bool dosave = true);

    void PushUndoAction(QUndoCommand * cmd);

    QMenu *makeSpriteContextMenu(QModelIndex entry);

    Sprite *            currentSprite();
    QModelIndex         currentSpriteModelIndex();
    MFrame *            currentFrame();
    Image *             currentImage();
    AnimSequence *      currentAnimSequence();
    eTreeElemDataType   currentEntryType()const;
    QModelIndexList     currentlySelectedItems()const;

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;

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

    void on_actionSprite_Sheet_Auto_Importer_triggered();

    void on_actionAdvanced_triggered();

    void on_action_Settings_triggered();



private:
    Ui::MainWindow          *ui;
    QScopedPointer<QLabel>  m_pStatusFileType;
    QScopedPointer<QLabel>  m_pStatusCoordinates;
    QScopedPointer<QLabel>  m_pStatusError;
    QPointer<QAction>       m_pActionAddCnt;
    QPointer<QAction>       m_pActionRemCnt;
    QScopedPointer<QMenu>   m_pContainerMenu;       //Menubar doesn't take ownership
    QPointer<QAction>       m_pContainerMenuAction;
    QPixmap                 m_imgNoImg;             //Image displayed when no image can be displayed in a view!
    DialogAbout             m_aboutdiag;
    DialogProgressBar       m_progress;
    QString                 m_lastSavePath;
    QPersistentModelIndex   m_curItem;
    QUndoStack              m_undoStack;
};


#endif // MAINWINDOW_H
