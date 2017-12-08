#include "scenerenderer.hpp"

//*****************************************************************
//  AnimatedSpriteItem
//*****************************************************************
AnimatedSpriteItem::AnimatedSpriteItem(Sprite *pspr, fmt::AnimDB::animseqid_t seqid, bool bshouldloop)
    :QGraphicsObject(),
      m_spr(pspr),
      m_curseqid(seqid),
      m_bshouldloop(bshouldloop),
      m_bstopping(false)
{Init();}

AnimatedSpriteItem::~AnimatedSpriteItem()
{

}

void AnimatedSpriteItem::Init()
{
    m_bstopping     = false;
    m_curfrm        = 0;
    if(m_curfrm < m_cachedframes.size())
        m_ticksnextfrm  = m_cachedframes[m_curfrm].duration;
    else
        m_ticksnextfrm = 0;
}

void AnimatedSpriteItem::UpdateFrame()
{
    if(m_loading || m_bstopping)
        return;
    QMutexLocker lk(&m_mtxcache);
    ++m_curfrm;
    if( m_curfrm >= m_cachedframes.size() )
    {
        m_curfrm = 0;
        emit loopcomplete();
    }
    emit framechanged(m_curfrm);
    m_ticksnextfrm = m_cachedframes[m_curfrm].duration;
    qDebug("Frame Updated to %d, next in %d ticks\n",  m_curfrm, m_ticksnextfrm);
}

QFuture<QVector<QImage> > AnimatedSpriteItem::DumpSequence() const
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
//            QImage pic = QImage(m_biggestFrame.width(),
//                                m_biggestFrame.height(),
//                                QImage::Format::Format_ARGB32_Premultiplied);
//            QPainter painter(&pic);
//            int xdiff = (m_cachedframes[curfrm].area.x() - m_biggestFrame.x());
//            int ydiff = (m_cachedframes[curfrm].area.y() - m_biggestFrame.y());
//            painter.drawPixmap( m_cachedframes[curfrm].offsetx + xdiff,
//                                m_cachedframes[curfrm].offsety + ydiff,
//                                m_cachedframes[curfrm].img );

            //outimgs.push_back(pic);
            outimgs.push_back(qMove(m_cachedframes[curfrm].img.toImage()));
        }
        return qMove(outimgs);
    };
    return QtConcurrent::run(lambdaDump);
}

void AnimatedSpriteItem::LoadSequence()
{
    Init();
    {
        QMutexLocker lk(&m_mtxcache);
        m_cachedframes.resize(0);
    }

    AnimSequence  * pseq = m_spr->getAnimSequence(m_curseqid);

    //Get largest frame resolution, get the diff for each smaller frames  and add half to align the imge.

    if(pseq)
    {
        QRect boundsbiggest;

        int x1 = 512;
        int y1 = 512;
        int x2 = 0;
        int y2 = 0;
        //First pass, find largest frame
        for(const auto & afrm : *pseq )
        {
            QRect  area;
            MFrame * pfrm = m_spr->getFrame(afrm.frmidx());
            if(pfrm != nullptr )
                area = pfrm->calcFrameBounds();

            if( x2 <= area.width() + area.x() )
                x2 = area.width() + area.x();
            if( y2 <= area.height() + area.y() )
                y2 = area.height() + area.y();

            if( x1 > area.x() )
                x1 = area.x();
            if( y1 > area.y() )
                y1 = area.y();
        }

        boundsbiggest = QRect(QPoint(x1, y1), QPoint(x2, y2) );

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
            }
            else
                qDebug("AnimatedSpriteItem::LoadSequence(): Got invalid frame index %d!\n", afrm.frmidx());

            //#TODO: Removeme! #DEBUG
            //target.save( QString("./aimfrm_%1.png").arg(m_cachedframes.size()), "png");

            cachedanimfrm_t dest;
            dest.area     = area;
            dest.shadowx  = afrm.shadowx();
            dest.shadowy  = afrm.shadowy();
            dest.offsetx  = afrm.xoffset();
            dest.offsety  = afrm.yoffset();
            dest.duration = afrm.duration();
            dest.img      = qMove(QPixmap::fromImage(target));

            {
                QMutexLocker lk(&m_mtxcache);
                m_cachedframes.push_back(qMove(dest));
            }
        }

        m_biggestFrame = boundsbiggest;

        {
            QMutexLocker lk(&m_mtxcache);
            for(cachedanimfrm_t & afrm : m_cachedframes )
            {
                //try to align frames
                if( afrm.area.x() > m_biggestFrame.x() )
                    afrm.area.setX(afrm.area.x() + (afrm.area.x() - m_biggestFrame.x()) ); //add the difference!
                if( afrm.area.y() > m_biggestFrame.y() )
                    afrm.area.setY(afrm.area.y() + (afrm.area.y() - m_biggestFrame.y()) ); //add the difference!
            }
        }

    }
    else
        qCritical("AnimatedSpriteItem::LoadSequence(): Invalid sequence id %d!\n", m_curseqid);

    {
        QMutexLocker lk(&m_mtxcache);
        if(!m_cachedframes.empty())
            m_ticksnextfrm = m_cachedframes.front().duration;
    }
}

