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
FramePart::FramePart(FrameEditor * pfrmedit, const QPixmap & framepart, const QModelIndex & itemidx, QGraphicsItem *parent)
     :parent_t(pfrmedit, itemidx, parent)
{
    m_pPart = static_cast<MFramePart*>(itemidx.internalPointer());
    Q_ASSERT(m_pPart);
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
    if(getFrameEditor()->getEditorMode() != eEditorMode::FrameParts)
    {
        //Darken the image if in another mode
        QImage tempimg = m_pixmap.toImage().convertToFormat(QImage::Format::Format_Grayscale8);
        painter->drawImage(boundingRect(), tempimg, boundingRect());
    }
    else if(isSelected())
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

MFramePart *FramePart::getPart()
{
    return m_pPart;
}

const MFramePart *FramePart::getPart() const
{
    return m_pPart;
}

bool FramePart::matchPart(const MFramePart* part)const
{
    return part == m_pPart;
}

void FramePart::updateOffset()
{
    setPos(m_pPart->getXOffset(), m_pPart->getYOffset());
}

void FramePart::commitOffset(QAbstractItemModel *model)
{
    QModelIndex myidx = model->index(m_pPart->nodeIndex(), 0);
    model->setData(myidx.siblingAtColumn(static_cast<int>(eFramePartColumnsType::XOffset)), {qRound(pos().x())}, Qt::EditRole);
    model->setData(myidx.siblingAtColumn(static_cast<int>(eFramePartColumnsType::YOffset)), {qRound(pos().y())}, Qt::EditRole);
}

//QModelIndex FramePart::partID() const
//{
//    return m_partid;
//}

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

QString FramePart::getItemDisplayName() const
{
    return m_pPart->nodeDisplayName();
}

bool FramePart::shouldShowBoundingBox() const
{
    return m_bshowoutline;
}
