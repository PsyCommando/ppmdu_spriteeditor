#include "frame_editor_marker.hpp"
#include <src/data/sprite/effectoffsetset.hpp>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

const QString AttachMarker::PosMarker{'+'};
const QColor ColorSelected  {128, 128, 0};
const QColor ColorNormal    {255,   0, 0};
const QPoint NameOffset     {8,0};


AttachMarker::AttachMarker(FrameEditor * frmedit, const QModelIndex & idx, QGraphicsItem * parent)
    :parent_t(frmedit, parent), m_txtitem(this)
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
    //Add in the margin around the middle marker, since markers are aligned from the middle, not the top corner
    QPointF center = boundingRect().center();
    setPos(m_poffset->getX() + (256 - center.x()), m_poffset->getY() + (256 - center.y()));
}