void AnimatedSpriteItem::ScheduleSequenceLoad()
{
    if(m_bstopping)
        return;

    m_seqloadupdate = QtConcurrent::run( this, &AnimatedSpriteItem::LoadSequence );
    connect(&m_seqloadwatch, SIGNAL(finished()), this, SLOT(framesCached()));
    m_seqloadwatch.setFuture(m_seqloadupdate);
    emit loading();
    m_loading = true;
    emit framechanged(boundingRect());
    //qDebug("AnimatedSpriteItem::ScheduleSequenceLoad(): Scheduled!\n");
}

void AnimatedSpriteItem::ScheduleFrameUpdate()
{
    if(m_bstopping)
        return;

    if(m_frmupdatewatch.isRunning())
        m_frmupdatewatch.waitForFinished();

    m_frmupdate = QtConcurrent::run( this, &AnimatedSpriteItem::UpdateFrame );
    connect(&m_frmupdatewatch, SIGNAL(finished()), this, SLOT(frameupdated()));
    m_frmupdatewatch.setFuture(m_frmupdate);
    //qDebug("AnimatedSpriteItem::ScheduleFrameUpdate(): Scheduled!\n");
}

void AnimatedSpriteItem::WaitStop()
{
    m_bstopping = true;
    setloop(false);
    if(m_seqloadwatch.isRunning())
        m_seqloadwatch.waitForFinished();
    if(m_frmupdatewatch.isRunning())
        m_frmupdatewatch.waitForFinished();
}

QRectF AnimatedSpriteItem::boundingRect() const
{
    if( m_loading || m_curfrm >= m_cachedframes.size())
        return QRectF(0.0, 0.0,150.0, 150.0 );
    QMutexLocker lk( &(const_cast<AnimatedSpriteItem*>(this)->m_mtxcache));
    //        QRectF tmp = m_cachedframes[m_curfrm].img.rect();
    //        tmp.setWidth( tmp.width() * scale() );
    //        tmp.setHeight( tmp.height() * scale() );
    //        return tmp;
    QRectF bbox;//(m_biggestFrame);
    bbox.setX( -256 );
    bbox.setY( -256 );
    bbox.setWidth(1024);
    bbox.setHeight(1024);
    return bbox;
    //return QRectF(m_cachedframes[m_curfrm].img.rect());
}

void AnimatedSpriteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
        QMutexLocker lk(&m_mtxcache);
        int curfrm = m_curfrm;

        painter->setCompositionMode (QPainter::CompositionMode_Source);
        painter->fillRect(painter->viewport(), QColor(m_spr->getPalette().front()));
        painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

        painter->setBackground(QBrush(QColor(m_spr->getPalette().front())));
        painter->setBackgroundMode(Qt::BGMode::OpaqueMode);

        int xdiff = (m_cachedframes[curfrm].area.x() - m_biggestFrame.x());
        int ydiff = (m_cachedframes[curfrm].area.y() - m_biggestFrame.y());

//        QRadialGradient radialGrad( QPointF(8,
//                                            8),
//                                    16);
//        radialGrad.setColorAt(0,    QColor(0,0,0, 200));
//        radialGrad.setColorAt(0.5,  QColor(0,0,0, 128) );
//        radialGrad.setColorAt(0.9,  QColor(0,0,0, 0) );

//        QBrush shadowbrush(radialGrad);
//        painter->setBrush(shadowbrush);
//        painter->setPen(QPen(QColor(0,0,0,0)));
//        painter->drawEllipse(m_cachedframes[curfrm].shadowx + xdiff,
//                             m_cachedframes[curfrm].shadowy + ydiff,
//                             m_cachedframes[curfrm].area.width(),
//                             m_cachedframes[curfrm].area.height());

        painter->drawPixmap( m_cachedframes[curfrm].offsetx + xdiff,
                             m_cachedframes[curfrm].offsety + ydiff,
                             m_cachedframes[curfrm].img );

    }
}

void AnimatedSpriteItem::tick(unsigned int /*curticks*/)
{
    if(m_ticksnextfrm > 0)
        --m_ticksnextfrm;
    else
        ScheduleFrameUpdate();
}

void AnimatedSpriteItem::reload()
{
    ScheduleSequenceLoad();
}

void AnimatedSpriteItem::setloop(bool bshouldloop)
{
    m_bshouldloop = bshouldloop;
}

void AnimatedSpriteItem::framesCached()
{
    emit loadfinished();
    m_loading = false;
    emit framechanged(m_curfrm);
    emit framechanged(boundingRect());
    emit rangechanged(0, m_cachedframes.size());
}

void AnimatedSpriteItem::frameupdated()
{
    //emit framechanged(m_curfrm);
    emit framechanged(boundingRect());
}

void AnimatedSpriteItem::setCurFrame(int frmid)
{
    QMutexLocker lk(&m_mtxcache);
    if(frmid < m_cachedframes.size())
    {
        m_curfrm       = frmid;
        m_ticksnextfrm = m_cachedframes[m_curfrm].duration;
    }
    //emit framechanged(m_curfrm);
    emit framechanged(boundingRect());
}



//*****************************************************************
//  SceneRenderer
//*****************************************************************
SceneRenderer::SceneRenderer(bool bshouldloop, QObject *parent)
    : QObject(parent), m_animsprite(nullptr), m_ticks(0), m_shouldLoop(bshouldloop), m_spr(nullptr), m_seqid(-1), m_timer(nullptr)
{
    Reset();
}

SceneRenderer::~SceneRenderer()
{
    clearAnimSprite();
}

void SceneRenderer::clearAnimSprite()
{
    if(m_animsprite)
    {
        disconnect(this, &SceneRenderer::tick, m_animsprite, &AnimatedSpriteItem::tick);
        disconnect(this, &SceneRenderer::setCurFrm, m_animsprite, &AnimatedSpriteItem::setCurFrame );
        disconnect(m_animsprite, qOverload<int>(&AnimatedSpriteItem::framechanged), this, &SceneRenderer::OnFrameChanged );
        disconnect(m_animsprite, &AnimatedSpriteItem::rangechanged, this, &SceneRenderer::OnRangeChanged );
        disconnect(m_animsprite, &AnimatedSpriteItem::loopcomplete, this, &SceneRenderer::loopComplete);
        m_animsprite->WaitStop();
        m_animScene.removeItem(m_animsprite);
        delete m_animsprite;
        m_animsprite = nullptr;
    }
}

