#include "frameeditor.hpp"
#include <src/randomgenhelper.hpp>

const int FrameEditorSceneWidth  = 512;
const int FrameEditorSceneHeight = 256;
const int FrameEditorSceneXPos   = 0;
const int FrameEditorSceneYPos   = 0;

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
    m_xAxisGuide = new QGraphicsLineItem(0, FrameEditorSceneWidth, 0, -FrameEditorSceneWidth, this);
    m_yAxisGuide = new QGraphicsLineItem(FrameEditorSceneWidth, 0, -FrameEditorSceneWidth, 0, this);
    m_xAxisGuide->setPen( QPen(QBrush(QColor(200, 0, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_yAxisGuide->setPen( QPen(QBrush(QColor(0, 200, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_xAxisGuide->setVisible(false);
    m_yAxisGuide->setVisible(false);
}

void FramePart::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen boundingboxpen( QBrush(m_overlayColor), 1,Qt::PenStyle::DashLine );
    if(isSelected())
    {
        QColor highlighted = m_overlayColor;
        highlighted.setRed(240);
        highlighted.setGreen(240);
        boundingboxpen.setColor(highlighted);
        boundingboxpen.setWidthF(2.0);
    }
    //QGraphicsPixmapItem::paint(painter, option, widget); //Makes the thing crash on drag because of null-ed out option and widget I guess!
    //#TODO: Add our own overlayed stuff here!
    painter->drawPixmap(boundingRect(), pixmap(), boundingRect());

    if(!m_bshowoutline && !isSelected())
        return;

    //We want to draw the bounding box!
    QPainterPath path = shape();
    painter->setPen(boundingboxpen);

    //So it doesn't blend in the part's image, we set this to difference!
    painter->setCompositionMode(QPainter::CompositionMode_Screen);
    painter->drawPath(path);
}

int FramePart::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return Type;
}

const QColor &FramePart::overlayColor() const {return m_overlayColor;}

QColor &FramePart::overlayColor() {return m_overlayColor;}

int FramePart::partID() const{return m_partid;}

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

    QPointF newpos = value.toPointF();
    //Restrict it so it only moves within the allowed range!
    qreal xpos = qBound<qreal>( 0.0, newpos.x(), FrameEditorSceneWidth-1 );
    qreal ypos = qBound<qreal>( 0.0, newpos.y(), FrameEditorSceneHeight-1 );
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

//=======================================================================================================
//  FrameEditor
//=======================================================================================================
FrameEditor::FrameEditor(MFrame *frm, QObject *parent)
    :QGraphicsScene(parent),
      m_pfrm(frm)
{
    //setBackgroundBrush(QBrush(QColor::fromHsv(195, 6, 25)));
}

void FrameEditor::initScene(bool bdrawoutline, bool bdrawmarker, bool btransparency)
{
    m_bDrawMiddleMarker = bdrawmarker;
    m_bTransparency     = btransparency;
    m_bDrawOutline      = bdrawoutline;
    for( auto & part : m_parts )
        part->setShowOutline(m_bDrawOutline);
    initScene();
}

void FrameEditor::initScene()
{
    clear();
    setSceneRect( FrameEditorSceneXPos,
                  FrameEditorSceneYPos,
                  FrameEditorSceneWidth,
                  FrameEditorSceneHeight );
    m_parts.clear();
    for( int i = 0; i < m_pfrm->nodeChildCount(); ++i )
    {
        MFramePart * ppart = static_cast<MFramePart*>(m_pfrm->nodeChild(i));
        FramePart  * gfx = new FramePart( QPixmap::fromImage(ppart->drawPart(m_bTransparency)), i, this );
        gfx->setShowOutline(m_bDrawOutline);
        m_parts.push_back(gfx);
        addItem(gfx);
        gfx->setPos( ppart->getPartData().getXOffset(), ppart->getPartData().getYOffset() );
        connect( gfx, &FramePart::wasDragged, this, &FrameEditor::onPartMoved );
    }

    if(!views().empty())
    {
        for( auto & v : views())
            v->centerOn( 256, 256 ); //Point of interest of the scene!
    }

    if(!m_bDrawMiddleMarker)
        return; //skip on this then!

    //=== Mark Center ===
    QColor midcolor(QColor::fromHsv(40,250,180, 128));
    QPen   midpen(QBrush(midcolor), 1, Qt::DotLine );

    QGraphicsView * gv = (!views().empty())? views().first() : nullptr;
    int linebottom = (gv)? gv->rect().bottom() : FrameEditorSceneHeight * 2;


    QGraphicsLineItem * ln = addLine(FrameEditorSceneWidth/2, 0, FrameEditorSceneWidth/2, linebottom);
//    m_middleMarker.reset( new QGraphicsLineItem(,
//                                                0,
//                                                ,
//                                                ) );
    ln->setVisible(true);
    ln->setPen(midpen);
    ln->setZValue(m_midmarkZ);

    QGraphicsSimpleTextItem* txt = addSimpleText(QString(tr("CNTR X:%1")).arg(FrameEditorSceneWidth/2));
    txt->setPos((FrameEditorSceneWidth/2), 128);
    txt->setBrush( QBrush(midcolor.lighter(200)) );
    txt->setParentItem(ln);
}

void FrameEditor::onPartMoved()
{
    FramePart * pp = qobject_cast<FramePart*>(sender());
    if(pp)
    {
        QVariant offset;
        offset.setValue<QPair<int,int>>( QPair<int,int>(qRound(pp->pos().x()),
                                                        qRound(pp->pos().y())) );
        m_pfrm->setData( m_pfrm->getModel()->index( m_parts.indexOf(pp),
                                                    static_cast<int>(eFramesColumnsType::Offset),
                                                    QModelIndex() ),
                         offset );
        emit partMoved();
    }
}

void FrameEditor::partListChanged(MFrame *pfrm)
{
    m_pfrm = pfrm;
    initScene();
}

void FrameEditor::updateParts()
{
    bool sceneneedsrebuild = false;
    if( m_pfrm->nodeChildCount() != m_parts.size() )
    {
        qDebug("FrameEditor::updateParts(): Scene integrity has changed. Rebuilding!\n");
        initScene(); //Force scene refresh if those differ!
        return;
    }

    for(const auto & p : m_parts)
    {
        if(!p.isNull())
        {
            MFramePart * partdata = static_cast<MFramePart *>(m_pfrm->nodeChild(p->partID()));
            if(partdata)
                p->setPos(partdata->getPartData().getXOffset(), partdata->getPartData().getYOffset());
            else
            {
                qWarning("FrameEditor::updateParts(): Scene contains invalid parts. Needs complete rebuild!");
                sceneneedsrebuild = true;
                break;
            }
        }
    }

    if(sceneneedsrebuild)
        initScene();
}

void FrameEditor::setDrawMiddleGuide(bool bdraw)
{
    m_bDrawMiddleMarker = bdraw;
    initScene();
    update();
}

void FrameEditor::setDrawOutlines(bool bdraw)
{
    m_bDrawOutline = bdraw;
    for( auto & part : m_parts )
        part->setShowOutline(m_bDrawOutline);
    update();
}

void FrameEditor::setTransparencyEnabled(bool benabled)
{
    m_bTransparency = benabled;
    for( auto & part : m_parts )
        part->setTransparencyEnabled(benabled);
    initScene();
    update();
}

void FrameEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    painter->setBrush(QBrush( QColor::fromHsv(195, 6, 50)) );
    //        painter->setBackgroundMode(Qt::BGMode::OpaqueMode);
    //        painter->setBackground(QBrush( QColor::fromHsv(195, 6, 25)));
    painter->drawRect(sceneRect());


    qreal left = rect.left() - (qRound(rect.left()) % m_gridsz );
    qreal top  = rect.top()  - (qRound(rect.top())  % m_gridsz );

    //const int NbLinesXAxis = qRound(sceneRect().width()) / m_gridsz;
    //const int NbLinesYAxis = FrameEditorSceneHeight / m_gridsz;
    QVarLengthArray<QLineF, 256> linesx;
    QVarLengthArray<QLineF, 256> linesy;
    QPen penv(QColor::fromHsv( 200, 30, 70 ));
    QPen penh(QColor::fromHsv( 220, 40, 80 ));

    //=== Draw minor lines every 8 pixels ===
    for (qreal x = left; x < rect.right(); x += m_gridsz)
        linesx.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += m_gridsz)
        linesy.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(penv);
    painter->drawLines(linesx.data(), linesx.size());

    painter->setPen(penh);
    painter->drawLines(linesy.data(), linesy.size());

    //=== Draw major lines every 16 pixels ===
    penv.setColor(QColor::fromHsv( 5, 50, 70 ));
    penh.setColor(QColor::fromHsv( 10, 60, 80 ));
    linesx.clear();
    linesy.clear();
    for (qreal x = left; x < rect.right(); x += m_gridsz*2)
        linesx.append(QLineF(x, rect.top(), x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += m_gridsz*2)
        linesy.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(penv);
    painter->drawLines(linesx.data(), linesx.size());

    painter->setPen(penh);
    painter->drawLines(linesy.data(), linesy.size());

    //=== Mark limits ===
    QColor limitcolor(128,128,0);
    QPen limitpen(QBrush(limitcolor), 2 );
    painter->setPen(limitpen);
    painter->drawLine( 0, FrameEditorSceneHeight, FrameEditorSceneWidth, FrameEditorSceneHeight);
    painter->drawLine( FrameEditorSceneWidth, 0, FrameEditorSceneWidth, FrameEditorSceneHeight);

    painter->setPen(QPen(QBrush(limitcolor.lighter(200)), 2));
    painter->drawText(FrameEditorSceneWidth - 48, 12, QString(tr("MAX X:%1")).arg(FrameEditorSceneWidth));
    painter->drawText(0, FrameEditorSceneHeight, QString(tr("MAX Y(V Wrap):%1")).arg(FrameEditorSceneHeight));

    m_bgarea = rect;
}

void FrameEditor::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    /*
Returns the distance that the wheel is rotated, in eighths (1/8s) of a degree. A positive value indicates that the wheel was rotated forwards away from the user; a negative value indicates that the wheel was rotated backwards toward the user.
Most mouse types work in steps of 15 degrees, in which case the delta value is a multiple of 120 (== 15 * 8).
        */
    int degrees = event->delta() / 8;
    event->accept();
    emit zoom(qRound(degrees/15.0 * 50.0));
    invalidate();
}

void FrameEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent */*event*/)
{
}

void FrameEditor::keyPressEvent(QKeyEvent */*event*/)
{
}

void FrameEditor::keyReleaseEvent(QKeyEvent */*event*/)
{
}
