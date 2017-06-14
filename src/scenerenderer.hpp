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

#include <src/sprite.h>
#include <src/spritemanager.h>

//
//
//
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

    AnimatedSpriteItem(Sprite * pspr, fmt::AnimDB::animseqid_t seqid, bool bshouldloop)
        :QGraphicsObject(),
          m_spr(pspr),
          m_curseqid(seqid),
          m_bshouldloop(bshouldloop)
    {Init();}

    void Init()
    {
        m_curfrm        = 0;
        if(m_curfrm < m_cachedframes.size())
            m_ticksnextfrm  = m_cachedframes[m_curfrm].duration;
        else
            m_ticksnextfrm = 0;
    }

    void UpdateFrame()
    {
        if(m_loading)
            return;
        QMutexLocker lk(&m_mtxcache);
        ++m_curfrm;
        emit framechanged(m_curfrm);
        if( m_curfrm >= m_cachedframes.size() )
        {
            m_curfrm = 0;
            emit loopcomplete();
        }
        m_ticksnextfrm = m_cachedframes[m_curfrm].duration;
    }

    QFuture<QVector<QImage>> DumpSequence()const
    {
        if(m_cachedframes.empty())
        {
            qInfo("AnimatedSpriteItem::DumpSequence():No sequence was loaded! Nothing to export!\n");
            return qMove(QFuture<QVector<QImage>>());
        }
        auto lambdaDump = [&]()->QVector<QImage>
        {
            QMutexLocker lk(& const_cast<AnimatedSpriteItem*>(this)->m_mtxcache);
            QVector<QImage> outimgs;
            for(int curfrm = 0; curfrm < m_cachedframes.size(); ++curfrm)
            {
                QImage pic = QImage(1024,1024, QImage::Format::Format_ARGB32_Premultiplied);
                QPainter painter(&pic);
                int xdiff = (m_cachedframes[curfrm].area.x() - m_biggestFrame.x());
                int ydiff = (m_cachedframes[curfrm].area.y() - m_biggestFrame.y());
                painter.drawPixmap( m_cachedframes[curfrm].offsetx + xdiff,
                                    m_cachedframes[curfrm].offsety + ydiff,
                                    m_cachedframes[curfrm].img );

                outimgs.push_back(pic.copy(m_cachedframes[curfrm].offsetx + xdiff,
                                           m_cachedframes[curfrm].offsety + ydiff,
                                           m_cachedframes[curfrm].area.width(),
                                           m_cachedframes[curfrm].area.height()));
            }
            return qMove(outimgs);
        };

        return QtConcurrent::run(lambdaDump);
    }

    void LoadSequence()
    {
        QMutexLocker lk(&m_mtxcache);
        Init();
        m_cachedframes.resize(0);

        AnimSequence  * pseq = m_spr->getAnimSequence(m_curseqid);

        //Get largest frame resolution, get the diff for each smaller frames  and add half to align the imge.

        if(pseq)
        {
            QRect boundsbiggest( 512, 512, 0, 0 );

            //First pass, find largest frame
            for(const auto & afrm : *pseq )
            {
                QRect  area;
                MFrame * pfrm = m_spr->getFrame(afrm.frmidx());
                if(pfrm != nullptr )
                    area = pfrm->calcFrameBounds();

                if( boundsbiggest.x() > area.x() )
                    boundsbiggest.setX(area.x());
                if( boundsbiggest.y() > area.y() )
                    boundsbiggest.setY(area.y());

                if( boundsbiggest.width() < area.width() )
                    boundsbiggest.setWidth(area.width());
                if( boundsbiggest.height() < area.height() )
                    boundsbiggest.setHeight(area.height());
            }


            //Second pass, assemble and crop to largest frame
            for(const auto & afrm : *pseq )
            {
                QImage target;
                QRect  area;
                MFrame * pfrm = m_spr->getFrame(afrm.frmidx());
                //
                if(pfrm != nullptr )
                {
                    target = qMove(pfrm->AssembleFrame(afrm.xoffset(), afrm.yoffset(), boundsbiggest, &area));
//                    if(target.colorTable().size() > 1)
//                        target.createMaskFromColor( target.colorTable().front(), Qt::MaskOutColor );
//                    target = target.copy(area);
                }
                else
                    qDebug("AnimatedSpriteItem::LoadSequence(): Got invalid frame index %d!\n", afrm.frmidx());

                //#TODO: Removeme! #DEBUG
                //target.save( QString("./aimfrm_%1.png").arg(m_cachedframes.size()), "png");

                cachedanimfrm_t dest;
//                if( boundsbiggest.x() > area.x() )
//                    boundsbiggest.setX(area.x());
//                if( boundsbiggest.y() > area.y() )
//                    boundsbiggest.setY(area.y());

//                if( boundsbiggest.width() < area.width() )
//                    boundsbiggest.setWidth(area.width());
//                if( boundsbiggest.height() < area.height() )
//                    boundsbiggest.setHeight(area.height());


//                if(target.colorTable().size() > 1)
//                    pixm.setMask(pixm.createMaskFromColor( target.colorTable().front(), Qt::MaskOutColor ));
                dest.area     = area;
                dest.shadowx  = afrm.shadowx();
                dest.shadowy  = afrm.shadowy();
                dest.offsetx  = afrm.xoffset();
                dest.offsety  = afrm.yoffset();
                dest.duration = afrm.duration();
                dest.img      = qMove(QPixmap::fromImage(target));
                m_cachedframes.push_back(qMove(dest));
            }

            m_biggestFrame = boundsbiggest;

            for(cachedanimfrm_t & afrm : m_cachedframes )
            {
                //try to align frames
                if( afrm.area.x() > m_biggestFrame.x() )
                    afrm.area.setX(afrm.area.x() + (afrm.area.x() - m_biggestFrame.x()) ); //add the difference!
                if( afrm.area.y() > m_biggestFrame.y() )
                    afrm.area.setY(afrm.area.y() + (afrm.area.y() - m_biggestFrame.y()) ); //add the difference!
            }

        }
        else
            qCritical("AnimatedSpriteItem::LoadSequence(): Invalid sequence id %d!\n", m_curseqid);

        if(!m_cachedframes.empty())
            m_ticksnextfrm = m_cachedframes.front().duration;
    }

    void ScheduleSequenceLoad()
    {
        m_seqloadupdate = QtConcurrent::run( this, &AnimatedSpriteItem::LoadSequence );
        connect(&m_seqloadwatch, SIGNAL(finished()), this, SLOT(framesCached()));
        m_seqloadwatch.setFuture(m_seqloadupdate);
        emit loading();
        m_loading = true;
        emit framechanged(boundingRect());
        //qDebug("AnimatedSpriteItem::ScheduleSequenceLoad(): Scheduled!\n");
    }

    void ScheduleFrameUpdate()
    {
        if(m_frmupdatewatch.isRunning())
            m_frmupdatewatch.waitForFinished();

        m_frmupdate = QtConcurrent::run( this, &AnimatedSpriteItem::UpdateFrame );
        connect(&m_frmupdatewatch, SIGNAL(finished()), this, SLOT(frameupdated()));
        m_frmupdatewatch.setFuture(m_frmupdate);
        //qDebug("AnimatedSpriteItem::ScheduleFrameUpdate(): Scheduled!\n");
    }



    // QGraphicsItem interface
