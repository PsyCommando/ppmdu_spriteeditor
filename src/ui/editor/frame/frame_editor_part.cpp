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
FramePart::FramePart(const QPixmap &framepart, int framepartid, FrameEditor *pfrmedit, QGraphicsItem *parent)
    :QObject(nullptr),
      QGraphicsPixmapItem(framepart,parent)
{
    m_partid = framepartid;
    m_frmedit = pfrmedit;
    //        int h = 0, s = 0, v= 0;
    //        m_overlayColor.getHsv(&h, &s, &v);
    //        m_overlayColor.setHsv(h,s/2,v/2,200);
    m_overlayColor= QColor::fromHsv(GetRandomGenerator().generate() % 255, 200, 128, 200);
    m_overlayColor = m_overlayColor.lighter(100);
    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemSendsScenePositionChanges);

    //Make the tooltip display
    m_tooltip = new QGraphicsTextItem(this);
    m_tooltip->setDefaultTextColor(QColor::fromHsv(198, 128, 100, 225));
    m_tooltip->setTextInteractionFlags(Qt::NoTextInteraction);
    m_tooltip->setZValue(99);
    m_tooltip->setPos(DefXTooltip, DefYTooltip);
    QFont tooltipfnt;
    tooltipfnt.setStyleHint(QFont::Monospace);
    tooltipfnt.setFamily("mono");
    tooltipfnt.setPointSize(12);
    tooltipfnt.setBold(true);
    m_tooltip->setFont(tooltipfnt);
    connect(this, &FramePart::updateTooltipPos, m_tooltip.data(), &QGraphicsTextItem::setPlainText );
    //m_tooltip->adjustSize();

    //Add a bg rectangle for the position!
    QRectF textbg = QFontMetrics(tooltipfnt).boundingRect(m_tooltip->toPlainText());
    textbg.setX(0);
    textbg.setY(0);
    QGraphicsPolygonItem * poly = new QGraphicsPolygonItem( QPolygonF(textbg), m_tooltip);
    poly->setZValue(-1);
    poly->setPen(Qt::PenStyle::NoPen);
    poly->setBrush(QBrush(Qt::GlobalColor::lightGray));
    m_tooltip->setVisible(false);

    //Make the guides
    QRectF scenerect = pfrmedit->sceneRect();
    m_xAxisGuide = new QGraphicsLineItem(0, scenerect.width(), 0, -scenerect.width(), this);
    m_yAxisGuide = new QGraphicsLineItem(scenerect.width(), 0, -scenerect.width(), 0, this);
    m_xAxisGuide->setPen( QPen(QBrush(QColor(200, 0, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_yAxisGuide->setPen( QPen(QBrush(QColor(0, 200, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_xAxisGuide->setVisible(false);
    m_yAxisGuide->setVisible(false);
}

void FramePart::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Paint the image
    paintImage(painter, option, widget);

    //Then paint the bounds
    paintBoundingBox(painter, option, widget);
}

void FramePart::paintImage(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    if(isSelected())
    {
        //Invert the colors of the part if selected
        QImage tempimg = pixmap().toImage();
        tempimg.invertPixels();
        painter->drawImage(boundingRect(), tempimg, boundingRect());
    }
    else
        painter->drawPixmap(boundingRect(), pixmap(), boundingRect());
}

void FramePart::paintBoundingBox(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    static const qreal OUTLINE_PEN_WIDTH = 0.6;
    static const qreal OUTLINE_SHRINK = 8;
    static const QMarginsF BBOX_MARGIN(OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK);

    //If no outlines, and not selected, we don't bother
    if(!m_bshowoutline)
        return;

    QPen    boundingboxpen( QBrush(m_overlayColor), OUTLINE_PEN_WIDTH, Qt::PenStyle::DashLine );
    QRectF  bounds = boundingRect();

    //Shrink the bounds by the thickness of the pen, so we don't enlarge the image
    bounds.marginsRemoved(BBOX_MARGIN);

    //Setup the pen differently when the part is seleected
    if(isSelected())
    {
        QColor highlighted = m_overlayColor;
        highlighted.setRed(240);
        highlighted.setGreen(240);
        boundingboxpen.setColor(highlighted);
        boundingboxpen.setWidthF(0.8);
        boundingboxpen.setStyle(Qt::PenStyle::SolidLine);
    }
    painter->setPen(boundingboxpen);

    //Make it visible on similar color images
    painter->setCompositionMode(QPainter::CompositionMode_Screen);
    painter->drawRect(bounds);
}

int FramePart::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}

const QColor &FramePart::overlayColor() const {return m_overlayColor;}

QColor &FramePart::overlayColor() {return m_overlayColor;}

int FramePart::partID() const{return m_partid;}

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

void FramePart::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsPixmapItem::dropEvent(event);
}

void FramePart::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsPixmapItem::dropEvent(event);
}

void FramePart::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsPixmapItem::dropEvent(event);
}

void FramePart::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->button() == Qt::MouseButton::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);
        m_tooltip->setVisible(false);
        m_xAxisGuide->setVisible(false);
        m_yAxisGuide->setVisible(false);
        emit wasDragged();
        emit posChanged(pos());
    }
    QGraphicsPixmapItem::mouseReleaseEvent(ev);
}

