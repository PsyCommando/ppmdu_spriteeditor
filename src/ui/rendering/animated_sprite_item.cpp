#include "animated_sprite_item.hpp"

const int DEF_BOUNDING_POS_X  = -256;
const int DEF_BOUNDING_POS_Y  = -256;
const int DEF_BOUNDING_WIDTH  = 1024;
const int DEF_BOUNDING_HEIGHT = 1024;


AnimatedSpriteItem::AnimatedSpriteItem()
{}

AnimatedSpriteItem::AnimatedSpriteItem(QVector<cachedframe> &&cachedframes, QVector<QColor> && palette)
    : m_cachedframes(cachedframes), m_cachedpal(palette)
{
    //compute the duration
    for(const cachedframe & c : cachedframes )
        m_cachedDuration += c.duration;
}

AnimatedSpriteItem::~AnimatedSpriteItem()
{
}

void AnimatedSpriteItem::Reset()
{
    m_curfrm = 0;
    if(m_curfrm < m_cachedframes.size())
        m_ticksnextfrm  = m_cachedframes[m_curfrm].duration;
    else
        m_ticksnextfrm = 0;
    m_tickselapsed = 0;
    emit playback_complete(); //Don't forget to notify we're back to the first frame!
}

QVector<QImage> AnimatedSpriteItem::DumpFrames()const
{
    if(m_cachedframes.empty())
    {
        qInfo("AnimatedSpriteItem::DumpSequence():No sequence was loaded! Nothing to export!\n");
        return qMove(QFuture<QVector<QImage>>());
    }

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
        outimgs.push_back(m_cachedframes[curfrm].img.toImage());
    }
    return outimgs;
}

QRectF AnimatedSpriteItem::boundingRect() const
{
    QPointF mypos = this->pos();
    if( m_curfrm >= m_cachedframes.size())
        return QRectF(mypos.x(), mypos.y(),32.0, 32.0 );
    QRectF bbox;
    bbox.setX(mypos.x());
    bbox.setY(mypos.y());
    bbox.setWidth(m_cachedframes[m_curfrm].img.width());
    bbox.setHeight(m_cachedframes[m_curfrm].img.height());
//    bbox.setX(DEF_BOUNDING_POS_X);
//    bbox.setY(DEF_BOUNDING_POS_Y);
//    bbox.setWidth(DEF_BOUNDING_WIDTH);
//    bbox.setHeight(DEF_BOUNDING_HEIGHT);
    return bbox;
}

void AnimatedSpriteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    if(m_cachedframes.empty())
        return;
    int curfrm = m_curfrm;

    //painter->setCompositionMode (QPainter::CompositionMode_Source);
    //painter->fillRect(painter->viewport(), m_cachedpal.front());
    //painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

    painter->setBackground(QBrush(m_cachedpal.front()));
    //painter->setBackgroundMode(Qt::BGMode::OpaqueMode);

    int xdiff = 0;// (m_cachedframes[curfrm].area.x() - m_biggestFrame.x());
    int ydiff = 0; //(m_cachedframes[curfrm].area.y() - m_biggestFrame.y());

    painter->drawPixmap( m_cachedframes[curfrm].offsetx + xdiff,
                         m_cachedframes[curfrm].offsety + ydiff,
                         m_cachedframes[curfrm].img );
}


void AnimatedSpriteItem::tick(unsigned int /*curticks*/)
{
    if(m_ticksnextfrm > 0)
        --m_ticksnextfrm;
    else
        UpdateFrame();
    ++m_tickselapsed;
}

void AnimatedSpriteItem::UpdateFrame()
{
    ++m_curfrm;
    if( m_curfrm >= m_cachedframes.size() )
    {
        m_tickselapsed = 0;
        m_curfrm = 0;
        emit playback_complete(); //When we got through all our frames notify the scene!
    }
    emit frame_changed(m_curfrm, boundingRect());
    m_ticksnextfrm = m_cachedframes[m_curfrm].duration;
    //qDebug("Frame Updated to %d, next in %d ticks\n",  m_curfrm, m_ticksnextfrm);
}

void AnimatedSpriteItem::setCurFrame(int frmid)
{
    if(frmid < m_cachedframes.size())
    {
        m_curfrm       = frmid;
        m_ticksnextfrm = m_cachedframes[m_curfrm].duration;
        m_tickselapsed = 0;
        for(int i = 0; i < m_curfrm; ++i)
            m_tickselapsed += m_cachedframes[i].duration;
        emit frame_changed(m_curfrm, boundingRect());
    }
}