public:
    QRectF boundingRect() const override
    {
        if( m_loading || m_curfrm >= m_cachedframes.size())
            return QRectF(0.0, 0.0,150.0, 150.0 );
        QMutexLocker lk( &(const_cast<AnimatedSpriteItem*>(this)->m_mtxcache));
//        QRectF tmp = m_cachedframes[m_curfrm].img.rect();
//        tmp.setWidth( tmp.width() * scale() );
//        tmp.setHeight( tmp.height() * scale() );
//        return tmp;
        QRectF bbox;//(m_biggestFrame);
        bbox.setX( -512 );
        bbox.setY( -512 );
        bbox.setWidth(1024);
        bbox.setHeight(1024);
        return bbox;
        //return QRectF(m_cachedframes[m_curfrm].img.rect());
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
    {
        if(m_cachedframes.empty())
            return;
        if(m_loading)
        {
            QPen oldpen = painter->pen();
            //Show we're loading
            painter->setPen( QColor( 110, 110, 0 ) );
            painter->drawText( 0, 0, QString("Loading..") );
            painter->setPen(oldpen);
        }
        else
        {
//            QPen    oldpen      = painter->pen();
            QBrush  oldbgbrush    = painter->background();
            QMutexLocker lk(&m_mtxcache);
            int curfrm = m_curfrm;

            painter->setCompositionMode (QPainter::CompositionMode_Source);
            painter->fillRect(painter->viewport(), QColor(m_spr->getPalette().front())/*Qt::transparent*/);
            painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

//            painter->setPen( QColor( 0, 0, 0, 0 ) );
            painter->setBackground(QBrush(QColor(m_spr->getPalette().front())));
            painter->setBackgroundMode(Qt::BGMode::OpaqueMode);

//            painter->drawEllipse(m_cachedframes[curfrm].shadowx,
//                                 m_cachedframes[curfrm].shadowy,
//                                 10, 10);
            int xdiff = (m_cachedframes[curfrm].area.x() - m_biggestFrame.x());
            int ydiff = (m_cachedframes[curfrm].area.y() - m_biggestFrame.y());
            painter->drawPixmap( m_cachedframes[curfrm].offsetx + xdiff,
                                 m_cachedframes[curfrm].offsety + ydiff,
                                 m_cachedframes[curfrm].img );

//            painter->setPen(oldpen);
            //painter->setBackground(oldbgbrush);
        }
    }

private:



private:
    QFutureWatcher<void> m_seqloadwatch;
    QFuture<void>        m_seqloadupdate;

    QFutureWatcher<void> m_frmupdatewatch;
    QFuture<void>        m_frmupdate;

    Sprite         * m_spr;
    fmt::AnimDB::animseqid_t m_curseqid;

    QMutex           m_mtxcache;
    QVector<cachedanimfrm_t> m_cachedframes;

    int              m_curfrm;
    unsigned int     m_ticksnextfrm;
    bool             m_bshouldloop;
    std::atomic_bool m_loading;
    QRect            m_biggestFrame;

    //static const unsigned int NO_TICKS = 0xFFFFFFFF;

signals:
    void framechanged(int/*frmidx*/);
    void framechanged(QRectF);
    void rangechanged(int,int);

    void loadfinished();
    void loading();
    void loopcomplete();

public slots:
    //Called every ticks, the sprite decides if it needs to do anything
    void tick(unsigned int curticks)
    {
//        if(m_ticksnextfrm == NO_TICKS)
//            return;
        if(m_ticksnextfrm > 0)
            --m_ticksnextfrm;
        else
            ScheduleFrameUpdate();
    }

    //Rebuilds the cached frames!
    void reload()
    {
        Init();
    }

    void setloop(bool bshouldloop)
    {
        m_bshouldloop = bshouldloop;
    }

    void framesCached()
    {
        emit loadfinished();
        m_loading = false;
        emit framechanged(m_curfrm);
        emit framechanged(boundingRect());
        emit rangechanged(0, m_cachedframes.size());
    }

    void frameupdated()
    {
        //emit framechanged(m_curfrm);
        emit framechanged(boundingRect());
    }

    void setCurFrame(int frmid)
    {
        if(frmid < m_cachedframes.size())
            m_curfrm = frmid;
        //emit framechanged(m_curfrm);
        emit framechanged(boundingRect());
    }

};