void FramePart::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->button() == Qt::MouseButton::LeftButton)
    {
        setCursor(Qt::ClosedHandCursor);
        m_tooltip->setVisible(true);
        m_xAxisGuide->setVisible(true);
        m_yAxisGuide->setVisible(true);
    }
    QGraphicsPixmapItem::mousePressEvent(ev);
}

void FramePart::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //Select the part when double clicked!
    emit beingDoubleClicked();
    QGraphicsPixmapItem::mouseDoubleClickEvent(event);
}

void FramePart::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() < QApplication::startDragDistance())
        return;

    UpdateTooltip();
    setCursor(Qt::OpenHandCursor);
    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void FramePart::UpdateTooltip()
{
    if( pos().x() > XMaxThresholdTooltip )
        m_tooltip->setX(m_tooltip->boundingRect().width() * -1.0);
    else
        m_tooltip->setX(DefXTooltip);

    if( pos().y() <= YMinThresholdTooltip )
        m_tooltip->setY(pixmap().height() * -1.0);
    else
        m_tooltip->setY(DefYTooltip);

    //qDebug("x:%f, y:%f\n", m_tooltip->x(), m_tooltip->y());

    emit updateTooltipPos(QString("(x:%1, y:%2)").arg(qRound(pos().x())).arg(qRound(pos().y())));
}

QVariant FramePart::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change != ItemPositionChange)
        return QGraphicsPixmapItem::itemChange(change, value);

    QRectF scene = m_frmedit->sceneRect();
    QPointF newpos = value.toPointF();
    //Restrict it so it only moves within the allowed range!
    qreal xpos = qBound<qreal>( 0.0, newpos.x(), scene.width()-1 );
    qreal ypos = qBound<qreal>( 0.0, newpos.y(), scene.height()-1 );
    return QPointF(qRound(xpos), qRound(ypos)); //The pos has to be an integer value!

}

bool FramePart::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    return QGraphicsPixmapItem::collidesWithItem(other,mode);
}

bool FramePart::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
    return QGraphicsPixmapItem::collidesWithPath(path,mode);
}

void FramePart::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateTooltip();
    m_tooltip->setVisible(true);
    QGraphicsPixmapItem::hoverEnterEvent(event);
}

void FramePart::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_tooltip->setVisible(false);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

void FramePart::keyPressEvent(QKeyEvent *event)
{
    QGraphicsPixmapItem::keyPressEvent(event);
}

void FramePart::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsPixmapItem::keyReleaseEvent(event);
}

void FramePart::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsPixmapItem::contextMenuEvent(event);
}

