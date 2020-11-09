#include "frame_editor_part.hpp"
#include "src/ui/editor/frame/frame_editor.hpp"
#include <src/utility/randomgenhelper.hpp>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

//=======================================================================================================
//  FramePart
//=======================================================================================================
FramePart::FramePart(FrameEditor * pfrmedit, const QPixmap & framepart, const QModelIndex & framepartid, QGraphicsItem *parent)
     :parent_t(pfrmedit, parent)
{
    m_partid = framepartid;
    m_pixmap = framepart;
}

void FramePart::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Paint the image
    paintImage(painter, option, widget);
    parent_t::paint(painter, option, widget);
}

void FramePart::paintImage(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    if(isSelected())
    {
        //Invert the colors of the part if selected
        QImage tempimg = m_pixmap.toImage();
        tempimg.invertPixels();
        painter->drawImage(boundingRect(), tempimg, boundingRect());
    }
    else
        painter->drawPixmap(boundingRect(), m_pixmap, boundingRect());
}

int FramePart::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}

const QColor &FramePart::overlayColor() const {return m_overlayColor;}

QColor &FramePart::overlayColor() {return m_overlayColor;}

QModelIndex FramePart::partID() const
{
    return m_partid;
}

QRectF FramePart::boundingRect() const
{
    return m_pixmap.rect();
}

void FramePart::setShowOutline(bool bshow)
{
    m_bshowoutline = bshow;
    update();
}

void FramePart::setTransparencyEnabled(bool benabled)
{
    m_bTransparency = benabled;
    update();
}

void FramePart::onPosChanged()
{
    //        setToolTip(QString("(%1, %2)")
    //                  .arg(pos().x()).arg(pos().y()));
}