//
//
//
class SceneRenderer : public QObject
{
    Q_OBJECT
public:
    explicit SceneRenderer(bool bshouldloop, QObject *parent = 0);

    //void              setSprite( Sprite * spr ) { m_spr = spr; }
    //Sprite          * getSprite()               { return m_spr; }
    //const Sprite    * getSprite()const          { return m_spr; }

    //void setSequenceId( fmt::AnimDB::animseqid_t id ) {}

    void setScene( Sprite * spr, fmt::AnimDB::animseqid_t id )
    {
        if(m_animsprite)
        {
            m_animScene.removeItem(m_animsprite);
            m_animsprite = nullptr;
        }

        m_animsprite = new AnimatedSpriteItem(spr, id, m_shouldLoop);
        connect(this, &SceneRenderer::tick, m_animsprite, &AnimatedSpriteItem::tick);
        //connect(this, &SceneRenderer::setloop, m_animsprite, &AnimatedSpriteItem::setloop);
        connect(this, SIGNAL(setCurFrm(int)), m_animsprite, SLOT(setCurFrame(int)) );

        connect(m_animsprite, SIGNAL(framechanged(QRectF)), &m_animScene, SLOT(update(QRectF)) );
//        connect(m_animsprite, SIGNAL(framechanged(int)), this, SLOT(OnFrameChanged(int)) );
//        connect(m_animsprite, SIGNAL(rangechanged(int,int)), this, SLOT(OnRangeChanged(int,int)) );

        connect(m_animsprite, SIGNAL(loopcomplete()), this, SLOT(loopComplete()));
        m_animsprite->ScheduleSequenceLoad();
        m_animScene.addItem(m_animsprite);
    }

