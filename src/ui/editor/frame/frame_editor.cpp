#include "frame_editor.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/models/framepart_model.hpp>
#include <src/ui/editor/frame/frame_editor_part.hpp>
#include <src/utility/randomgenhelper.hpp>
#include <QKeyEvent>
#include <QPointer>
#include <QGraphicsView>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>

const int FrameEditorSceneWidth  = 512;
const int FrameEditorSceneHeight = 256;
const int FrameEditorSceneXPos   = 0;
const int FrameEditorSceneYPos   = 0;

//=======================================================================================================
//  FrameEditor
//=======================================================================================================
FrameEditor::FrameEditor(MFrame *frm, Sprite * pspr, QObject *parent)
    :QGraphicsScene(parent)
{
    m_pfrm = frm;
    m_psprite = pspr;
    m_model = new MFramePartModel(m_pfrm, pspr);
    m_attachModel.reset(new EffectSetModel(pspr->getAttachMarkers(m_pfrm->nodeIndex()), pspr));
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
    m_markers.clear();
    for( int i = 0; i < m_pfrm->nodeChildCount(); ++i )
    {
        MFramePart * ppart = static_cast<MFramePart*>(m_pfrm->nodeChild(i));
        FramePart  * gfx = new FramePart( this, QPixmap::fromImage(ppart->drawPart(m_psprite, m_bTransparency)), m_model->indexOfChildNode(m_pfrm) );
        gfx->setShowOutline(m_bDrawOutline);
        m_parts.push_back(gfx);
        addItem(gfx);
        gfx->setPos( ppart->getPartData().getXOffset(), ppart->getPartData().getYOffset() );
        connect( gfx, &EditableItem::wasDragged, this, &FrameEditor::onPartMoved );
    }

    for(int i = 0; i < m_attachModel->rowCount(QModelIndex()); ++i)
    {
        AttachMarker * mark = new AttachMarker(this, m_attachModel->index(i, 0, QModelIndex()));
        addItem(mark);
        m_markers.push_back(mark);
        connect(mark, &EditableItem::wasDragged, this, &FrameEditor::onPartMoved);
        mark->update();
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
    ln->setVisible(true);
    ln->setPen(midpen);
    ln->setZValue(m_midmarkZ);

    QGraphicsSimpleTextItem* txt = addSimpleText(QString(tr("CNTR X:%1")).arg(FrameEditorSceneWidth/2));
    txt->setPos((FrameEditorSceneWidth/2), 128);
    txt->setBrush( QBrush(midcolor.lighter(200)) );
    txt->setParentItem(ln);

    //Make the guides
    QRectF scenerect = sceneRect();
    m_xAxisGuide.reset(new QGraphicsLineItem(0, scenerect.width(), 0, -scenerect.width()));
    m_yAxisGuide.reset(new QGraphicsLineItem(scenerect.width(), 0, -scenerect.width(), 0));
    m_xAxisGuide->setPen( QPen(QBrush(QColor(200, 0, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_yAxisGuide->setPen( QPen(QBrush(QColor(0, 200, 0, 180)), 1, Qt::PenStyle::DashLine) );
    m_xAxisGuide->setVisible(false);
    m_yAxisGuide->setVisible(false);
}

void FrameEditor::deInitScene()
{
    clear();
    m_parts.clear();
    m_markers.clear();
    m_xAxisGuide.reset();
    m_yAxisGuide.reset();
}

void FrameEditor::onPartMoved()
{
    EditableItem * pp = qobject_cast<EditableItem*>(sender());
    if(pp)
    {
        QModelIndex idx = pp->getItemIndex();
        if(pp->getDataType() == eTreeElemDataType::effectOffset)
        {
            m_attachModel->setData(idx.siblingAtColumn(static_cast<int>(EffectSetModel::eColumns::XOffset)), {qRound(pp->pos().x())}, Qt::EditRole);
            m_attachModel->setData(idx.siblingAtColumn(static_cast<int>(EffectSetModel::eColumns::YOffset)), {qRound(pp->pos().y())}, Qt::EditRole);
        }
        else if(pp->getDataType() == eTreeElemDataType::framepart)
        {
            m_model->setData(idx.siblingAtColumn(static_cast<int>(eFramePartColumnsType::direct_XOffset)), {qRound(pp->pos().x())}, Qt::EditRole);
            m_model->setData(idx.siblingAtColumn(static_cast<int>(eFramePartColumnsType::direct_YOffset)), {qRound(pp->pos().y())}, Qt::EditRole);
        }
        m_xAxisGuide->setVisible(true);
        m_yAxisGuide->setVisible(true);
        m_xAxisGuide->setPos(QPointF{pp->pos().x(), 0});
        m_yAxisGuide->setPos(QPointF{0, pp->pos().y()});
        emit partMoved();
    }
}

void FrameEditor::partListChanged(MFrame *pfrm)
{
    m_pfrm = pfrm;
    initScene();
}

void FrameEditor::updateScene()
{
    bool sceneneedsrebuild = false;

    if( m_pfrm->nodeChildCount() != m_parts.size() ||
        m_attachModel->rowCount(QModelIndex()) != m_markers.size() )
    {
        qDebug("FrameEditor::updateParts(): Scene integrity has changed. Rebuilding!\n");
        initScene(); //Force scene refresh if those differ!
        return;
    }

    for(const auto & p : m_parts)
    {
        if(!p.isNull())
        {
            MFramePart * partdata = static_cast<MFramePart *>(p->partID().internalPointer());
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

    //Do markers updates
    for(int i = 0; i < m_markers.size(); ++i)
    {
        //const EffectOffsetSet * pset = static_cast<const EffectOffsetSet *>(m_attachModel->getItem(m_attachModel->index(i, 0, QModelIndex())));
        m_markers[i]->updateOffset();
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

void FrameEditor::setEditorMode(eEditorMode mode)
{
    if(mode == m_mode)
        return; //Nothing to do

    m_mode = mode;
    //Toggle part states depending on the mode
    bool bpartsEnabled = m_mode == eEditorMode::FrameParts;
    for(auto & p : m_parts)
    {
        p->setEnabled(bpartsEnabled);
    }
    bool bmarkerEnabled = m_mode == eEditorMode::AttachmentPoints;
    for(auto & p : m_markers)
    {
        p->setEnabled(bmarkerEnabled);
    }
    update();
}

void FrameEditor::deleteItem(EditableItem * item)
{
    QModelIndex idx = item->getItemIndex();
    if(idx.isValid())
    {
        QAbstractItemModel * pmodel = const_cast<QAbstractItemModel *>(idx.model()); //We own the pointer to these already, so this hack is mostly okay
        pmodel->removeRow(idx.row(), QModelIndex());
    }
    removeItem(item);
    update();
}

void FrameEditor::attachRulers(EditableItem * item)
{
    if(!item)
    {
        clearRulers();
        return;
    }
    m_rulerItem = item;
    connect(item, &EditableItem::posChanged,[this](const QPointF & pos){this->m_xAxisGuide->setPos(pos.x(), 0);});
    m_xAxisGuide->setPos(m_rulerItem->pos().x(), 0);
    m_xAxisGuide->setVisible(true);
    connect(item, &EditableItem::posChanged,[this](const QPointF & pos){this->m_yAxisGuide->setPos(0, pos.y());});
    m_yAxisGuide->setPos(0, m_rulerItem->pos().y());
    m_yAxisGuide->setVisible(true);
}

void FrameEditor::clearRulers()
{
    if(m_rulerItem)
        disconnect(m_rulerItem, &EditableItem::posChanged, nullptr, nullptr);
    m_xAxisGuide->setVisible(false);
    m_yAxisGuide->setVisible(false);
    m_rulerItem = nullptr;
}

void FrameEditor::selectMarker(QModelIndex marker)
{
    clearSelection();
    if(marker.isValid())
    {
        m_markers[marker.row()]->setSelected(true);
    }
}

void FrameEditor::drawGrid(QPainter * painter, const QRectF & rect)
{
    qreal left = rect.left() - (qRound(rect.left()) % m_gridsz );
    qreal top  = rect.top()  - (qRound(rect.top())  % m_gridsz );
    QVarLengthArray<QLineF, 256> linesx;
    QVarLengthArray<QLineF, 256> linesy;
    QPen penv(QColor::fromHsv( 200, 30, 70 ), 0.4);
    QPen penh(QColor::fromHsv( 220, 40, 80 ), 0.4);

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
    penv.setWidthF(0.6);
    penh.setWidthF(0.6);
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
}

void FrameEditor::drawLimits(QPainter * painter, const QRectF & /*rect*/)
{
    QColor limitcolor(128,128,0);
    QPen limitpen(QBrush(limitcolor), 1.0 );
    painter->setPen(limitpen);
    painter->drawLine( 0, FrameEditorSceneHeight, FrameEditorSceneWidth, FrameEditorSceneHeight);
    painter->drawLine( FrameEditorSceneWidth, 0, FrameEditorSceneWidth, FrameEditorSceneHeight);

    painter->setPen(QPen(QBrush(limitcolor.lighter(200)), 1.3));
    painter->drawText(FrameEditorSceneWidth - 48, 12, QString(tr("MAX X:%1")).arg(FrameEditorSceneWidth));
    painter->drawText(0, FrameEditorSceneHeight, QString(tr("MAX Y(V Wrap):%1")).arg(FrameEditorSceneHeight));
}

void FrameEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    painter->setBrush(QBrush( QColor::fromHsv(195, 6, 50)) );
    painter->drawRect(sceneRect());
    drawGrid(painter, rect);
    drawLimits(painter, rect);
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
    emit mousePosUpdate(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void FrameEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_xAxisGuide->setVisible(false);
    m_yAxisGuide->setVisible(false);
    QGraphicsScene::mouseReleaseEvent(event);
}
