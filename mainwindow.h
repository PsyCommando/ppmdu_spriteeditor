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
#include "src/spritemanager.h"
#include <src/scenerenderer.hpp>
#include <atomic>

namespace Ui {
class MainWindow;
}


struct animframe
{
    QPixmap         frm;
    fmt::animfrm_t  info;
};

class AnimViewScene : public QObject
{
    friend class AnimViewerManager;
    Q_OBJECT
    std::atomic_bool           m_brun;
    std::atomic_bool         & m_bloop;
    std::atomic_int          & m_curfrm;
    const QVector<animframe> & m_frms;
public:
    AnimViewScene(std::atomic_int & curfrm, const QVector<animframe> & frms, std::atomic_bool & loop)
        :m_curfrm(curfrm),m_brun(true), m_frms(frms), m_bloop(loop)
    {
    }

private:

public slots:
    volatile void doWork()
    {
        if(m_curfrm >= m_frms.size())
            m_curfrm = 0;
        m_brun = true;
        for(; m_brun.load();)
        {
            //qDebug("doing work!");
            emit redrawPlz();
            QThread::msleep(m_frms[m_curfrm].info.duration * 24);

            ++m_curfrm;
            if(m_curfrm >= m_frms.size() && m_bloop)
                m_curfrm = 0;
            else if(m_curfrm >= m_frms.size() && !m_bloop)
                break;
        }
        emit finished();
    }

    void stopLoop()
    {
        m_brun = false;
        qDebug("AnimViewScene::stopLoop():Got signal to stop animation thread!");
    }

    void toggleloop(bool bloop)
    {
        m_bloop = bloop;
    }

signals:
    void redrawPlz();
    void finished();
    void error(QString err);

};

class AnimViewerManager : public QObject
{
    Q_OBJECT
public:
    AnimViewerManager(QGraphicsScene * pscene, bool bloop)
        :m_pScene(pscene),m_pworker(nullptr), m_shouldloop(bloop),m_bisPlaying(false)
    {
    }

    ~AnimViewerManager()
    {
        emit deleting();
        m_mutexWorker.lock();
        if(m_pworker)
        {
            m_pworker->stopLoop();
            m_pworker->deleteLater();
        }
        m_mutexWorker.unlock();
        m_workerThread.terminate();
        if(!m_workerThread.wait(5000))
            m_workerThread.terminate();
    }

    void setSequence(AnimSequence * pseq, Sprite * spr )
    {
        //Parse:
        if(!pseq)
        {
            Q_ASSERT(false);
            qFatal("AnimViewerManager::setSequence(): Invalid sequence ID!\n");
            return;
        }
        if(!m_pScene)
            qFatal("AnimViewerManager::setSequence(): NO SCENE!!");

        Stop();
        m_mutexWorker.lock();
        m_pworker = nullptr;
        m_anim.resize(0);
        m_currentfrm = 0;
        m_sprite     = nullptr;
        m_bisPlaying = false;

        if(pseq->getSeqLength() == 0)
        {
            qInfo("AnimViewerManager::setSequence(): Got empty sequence!");
            return;
        }

        qDebug() << "AnimViewerManager::setSequence(): Rendering sequence..\n";
        m_anim.reserve(pseq->getSeqLength());
        for( const auto & anifrm : *pseq )
        {

            animframe   cvfrm;
            //QImage      result(origfrm.width() + abs(anifrm.xoffs),origfrm.height() + abs(anifrm.yoffs));
            //QPainter    paintr(result);

            MFrame * pfrm = spr->getFrame(anifrm.frmidx());
            if(!pfrm)
                qFatal(":Bad frame index (%d)!", anifrm.frmidx());

            cvfrm.info  = anifrm.exportFrame();
            //paintr.drawPixmap( anifrm.xoffs, anifrm.yoffs, origfrm.width(), origfrm.height(), origfrm );
            cvfrm.frm   = qMove(pfrm->AssembleFrameToPixmap(0,0));
            m_anim.push_back(qMove(cvfrm));
        }

        m_sprite  = m_pScene->addPixmap(m_anim.front().frm);
        m_pworker = new AnimViewScene(m_currentfrm, m_anim, m_shouldloop); //the object will be deleted via event
        connect(m_pworker, &AnimViewScene::finished, this,      &AnimViewerManager::threadfinished);
        //connect(&m_workerThread, &QThread::finished, m_pworker, &QObject::deleteLater);

        connect(this, &AnimViewerManager::deleting, m_pworker, &QObject::deleteLater);

        //connect(this, &AnimViewerManager::toggleloop, m_pworker, &AnimViewScene::toggleloop);
        connect(this, &AnimViewerManager::operate, m_pworker, &AnimViewScene::doWork);
        connect(m_pworker, &AnimViewScene::redrawPlz, this, &AnimViewerManager::redrawScene);
        m_mutexWorker.unlock();
        m_pworker->moveToThread(&m_workerThread);
        qDebug() << "AnimViewerManager::setSequence(): Sequence rendered!\n";
    }