    void Reset()
    {
        stopAnimUpdates();
        m_timer.reset();
        if(m_animsprite)
        {
            m_animScene.removeItem(m_animsprite);
            m_animsprite = nullptr;
        }
        m_animScene.clear();
        //m_animScene.setSceneRect( -256, -128, 512, 256 );

        //X/Y axis
        QGraphicsLineItem * xaxis = m_animScene.addLine(-512,    0, 512,   0, QPen(QBrush(QColor(128,0,0)),1.5, Qt::PenStyle::DashLine) );
        QGraphicsLineItem * yaxis = m_animScene.addLine(   0, -512,   0, 512, QPen(QBrush(QColor(0,128,0)),1.5, Qt::PenStyle::DashLine) );
        xaxis->setZValue(1);
        yaxis->setZValue(1);
    }

    AnimatedSpriteItem * getAnimSprite() {return m_animsprite;}

    void setShouldLoop(bool state) {emit setloop(state); m_shouldLoop = state; }

    inline QGraphicsScene & getAnimScene()
    {
        return m_animScene;
    }

    inline const QGraphicsScene & getAnimScene()const
    {
        return m_animScene;
    }

    QVector<QImage> DumpSequence()const
    {
        if(!m_animsprite)
        {
            qInfo("SceneRenderer::DumpSequence(): No anim sprite instanciated. No anim to dump!\n");
            return QVector<QImage>();
        }

        QFuture<QVector<QImage>> fut = m_animsprite->DumpSequence();
        return qMove(fut.result());
    }


private:
    bool                   m_shouldLoop;
    unsigned int           m_ticks;
    QScopedPointer<QTimer> m_timer;
    QGraphicsScene         m_animScene;
    QGraphicsScene         m_frameScene;
    AnimatedSpriteItem   * m_animsprite;

    Sprite * m_spr;
    fmt::AnimDB::animseqid_t m_seqid;

    static const int TICK_RATE = 24;

signals:
    void tick(unsigned int);
    void setloop(bool);
    void framechanged(int);
    void rangechanged(int,int);

    void setCurFrm(int);

public slots:
    void startAnimUpdates()
    {
        if(!m_timer)
        {
            m_timer.reset(new QTimer);
            connect(m_timer.data(), &QTimer::timeout, this, &SceneRenderer::doTick );
            m_timer->setInterval(TICK_RATE);
            m_timer->start();
        }
//        if(!m_timer.isActive())
//        {
//            connect(&m_timer, &QTimer::timeout, this, &SceneRenderer::doTick );
//            m_timer.setInterval(TICK_RATE);
//            m_timer.start();
//        }
    }

    void stopAnimUpdates()
    {
        if(m_timer)
        {
            m_timer->stop();
            m_timer.reset();
            m_ticks = 0;
            if(m_animsprite)
                m_animsprite->Init();
        }

//        if(m_timer.isActive())
//        {
//            m_timer.stop();
//            disconnect(&m_timer, &QTimer::timeout, this, &SceneRenderer::doTick );
//            m_ticks = 0;
//            if(m_animsprite)
//                m_animsprite->Init();
//        }
    }

    void doTick()
    {
        ++m_ticks;
        emit tick(m_ticks);
    }

    void loopChanged(bool state)
    {
        setShouldLoop(state);
    }

    void invalidateAnim()
    {


    }

    void loopComplete()
    {
        if(!m_shouldLoop)
            stopAnimUpdates();
    }

    void OnFrameChanged( int curfrm )
    {
        //Notify the UI that the current frame changed!
        emit framechanged(curfrm);
    }

    void OnRangeChanged( int min, int max )
    {
        emit rangechanged(min,max);
    }

    void setCurrentFrame(int frmid)
    {
        emit setCurFrm(frmid);
    }

};

#endif // SCENERENDERER_HPP
