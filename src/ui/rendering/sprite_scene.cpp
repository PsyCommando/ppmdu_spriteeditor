#include "sprite_scene.hpp"
#include <src/ui/errorhelper.hpp>
#include <QGraphicsView>

SpriteScene::SpriteScene(QObject *parent)
    :QObject(parent)
{
    SetupSceneBackground();
}

SpriteScene::~SpriteScene()
{
    endAnimationPlayback();
}

void SpriteScene::ConnectSpriteSignals()
{
    if(!m_animsprite)
        return;
    AnimatedSpriteItem * cursprite = m_animsprite.get();
    connect(this, &SpriteScene::tick, cursprite, &AnimatedSpriteItem::tick);
    connect(this, &SpriteScene::setCurFrm, cursprite, &AnimatedSpriteItem::setCurFrame );
    connect(cursprite, &AnimatedSpriteItem::playback_complete, this, &SpriteScene::loopComplete);
    connect(cursprite, &AnimatedSpriteItem::frame_changed, this, &SpriteScene::OnFrameChanged);     //Install scene redraw signal

    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(!m_spr)
//        Q_ASSERT(false); //Crash or something
//    const SpritePropertiesModel * pmod = m_spr->model();
//    connect(pmod, &SpritePropertiesModel::dataChanged, this, &SpriteScene::OnSpriteDataChanged);

//    const AnimSequence * pseq = m_spr->getAnimSequence(m_seqid);
//    if(!pseq)
//        Q_ASSERT(false);
//    connect( pseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
}

void SpriteScene::DisconnectSpriteSignals()
{
    if(!m_animsprite)
        return;
    AnimatedSpriteItem * cursprite = m_animsprite.get();
    disconnect(this, &SpriteScene::tick, cursprite, &AnimatedSpriteItem::tick);
    disconnect(this, &SpriteScene::setCurFrm, cursprite, &AnimatedSpriteItem::setCurFrame );
    disconnect(cursprite, &AnimatedSpriteItem::playback_complete, this, &SpriteScene::loopComplete);
    disconnect(cursprite, &AnimatedSpriteItem::frame_changed, this, &SpriteScene::OnFrameChanged);

    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(!m_spr)
//        return;
//    const SpritePropertiesModel * pmod = m_spr->model();
//    disconnect(pmod, &SpritePropertiesModel::dataChanged, this, &SpriteScene::OnSpriteDataChanged);

//    const AnimSequence * pseq = m_spr->getAnimSequence(m_seqid);
//    if(!pseq)
//        Q_ASSERT(false);
//    disconnect( pseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
}

void SpriteScene::SetupSceneBackground()
{
    //X/Y axis
    QGraphicsLineItem * xaxis = m_animScene.addLine(-512,    0, 512,   0, QPen(QBrush(QColor(128,0,0)),1.5, Qt::PenStyle::DashLine) );
    QGraphicsLineItem * yaxis = m_animScene.addLine(   0, -512,   0, 512, QPen(QBrush(QColor(0,128,0)),1.5, Qt::PenStyle::DashLine) );
    xaxis->setZValue(1);
    yaxis->setZValue(1);
}

void SpriteScene::Clear()
{
    endAnimationPlayback();
    if(m_animsprite)
    {
        QMutexLocker lock(&m_mtxScene);
        DisconnectSpriteSignals();
        m_animScene.removeItem(m_animsprite.get());
        m_animsprite.reset();
    }
    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(m_spr)
//    {
//        const AnimSequence * pseq = m_spr->getAnimSequence(m_seqid);
//        if(pseq)
//            disconnect( pseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
//    }
}

void SpriteScene::Reset()
{
    Clear();
    m_ticks = 0;
}

QColor SpriteScene::getSpriteBGColor() const
{
    if(m_spr && !m_spr->getPalette().empty())
        return m_spr->getPalette().first();
    return QColor(220,220,220);
}

void SpriteScene::InstallAnimPreview(QGraphicsView *pview, const Sprite *pspr, const AnimSequence *paniseq)
{
    if(!pview || !pspr || !paniseq)
        Q_ASSERT(false);

    Reset();
    //Set the current sprite
    setSprite(pspr, paniseq);
    //Render and setup the animation
    loadAnimation();

    qDebug() << "SpriteScene::InstallAnimPreview(): Displaying animation..\n";
    pview->setScene(&getAnimScene());
    pview->centerOn(getAnimSprite());
}

void SpriteScene::UninstallAnimPreview(QGraphicsView *pview)
{
    Reset();
    pview->setScene(nullptr);
    pview->invalidateScene();
}

