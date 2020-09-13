#if 0
#ifndef SCENERENDERER_HPP
#define SCENERENDERER_HPP
/*
scenerenderer.hpp
description: Manages scenes used by the main interface for displaying sprite animations, and frame assembly.
*/

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QScopedPointer>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QGradient>
#include <QBitmap>

#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/spritemanager.hpp>

//=============================================================================
//  AnimatedSpriteItem
//=============================================================================
class AnimatedSpriteItem : public QGraphicsObject
{
    Q_OBJECT
public:
    struct cachedanimfrm_t
    {
        QPixmap img;
        QRect   area;
        int     duration;
        int     shadowx;
        int     shadowy;
        int     offsetx;
        int     offsety;
    };

    AnimatedSpriteItem(Sprite * pspr, fmt::AnimDB::animseqid_t seqid, bool bshouldloop);
    ~AnimatedSpriteItem();

    void Init();
    void UpdateFrame();
    //Dumps all rendered animation frames on another thread
    QFuture<QVector<QImage>> DumpSequence()const;
    void LoadSequence();

    //This starts the loading of a sequence on another thread without blocking the caller
    void ScheduleSequenceLoad();
    //This starts the updating of the current frame on another thread without blocking the caller
    void ScheduleFrameUpdate();

    //This is meant to be used by anything that wants to delete the object, so its not deleted while loading on another thread
    void WaitStop();

    fmt::AnimDB::animseqid_t getAnimationSequenceID()const{return m_curseqid; }
    const Sprite * getSprite()const{return m_spr; }
    Sprite * getSprite(){return m_spr; }

private:
    //Helper for waiting on async processes to complete
    void WaitForLoad();
    void WaitForUpdate();

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QFutureWatcher<void> m_seqloadwatch;
    QFuture<void>        m_seqloadupdate;

    QFutureWatcher<void> m_frmupdatewatch;
    QFuture<void>        m_frmupdate;

    Sprite         * m_spr;
    fmt::AnimDB::animseqid_t m_curseqid;

    QMutex           m_mtxcache; //mutex for the cached frames
    QVector<cachedanimfrm_t> m_cachedframes;

    QMutex          m_mtxseqload; //Mutex for setting up loading a new sequence

    int              m_curfrm;
    unsigned int     m_ticksnextfrm;
    bool             m_bstopping;
    bool             m_bshouldloop;
    std::atomic_bool m_loading;
    QRect            m_biggestFrame;

signals:
    void framechanged(int/*frmidx*/);
    void framechanged(QRectF);
    void rangechanged(int,int);
    void loadfinished();
    void loading();
    void loopcomplete();

public slots:
    //Called every ticks, the sprite decides if it needs to do anything
    void tick(unsigned int curticks);
    //Rebuilds the cached frames!
    void reload();
    void setloop(bool bshouldloop);
    void framesCached();
    void frameupdated();
    void setCurFrame(int frmid);
};

//=============================================================================
//  SceneRenderer
//=============================================================================
class SceneRenderer : public QObject
{
    Q_OBJECT
public:
    explicit SceneRenderer(bool bshouldloop, QObject *parent = 0);
    virtual ~SceneRenderer();

    void clearScene();
    void setScene( Sprite * spr, fmt::AnimDB::animseqid_t id );
    void Reset();

    AnimatedSpriteItem * getAnimSprite() {return m_animsprite;}
    void setShouldLoop(bool state) {emit setloop(state); m_shouldLoop = state; }

    inline QGraphicsScene       & getAnimScene()        {return m_animScene;}
    inline const QGraphicsScene & getAnimScene()const   {return m_animScene;}

    QVector<QImage> DumpSequence()const;

    QColor getSpriteBGColor()const;

    //Those should be used to set/unset a scene's data
    void InstallAnimPreview(QGraphicsView * pview, Sprite * pspr, AnimSequence * paniseq);
    void UninstallAnimPreview(QGraphicsView * pview);

private:
    bool                   m_shouldLoop;
    QScopedPointer<QTimer> m_timer;
    QGraphicsScene         m_animScene;
    QGraphicsScene         m_frameScene;
    Sprite * m_spr;

    static const int TICK_RATE = 24;
    AnimatedSpriteItem   * m_animsprite;
    unsigned int           m_ticks;
    fmt::AnimDB::animseqid_t m_seqid;

signals:

    //Signals to external controls from the animated sprite
    void framechanged(int);
    void rangechanged(int,int);

    //Signals to the animated sprite
    void setCurFrm(int);
    void tick(unsigned int);
    void setloop(bool);

public slots:
    void startAnimUpdates();
    void stopAnimUpdates();
    void doTick();
    void loopChanged(bool state);
    void reloadAnim();
    void loopComplete();
    void OnFrameChanged( int curfrm );
    void OnRangeChanged( int min, int max );
    void setCurrentFrame(int frmid);
    /***
     * This is called when the current animation's data changes.
    */
    void OnAnimDataChaged();

};

#endif // SCENERENDERER_HPP
#endif
