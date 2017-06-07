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

#include "src/spritemanager.h"
#include <src/scenerenderer.hpp>
#include <dialogabout.hpp>
#include <dialogprogressbar.hpp>
#include <atomic>

#include <src/frameeditor.hpp>

namespace Ui {
class MainWindow;
}


//struct animframe
//{
//    QPixmap         frm;
//    fmt::animfrm_t  info;
//};

//class AnimViewScene : public QObject
//{
//    friend class AnimViewerManager;
//    Q_OBJECT
//    std::atomic_bool           m_brun;
//    std::atomic_bool         & m_bloop;
//    std::atomic_int          & m_curfrm;
//    const QVector<animframe> & m_frms;
//public:
//    AnimViewScene(std::atomic_int & curfrm, const QVector<animframe> & frms, std::atomic_bool & loop)
//        :m_curfrm(curfrm),m_brun(true), m_frms(frms), m_bloop(loop)
//    {
//    }

//private:

//public slots:
//    void doWork()
//    {
//        if(m_curfrm >= m_frms.size())
//            m_curfrm = 0;
//        m_brun = true;
//        for(; m_brun.load();)
//        {
//            //qDebug("doing work!");
//            emit redrawPlz();
//            QThread::msleep(m_frms[m_curfrm].info.duration * 24);

//            ++m_curfrm;
//            if(m_curfrm >= m_frms.size() && m_bloop)
//                m_curfrm = 0;
//            else if(m_curfrm >= m_frms.size() && !m_bloop)
//                break;
//        }
//        emit finished();
//    }

//    void stopLoop()
//    {
//        m_brun = false;
//        qDebug("AnimViewScene::stopLoop():Got signal to stop animation thread!");
//    }

//    void toggleloop(bool bloop)
//    {
//        m_bloop = bloop;
//    }

//signals:
//    void redrawPlz();
//    void finished();
//    void error(QString err);

//};

//class AnimViewerManager : public QObject
//{
//    Q_OBJECT
//public:
//    AnimViewerManager(QGraphicsScene * pscene, bool bloop)
//        :m_pScene(pscene),m_pworker(nullptr), m_shouldloop(bloop),m_bisPlaying(false)
//    {
//    }

//    ~AnimViewerManager()
//    {
//        emit deleting();
//        m_mutexWorker.lock();
//        if(m_pworker)
//        {
//            m_pworker->stopLoop();
//            m_pworker->deleteLater();
//        }
//        m_mutexWorker.unlock();
//        m_workerThread.terminate();
//        if(!m_workerThread.wait(5000))
//            m_workerThread.terminate();
//    }

//    void setSequence(AnimSequence * pseq, Sprite * spr )
//    {
//        //Parse:
//        if(!pseq)
//        {
//            Q_ASSERT(false);
//            qFatal("AnimViewerManager::setSequence(): Invalid sequence ID!\n");
//            return;
//        }
//        if(!m_pScene)
//            qFatal("AnimViewerManager::setSequence(): NO SCENE!!");

//        Stop();
//        m_mutexWorker.lock();
//        m_pworker = nullptr;
//        m_anim.resize(0);
//        m_currentfrm = 0;
//        m_sprite     = nullptr;
//        m_bisPlaying = false;

//        if(pseq->getSeqLength() == 0)
//        {
//            qInfo("AnimViewerManager::setSequence(): Got empty sequence!");
//            return;
//        }

//        qDebug() << "AnimViewerManager::setSequence(): Rendering sequence..\n";
//        m_anim.reserve(pseq->getSeqLength());
//        for( const auto & anifrm : *pseq )
//        {

//            animframe   cvfrm;
//            //QImage      result(origfrm.width() + abs(anifrm.xoffs),origfrm.height() + abs(anifrm.yoffs));
//            //QPainter    paintr(result);

//            MFrame * pfrm = spr->getFrame(anifrm.frmidx());
//            if(!pfrm)
//                qFatal(":Bad frame index (%d)!", anifrm.frmidx());

