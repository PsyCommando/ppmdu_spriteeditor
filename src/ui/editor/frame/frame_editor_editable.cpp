#include "frame_editor_editable.hpp"
#include <src/ui/editor/frame/frame_editor.hpp>
#include <src/utility/randomgenhelper.hpp>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QObject>
#include <QFont>
#include <QKeyEvent>

const QColor EditableItem::DefToolTipColor {QColor::fromHsv(198, 128, 100, 225)};
const QKeySequence  DefKeyRight  {Qt::Key::Key_Right};
const QKeySequence  DefKeyLeft   {Qt::Key::Key_Left};
const QKeySequence  DefKeyUp     {Qt::Key::Key_Up};
const QKeySequence  DefKeyDown   {Qt::Key::Key_Down};
const int           DefMoveIncrements   {1};

EditableItem::EditableItem(FrameEditor *pfrmedit, QGraphicsItem * parent)
    :QObject(nullptr), QGraphicsItem(parent)
{
    m_frmedit = pfrmedit;
    m_BoundingBoxColor= QColor::fromHsv(GetRandomGenerator().generate() % 255, 200, 128, 200);
    m_BoundingBoxColor = m_BoundingBoxColor.lighter(100);
    setCursor(Qt::OpenHandCursor);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemSendsScenePositionChanges);
    SetupTooltip();
}

EditableItem::~EditableItem()
{

}

eTreeElemDataType EditableItem::getDataType() const
{
    if(!getItemIndex().isValid())
        return eTreeElemDataType::INVALID;
    const TreeNode * pnode = static_cast<const TreeNode*>(getItemIndex().internalPointer());
    return pnode? pnode->nodeDataTy() : eTreeElemDataType::INVALID;
}

void EditableItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Paint the bounding box
    paintBoundingBox(painter, option, widget);
}

void EditableItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->button() == Qt::MouseButton::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);
        m_tooltip->setVisible(false);
        emit wasDragged();
        emit posChanged(pos());
    }
    QGraphicsItem::mouseReleaseEvent(ev);
}

void EditableItem::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    if(ev->button() == Qt::MouseButton::LeftButton)
    {
        setCursor(Qt::ClosedHandCursor);
        m_tooltip->setVisible(true);
    }
    QGraphicsItem::mousePressEvent(ev);
}

void EditableItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //Select the part when double clicked!
    emit beingDoubleClicked();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void EditableItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() < QApplication::startDragDistance())
        return;

    UpdateTooltip();
    setCursor(Qt::OpenHandCursor);
    QGraphicsItem::mouseMoveEvent(event);
}

void EditableItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateTooltip();
    m_tooltip->setVisible(true);
    QGraphicsItem::hoverEnterEvent(event);
}

void EditableItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_tooltip->setVisible(false);
    QGraphicsItem::hoverLeaveEvent(event);
}

void EditableItem::keyPressEvent(QKeyEvent *event)
{
    QPointF mypos = pos();
    switch(event->key())
    {
    case Qt::Key::Key_Up:
        {
            changeMyPos(mypos.x(), mypos.y() + DefMoveIncrements);
            break;
        }
    case Qt::Key::Key_Down:
        {
            changeMyPos(mypos.x(), mypos.y() - DefMoveIncrements);
            break;
        }
    case Qt::Key::Key_Left:
        {
            changeMyPos(mypos.x() - DefMoveIncrements, mypos.y());
            break;
        }
    case Qt::Key::Key_Right:
        {
            changeMyPos(mypos.x() + DefMoveIncrements, mypos.y());
            break;
        }
    };

    QGraphicsItem::keyPressEvent(event);
}

void EditableItem::keyReleaseEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Delete))
    {
        m_frmedit->deleteItem(this);
    }
    QGraphicsItem::keyReleaseEvent(event);
}

void EditableItem::UpdateTooltip()
{
    if( pos().x() > XMaxThresholdTooltip )
        m_tooltip->setX(m_tooltip->boundingRect().width() * -1.0);
    else
        m_tooltip->setX(DefXTooltip);

    if( pos().y() <= YMinThresholdTooltip )
        m_tooltip->setY(boundingRect().height() * -1.0);
    else
        m_tooltip->setY(DefYTooltip);

    //qDebug("x:%f, y:%f\n", m_tooltip->x(), m_tooltip->y());

    emit updateTooltipPos(QString("(x:%1, y:%2)").arg(qRound(pos().x())).arg(qRound(pos().y())));
}

void EditableItem::paintBoundingBox(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    static const qreal OUTLINE_PEN_WIDTH = 0.6;
    static const qreal OUTLINE_SHRINK = 8;
    static const QMarginsF BBOX_MARGIN(OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK,OUTLINE_PEN_WIDTH*OUTLINE_SHRINK);

    //If no outlines, and not selected, we don't bother
    if(!shouldShowBoundingBox())
        return;

    QPen    boundingboxpen( QBrush(m_BoundingBoxColor), OUTLINE_PEN_WIDTH, Qt::PenStyle::DashLine );
    QRectF  bounds = boundingRect();

    //Shrink the bounds by the thickness of the pen, so we don't enlarge the image
    bounds.marginsRemoved(BBOX_MARGIN);

    //Setup the pen differently when the part is seleected
    if(isSelected())
    {
        QColor highlighted = m_BoundingBoxColor;
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

void EditableItem::changeMyPos(int x, int y)
{
    setPos(x, y);
    emit posChanged(pos());
}

bool EditableItem::shouldShowBoundingBox() const
{
    return true;
}

QVariant EditableItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change != ItemPositionChange)
        return QGraphicsItem::itemChange(change, value);

    QRectF scene = m_frmedit->sceneRect();
    QPointF newpos = value.toPointF();
    //Restrict it so it only moves within the allowed range!
    qreal xpos = qBound<qreal>( 0.0, newpos.x(), scene.width()-1 );
    qreal ypos = qBound<qreal>( 0.0, newpos.y(), scene.height()-1 );
    return QPointF(qRound(xpos), qRound(ypos)); //The pos has to be an integer value!
}

void EditableItem::SetupTooltip()
{
    //Make the tooltip display
    m_tooltip = new QGraphicsTextItem(this); //parented, will be destroyed automatically
    m_tooltip->setDefaultTextColor(DefToolTipColor);
    m_tooltip->setTextInteractionFlags(Qt::NoTextInteraction);
    m_tooltip->setZValue(DefZTooltip);
    m_tooltip->setPos(DefXTooltip, DefYTooltip);

    QFont tooltipfnt;
    tooltipfnt.setStyleHint(QFont::Monospace);
    tooltipfnt.setFamily("mono");
    tooltipfnt.setPointSize(8);
//    tooltipfnt.setBold(true);
    m_tooltip->setFont(tooltipfnt);
    connect(this, &EditableItem::updateTooltipPos, m_tooltip.data(), &QGraphicsTextItem::setPlainText);

    QRectF textbg = QFontMetrics(tooltipfnt).boundingRect(m_tooltip->toPlainText());
    textbg.setX(0);
    textbg.setY(0);
    QGraphicsPolygonItem * poly = new QGraphicsPolygonItem( QPolygonF(textbg), m_tooltip);
    poly->setZValue(-1);
    poly->setPen(Qt::PenStyle::NoPen);
    poly->setBrush(QBrush(Qt::GlobalColor::lightGray));
    m_tooltip->setVisible(false);
}