    void Play()
    {
        if(m_bisPlaying)
            return;
        QMutexLocker locker(&m_mutexWorker);
        if(!m_pworker)
            return;
        qDebug() << "AnimViewerManager::Play(): Playback started!\n";
        m_workerThread.start(QThread::HighPriority);
        emit operate();
        m_bisPlaying = true;
    }

    void Pause()
    {
        if(!m_bisPlaying)
            return;
        QMutexLocker locker(&m_mutexWorker);
        if(!m_pworker)
            return;
        qDebug() << "AnimViewerManager::Pause(): Playback paused at frame #" <<m_currentfrm <<"!\n";
        m_pworker->stopLoop();
        emit stopThread();
        m_workerThread.exit();
        m_bisPlaying = false;
    }

    void Stop()
    {
        if(!m_bisPlaying)
            return;
        QMutexLocker locker(&m_mutexWorker);
        if(!m_pworker)
            return;
        qDebug() << "AnimViewerManager::Reset(): Playback reset!\n";
        m_pworker->stopLoop();
        emit stopThread();
        m_workerThread.exit();
        m_currentfrm = 0;
        m_bisPlaying = false;
    }

    QGraphicsPixmapItem * getPixPtr()
    {
        return m_sprite;
    }

    void loop(bool bloop)
    {
        m_shouldloop = bloop;
    }

public slots:
     void redrawScene()
     {
         if( m_pScene && !m_anim.empty() && m_sprite )
         {
             animframe & curfrm = m_anim[m_currentfrm];
             m_sprite->setPixmap(curfrm.frm);
             m_sprite->setPos( curfrm.info.xoffs, curfrm.info.yoffs );
             m_pScene->invalidate();
         }
         else
             qDebug("AnimViewerManager::redrawScene(): Redrawing but no data was set to render!!");
     }

     void threadfinished()
     {
        QMutexLocker locker(&m_mutexWorker);
        m_bisPlaying = false;
     }

signals:
    void operate();
    void stopThread();
    void deleting();
    //void toggleloop(bool);
private:
    QThread                         m_workerThread;
    AnimViewScene                 * m_pworker;
    std::atomic_int                 m_currentfrm;
    QVector<animframe>              m_anim;
    QGraphicsPixmapItem           * m_sprite;
    QGraphicsScene                * m_pScene;
    QMutex                          m_mutexWorker;
    std::atomic_bool                m_shouldloop;
    bool                            m_bisPlaying;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Internal processing stuff
    void HideAllTabs();
    void ShowATab(QWidget * ptab);
    void DisplayStartScreen();
    void DisplayPropertiesPage(Sprite * spr);
    void DisplayAnimFramePage(Sprite * spr);
    void DisplayAnimSequencePage(Sprite * spr, AnimSequence *aniseq);
    void DisplayAnimTablePage(Sprite * spr);
    void DisplayPalettePage(Sprite * spr);
    void DisplayEffectsPage(Sprite * spr);
    void DisplayAnimGroupPage(Sprite * spr);
    void DisplayImagePage(Sprite * spr, Image * img);
    void DisplayImageListPage(Sprite * spr, ImageContainer *pimgs);

    void LoadContainer( const QString & path );
    void SaveContainer( const QString & path );
    void ExportContainer( const QString & path );
    void ImportContainer( const QString & path );

    void OnActionAddSprite();
    void OnActionRemSprite();

private:
    void updateActions();

    //Tweak the list view to better display either single sprites or pack files!!
    void setupListView();

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

private slots:
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

    void on_actionNewSprite_triggered();

    void on_actionNewSprite_Pack_File_triggered();

    void on_tv_sprcontent_clicked(const QModelIndex &index);

    void on_tv_sprcontent_customContextMenuRequested(const QPoint &pos);

    void on_tblviewImagesTest_doubleClicked(const QModelIndex &index);

    void on_action_Open_triggered();

    void on_btnSeqPlay_clicked();

    void on_btnSeqStop_clicked();

    void on_chkAnimSeqLoop_toggled(bool checked);

    void on_btnImageCrop_clicked();

private:
    Ui::MainWindow *ui;
    QScopedPointer<QLabel>  m_pStatusFileType;
    QScopedPointer<QAction> m_pActionAddSprite;
    QScopedPointer<QAction> m_pActionRemSprite;

    //QScopedPointer<AnimViewerManager> m_curanim;
    //QGraphicsScene                    m_animscene;
    SceneRenderer                     m_previewrender;
    QString                 m_lastSavePath;
};

#endif // MAINWINDOW_H