//            cvfrm.info  = anifrm.exportFrame();
//            //paintr.drawPixmap( anifrm.xoffs, anifrm.yoffs, origfrm.width(), origfrm.height(), origfrm );
//            cvfrm.frm   = qMove(pfrm->AssembleFrameToPixmap(0,0));
//            m_anim.push_back(qMove(cvfrm));
//        }

//        m_sprite  = m_pScene->addPixmap(m_anim.front().frm);
//        m_pworker = new AnimViewScene(m_currentfrm, m_anim, m_shouldloop); //the object will be deleted via event
//        connect(m_pworker, &AnimViewScene::finished, this,      &AnimViewerManager::threadfinished);
//        //connect(&m_workerThread, &QThread::finished, m_pworker, &QObject::deleteLater);

//        connect(this, &AnimViewerManager::deleting, m_pworker, &QObject::deleteLater);

//        //connect(this, &AnimViewerManager::toggleloop, m_pworker, &AnimViewScene::toggleloop);
//        connect(this, &AnimViewerManager::operate, m_pworker, &AnimViewScene::doWork);
//        connect(m_pworker, &AnimViewScene::redrawPlz, this, &AnimViewerManager::redrawScene);
//        m_mutexWorker.unlock();
//        m_pworker->moveToThread(&m_workerThread);
//        qDebug() << "AnimViewerManager::setSequence(): Sequence rendered!\n";
//    }

//    void Play()
//    {
//        if(m_bisPlaying)
//            return;
//        QMutexLocker locker(&m_mutexWorker);
//        if(!m_pworker)
//            return;
//        qDebug() << "AnimViewerManager::Play(): Playback started!\n";
//        m_workerThread.start(QThread::HighPriority);
//        emit operate();
//        m_bisPlaying = true;
//    }

//    void Pause()
//    {
//        if(!m_bisPlaying)
//            return;
//        QMutexLocker locker(&m_mutexWorker);
//        if(!m_pworker)
//            return;
//        qDebug() << "AnimViewerManager::Pause(): Playback paused at frame #" <<m_currentfrm <<"!\n";
//        m_pworker->stopLoop();
//        emit stopThread();
//        m_workerThread.exit();
//        m_bisPlaying = false;
//    }

//    void Stop()
//    {
//        if(!m_bisPlaying)
//            return;
//        QMutexLocker locker(&m_mutexWorker);
//        if(!m_pworker)
//            return;
//        qDebug() << "AnimViewerManager::Reset(): Playback reset!\n";
//        m_pworker->stopLoop();
//        emit stopThread();
//        m_workerThread.exit();
//        m_currentfrm = 0;
//        m_bisPlaying = false;
//    }

//    QGraphicsPixmapItem * getPixPtr()
//    {
//        return m_sprite;
//    }

//    void loop(bool bloop)
//    {
//        m_shouldloop = bloop;
//    }

//public slots:
//     void redrawScene()
//     {
//         if( m_pScene && !m_anim.empty() && m_sprite )
//         {
//             animframe & curfrm = m_anim[m_currentfrm];
//             m_sprite->setPixmap(curfrm.frm);
//             m_sprite->setPos( curfrm.info.xoffs, curfrm.info.yoffs );
//             m_pScene->invalidate();
//         }
//         else
//             qDebug("AnimViewerManager::redrawScene(): Redrawing but no data was set to render!!");
//     }

//     void threadfinished()
//     {
//        QMutexLocker locker(&m_mutexWorker);
//        m_bisPlaying = false;
//     }