void SceneRenderer::setScene(Sprite *spr, fmt::AnimDB::animseqid_t id)
{
    clearAnimSprite();
    m_animsprite = new AnimatedSpriteItem(spr, id, m_shouldLoop);
    connect(this, &SceneRenderer::tick, m_animsprite, &AnimatedSpriteItem::tick);
    connect(this, &SceneRenderer::setCurFrm, m_animsprite, &AnimatedSpriteItem::setCurFrame );

    connect(m_animsprite, qOverload<QRectF>(&AnimatedSpriteItem::framechanged), [&](QRectF r)
    {
        m_animScene.update(r);
    });

    connect(m_animsprite, qOverload<int>(&AnimatedSpriteItem::framechanged), this, &SceneRenderer::OnFrameChanged );
    connect(m_animsprite, &AnimatedSpriteItem::rangechanged, this, &SceneRenderer::OnRangeChanged );

    connect(m_animsprite, &AnimatedSpriteItem::loopcomplete, this, &SceneRenderer::loopComplete);
    m_animsprite->ScheduleSequenceLoad();
    m_animScene.addItem(m_animsprite);
}

void SceneRenderer::Reset()
{
    stopAnimUpdates();
    m_timer.reset();
    clearAnimSprite();
    m_animScene.clear();
    //m_animScene.setSceneRect( -256, -128, 512, 256 );

    //X/Y axis
    QGraphicsLineItem * xaxis = m_animScene.addLine(-512,    0, 512,   0, QPen(QBrush(QColor(128,0,0)),1.5, Qt::PenStyle::DashLine) );
    QGraphicsLineItem * yaxis = m_animScene.addLine(   0, -512,   0, 512, QPen(QBrush(QColor(0,128,0)),1.5, Qt::PenStyle::DashLine) );
    xaxis->setZValue(1);
    yaxis->setZValue(1);
}

QVector<QImage> SceneRenderer::DumpSequence() const
{
    if(!m_animsprite)
    {
        qInfo("SceneRenderer::DumpSequence(): No anim sprite instanciated. No anim to dump!\n");
        return QVector<QImage>();
    }

    QFuture<QVector<QImage>> fut = m_animsprite->DumpSequence();
    return qMove(fut.result());
}

QColor SceneRenderer::getSpriteBGColor() const
{
    if(m_spr && !m_spr->getPalette().empty())
        return m_spr->getPalette().first();
    return QColor(220,220,220);
}

void SceneRenderer::startAnimUpdates()
{
    if(!m_timer)
    {
        m_timer.reset(new QTimer);
        connect(m_timer.data(), &QTimer::timeout, this, &SceneRenderer::doTick );
        m_timer->setInterval(TICK_RATE);
        m_timer->start();
    }
}

void SceneRenderer::stopAnimUpdates()
{
    if(m_timer)
    {
        m_timer->stop();
        disconnect(m_timer.data(), &QTimer::timeout, this, &SceneRenderer::doTick );
        m_timer.reset();
        m_ticks = 0;
        if(m_animsprite)
        {
            m_animsprite->WaitStop();
            m_animsprite->Init();
        }
    }
}

void SceneRenderer::doTick()
{
    ++m_ticks;
    emit tick(m_ticks);
}

void SceneRenderer::loopChanged(bool state)
{
    setShouldLoop(state);
}

void SceneRenderer::reloadAnim()
{
    stopAnimUpdates();
    if(m_animsprite)
        m_animsprite->reload();
}

void SceneRenderer::loopComplete()
{
    if(!m_shouldLoop)
        stopAnimUpdates();
}

void SceneRenderer::OnFrameChanged(int curfrm)
{
    //Notify the UI that the current frame changed!
    emit framechanged(curfrm);
}

void SceneRenderer::OnRangeChanged(int min, int max)
{
    emit rangechanged(min,max);
}

void SceneRenderer::setCurrentFrame(int frmid)
{
    emit setCurFrm(frmid);
}
