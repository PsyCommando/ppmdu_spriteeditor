#include "frame_editor.hpp"
#include "src/ui/editor/frame/frame_editor_part.hpp"
#include <src/utility/randomgenhelper.hpp>
#include <QKeyEvent>
#include <QPointer>
#include <QGraphicsView>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>

const int FrameEditorSceneWidth  = 512;
const int FrameEditorSceneHeight = 256;
const int FrameEditorSceneXPos   = 0;
const int FrameEditorSceneYPos   = 0;

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
    QPen   midpen(QBrush(midcolor), 0.5, Qt::DotLine );

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

void FrameEditor::deInitScene()
{
    clear();
    m_parts.clear();
}

void FrameEditor::onPartMoved()
{
    FramePart * pp = qobject_cast<FramePart*>(sender());
    if(pp)
    {
        QVariant offset;
        offset.setValue<QPair<int,int>>( QPair<int,int>(qRound(pp->pos().x()),
                                                        qRound(pp->pos().y())) );
        m_pfrm->setData(
            m_pfrm->getModel()->index( m_parts.indexOf(pp),
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
    QPen penv(QColor::fromHsv( 200, 30, 70 ), 0.5);
    QPen penh(QColor::fromHsv( 220, 40, 80 ), 0.5);

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
    penv.setWidthF(0.8);
    penh.setWidthF(0.8);
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
    QPen limitpen(QBrush(limitcolor), 1.2 );
    painter->setPen(limitpen);
    painter->drawLine( 0, FrameEditorSceneHeight, FrameEditorSceneWidth, FrameEditorSceneHeight);
    painter->drawLine( FrameEditorSceneWidth, 0, FrameEditorSceneWidth, FrameEditorSceneHeight);

    painter->setPen(QPen(QBrush(limitcolor.lighter(200)), 1.3));
    painter->drawText(FrameEditorSceneWidth - 48, 12, QString(tr("MAX X:%1")).arg(FrameEditorSceneWidth));
    painter->drawText(0, FrameEditorSceneHeight, QString(tr("MAX Y(V Wrap):%1")).arg(FrameEditorSceneHeight));

    m_bgarea = rect;
}

void FrameEditor::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    /*
        Returns the distance that the wheel is rotated, in eighths (1/8s) of a degree.
        A positive value indicates that the wheel was rotated forwards away from the user;
        a negative value indicates that the wheel was rotated backwards toward the user.
        Most mouse types work in steps of 15 degrees, in which case the delta value is a multiple of 120 (== 15 * 8).
    */
    int degrees = event->delta() / 8;
    event->accept();
    emit zoom(qRound(degrees/15.0 * 50.0));
    invalidate();
}

void FrameEditor::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsScene::contextMenuEvent(event);
}

void FrameEditor::keyPressEvent(QKeyEvent *event)
{
    QGraphicsScene::keyPressEvent(event);
}

void FrameEditor::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsScene::keyReleaseEvent(event);
}


void FrameEditor::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);
}

void FrameEditor::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
}

void FrameEditor::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
}

void FrameEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}