//signals:
//    void operate();
//    void stopThread();
//    void deleting();
//    //void toggleloop(bool);
//private:
//    QThread                         m_workerThread;
//    AnimViewScene                 * m_pworker;
//    std::atomic_int                 m_currentfrm;
//    QVector<animframe>              m_anim;
//    QGraphicsPixmapItem           * m_sprite;
//    QGraphicsScene                * m_pScene;
//    QMutex                          m_mutexWorker;
//    std::atomic_bool                m_shouldloop;
//    bool                            m_bisPlaying;
//};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class TVSpritesContextMenu;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Settings
    void writeSettings();
    void readSettings();

    //Internal processing stuff
    void HideAllTabs();
    void ShowATab(QWidget * ptab);
    void DisplayStartScreen();
    void DisplayPropertiesPage(Sprite * spr);
    void DisplayMFramePage(Sprite * spr, MFrame *frm);
    void DisplayAnimSequencePage(Sprite * spr, AnimSequence *aniseq);
    void DisplayAnimTablePage(Sprite * spr);
    void DisplayPalettePage(Sprite * spr);
    void DisplayEffectsPage(Sprite * spr);
    void DisplayAnimGroupPage(Sprite * spr);
    void DisplayImagePage(Sprite * spr, Image * img);
    void DisplayImageListPage(Sprite * spr, ImageContainer *pimgs, Image * img = nullptr );

    void LoadContainer( const QString & path );
    void SaveContainer( const QString & path );
    void ExportContainer( const QString & path );
    void ImportContainer( const QString & path );
    void CloseContainer();

    void OnActionAddSprite();
    void OnActionRemSprite();

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

    int AskSaveChanges()
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Icon::Question);
        msgBox.setText("Really close this file?");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        return msgBox.exec();
    }

    void Warn(const QString & title, const QString & text)
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.setText(title);
        msgBox.setInformativeText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }

    void InitAnimScene();

    void ShowStatusMessage(const QString & msg);
    void ShowStatusErrorMessage(const QString & msg);

    QMenu *makeSpriteContextMenu(QModelIndex entry);

    Sprite      * currentSprite();
    MFrame      * currentFrame();
    Image       * currentImage();
    AnimSequence* currentAnimSequence();
    eTreeElemDataType currentEntryType();
    //For specific sub-tables
    MFramePart  * currentTblFrameParts();
    Image       * currentTblImages();


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
    void on_action_Quit_triggered();

    void on_tv_sprcontent_expanded(const QModelIndex &index);

    void on_action_Save_triggered();

    void on_actionSave_As_triggered();

    void on_action_Export_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_action_Settings_triggered();

    void on_action_About_triggered();

    void on_actionNewSprite_triggered();

    void on_actionNewSprite_Pack_File_triggered();

    void on_tv_sprcontent_clicked(const QModelIndex &index);

    void on_tv_sprcontent_customContextMenuRequested(const QPoint &pos);

    void on_action_Open_triggered();

    void on_btnSeqPlay_clicked();

    void on_btnSeqStop_clicked();

    void on_chkAnimSeqLoop_toggled(bool checked);

    void on_btnImageCrop_clicked();

    void ShowProgressDiag(QFuture<void> & task)
    {
        m_progress.setFuture(task);
        m_progress.setModal(true);
        m_progress.show();
    }

    void on_tblviewImages_clicked(const QModelIndex &index);

    void on_tblframeparts_clicked(const QModelIndex &index);

    void on_btnExportPalette_clicked();

    void on_btnImportPalette_clicked();

    void on_btnEditPalette_clicked();

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

    SceneRenderer           m_previewrender;
    QString                 m_lastSavePath;
//    int                     m_idxCurFrmPart;
//    int                     m_idxCurFrame;
//    int                     m_idxCurSprite;
    QScopedPointer<QDataWidgetMapper> m_frmdatmapper;
    QSettings               m_settings;



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

//======================================================================================
//  TVSpritesContextMenu
//======================================================================================
class TVSpritesContextMenu : public QMenu
{
    Q_OBJECT

    QPersistentModelIndex m_itemidx;
    TreeElement * m_pitem;
    QPointer<MainWindow> m_pmainwindow;

public:
    TVSpritesContextMenu( MainWindow * mainwindow, const QModelIndex & item, QWidget * parent = nullptr );

    void BuildMenu();

public:
    void ShowProperties();

    void SaveDump();

    void RemoveEntry();

signals:
    void afterclosed();

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override
    {
        QWidget::closeEvent(event);
        emit afterclosed();
    }
};

#endif // MAINWINDOW_H