void SpriteScene::setSprite(const Sprite *pspr, const AnimSequence *paniseq)
{
    Reset();
    m_spr = pspr;
    if(paniseq)
        m_seqid = paniseq->nodeIndex();
    else
        m_seqid = 0;
    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(m_spr)
//        connect(paniseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
    loadAnimation();
}

void SpriteScene::setSprite(const Sprite *pspr, fmt::AnimDB::animseqid_t seqid)
{
    Reset();
    m_spr = pspr;
    m_seqid = seqid;
    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(m_spr)
//    {
//        const AnimSequence *paniseq = pspr->getAnimSequence(seqid);
//        if(paniseq)
//            connect(paniseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
//    }
    loadAnimation();
}

void SpriteScene::setSequence(const AnimSequence *paniseq)
{
    Reset();
    m_seqid = paniseq->nodeIndex();
    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(m_spr)
//        connect(paniseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
    loadAnimation();
}

void SpriteScene::setSequence(fmt::AnimDB::animseqid_t seqid)
{
    Reset();
    m_seqid = seqid;
    //SHOULD BE DONE OUTSIDE THE CLASS!!
//    if(m_spr)
//    {
//        const AnimSequence *paniseq = m_spr->getAnimSequence(seqid);
//        if(paniseq)
//            connect(paniseq->getModel(), &QAbstractItemModel::dataChanged, this, &SpriteScene::OnAnimDataChaged);
//    }
    loadAnimation();
}

size_t SpriteScene::getAnimationLength() const
{
    if(!m_spr)
        return 0;
    const AnimSequence * seq = m_spr->getAnimSequence(m_seqid);
    if(!seq)
        return 0;
    return seq->size();
}

QVector<QImage> SpriteScene::DumpSequence()const
{
    if(!m_animsprite)
        return QVector<QImage>();
    return m_animsprite->DumpFrames();
}

void SpriteScene::beginAnimationPlayback()
{
    if(m_timer)
        return;

    m_timer.reset(new QTimer);
    connect(m_timer.data(), &QTimer::timeout, this, &SpriteScene::TimerTick );
    m_timer->setInterval(TICK_RATE);
    m_timer->start();
}

void SpriteScene::endAnimationPlayback()
{
    if(!m_timer)
        return;
    m_timer->stop();
    disconnect(m_timer.data(), &QTimer::timeout, this, &SpriteScene::TimerTick );
    m_timer.reset();
}

//
void SpriteScene::TimerTick()
{
    //When the timer ticks
    //send the tick to the sprite if it exists
    emit tick(++m_ticks);
}

void SpriteScene::setShouldLoop(bool state)
{
    m_shouldLoop = state;
}

void SpriteScene::reloadAnim()
{
    Reset();
    loadAnimation();
}

void SpriteScene::loadAnimation()
{
    endAnimationPlayback();
    QMutexLocker lock(&m_mtxScene);
    try
    {
        auto rendered = m_renderer.RenderSequence(*m_spr, m_seqid);
        m_animsprite.reset(new AnimatedSpriteItem(qMove(rendered.first), qMove(rendered.second)));
        ConnectSpriteSignals();
        m_animScene.addItem(m_animsprite.get());
        m_animsprite->setScale(2.0);
        emit rangechanged(0, m_animsprite->getNbFrames());
    }
    catch (const ExBadAnimSequence & e)
    {
        ErrorHelper::getInstance().sendErrorMessage(e.what());
    }
}

void SpriteScene::loopComplete()
{
    //Tell the UI we completed a loop of the full animation
}

void SpriteScene::OnAnimDataChaged()
{
    reloadAnim();
}

void SpriteScene::OnSpriteDataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
//    if(!roles.empty() && !roles.contains(Qt::EditRole))
//        return;

    //If its empty, or if it contains the edit role, we wanna update!
    reloadAnim();
}

void SpriteScene::OnFrameChanged(int curfrm, QRectF area)
{
    m_animScene.update(area);
    //Tell the UI we changed frame so the playback cursor can be updated!!!
    emit framechanged(curfrm, area);
}

void SpriteScene::setCurrentFrame(int frmid)
{
    if(!m_animsprite)
        return;
    m_animsprite->setCurFrame(frmid);
    //Don't emit frame changed here, since the animated sprite will do it for us
}

int SpriteScene::getCurrentFrame()const
{
    if(!m_animsprite)
        return -1;
    return m_animsprite->getCurrentFrame();
}

unsigned int SpriteScene::getSequenceLengthInTicks() const
{
    return m_animsprite? m_animsprite->getDuration() : 0;
}

unsigned int SpriteScene::getTimeElapsed()const
{
    return m_animsprite? m_animsprite->getTimeElapsed() : 0;
}
