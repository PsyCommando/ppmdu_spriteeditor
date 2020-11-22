#include "frame_editor_marker.hpp"
#include <src/ui/editor/frame/frame_editor.hpp>
#include <src/data/sprite/effectoffsetset.hpp>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

const QString AttachMarker::PosMarker{'+'};
const QColor ColorSelected  {128, 128, 0};
const QColor ColorNormal    {255,   0, 0};
const QPoint NameOffset     {8,0};


AttachMarker::AttachMarker(FrameEditor * frmedit, const QModelIndex & idx, QGraphicsItem * parent)
    :parent_t(frmedit, idx, parent), m_txtitem(this)
{
    m_poffset = static_cast<EffectOffset*>(idx.internalPointer());
    Q_ASSERT(m_poffset);
    m_txtitem.setPlainText(PosMarker);
    m_txtitem.setDefaultTextColor(ColorNormal);
    updateOffset();
}

AttachMarker::~AttachMarker()
{
}

QRectF AttachMarker::boundingRect() const
{
    return m_txtitem.boundingRect();
}

void AttachMarker::updateOffset()
{
    const QPointF sceneCntr     = getFrameEditor()->getSceneCenter();
    const QPointF markerCenter  = getCenterPoint(); //Add in the margin around the middle marker, since markers are aligned from the middle, not the top corner
    const int absX = (sceneCntr.x() - markerCenter.x()) + m_poffset->getX();
    const int absY = (sceneCntr.y() - markerCenter.y()) + m_poffset->getY();
    setPos(absX, absY);
}

void AttachMarker::commitOffset(QAbstractItemModel *model)
{
    const QPointF & sceneCntr   = getFrameEditor()->getSceneCenter();
    const QPointF & offCntr     = getCenterPoint();
    QModelIndex     myidx       = model->index(m_poffset->nodeIndex(), 0);
    const int       relativeX   = qRound(offCntr.x() - sceneCntr.x());
    const int       relativeY   = qRound(offCntr.y() - sceneCntr.y());

    model->setData(myidx.siblingAtColumn(static_cast<int>(EffectSetModel::eColumns::XOffset)), relativeX, Qt::EditRole);
    model->setData(myidx.siblingAtColumn(static_cast<int>(EffectSetModel::eColumns::YOffset)), relativeY, Qt::EditRole);
}

QString AttachMarker::getItemDisplayName() const
{
    return m_poffset->nodeDisplayName();
}


void AttachMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    QPainter temppaint;
//    QImage   tempimg;
//    temppaint.begin(&tempimg);
//    m_txtitem.paint(&temppaint, option, widget);
//    temppaint.end();
//    if(isSelected())
//        tempimg.invertPixels();
//    painter->drawImage(0, 0, tempimg);
    parent_t::paint(painter, option, widget);
}
