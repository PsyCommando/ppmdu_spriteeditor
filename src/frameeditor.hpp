#ifndef FRAMEEDITOR_HPP
#define FRAMEEDITOR_HPP
/*
frameeditor.hpp

Helper for managing the QGraphicsScene used for displaying and editing the parts of a frame!
*/
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsSceneDragDropEvent>
#include <QImage>
#include <QDrag>
#include <QVariant>
#include <QApplication>
#include <QBitmap>
#include <QPen>
#include <QBrush>
#include <QGraphicsTextItem>
#include <QGraphicsObject>
#include <QGraphicsPolygonItem>
#include <QGraphicsView>
#include <QMenu>

#include <src/sprite_img.hpp>
#include <src/sprite.h>

class FrameEditor;

extern const int FrameEditorSceneWidth  ;
extern const int FrameEditorSceneHeight ;
extern const int FrameEditorSceneXPos   ;
extern const int FrameEditorSceneYPos   ;

//class FrameToolTip : public QGraphicsTextItem
//{
//public:
//    using QGraphicsTextItem::QGraphicsTextItem;

//protected:
//    void setText(const QString & txt)
//    {
//        setPlainText(txt);
//    }
//};



//
//
//
class FramePart : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

    static const int XMaxThresholdTooltip = 370;
    static const int XMinThresholdTooltip = 0;
    static const int YMaxThresholdTooltip = 255;
    static const int YMinThresholdTooltip = 15;

    static const int DefXTooltip = 0;
    static const int DefYTooltip = -25;
    static const int FarRightXTooltip = -80;
    static const int FarLeftXTooltip = -80;


public:
    static const int FramePartType = UserType + 1;
    enum { Type = FramePartType };

    FramePart(const QPixmap & framepart, int framepartid, FrameEditor * pfrmedit, QGraphicsItem *parent = nullptr)
        :QObject(nullptr),
          QGraphicsPixmapItem(framepart,parent),
          m_partid(framepartid),
          m_frmedit(pfrmedit)
    {
//        int h = 0, s = 0, v= 0;
//        m_overlayColor.getHsv(&h, &s, &v);
//        m_overlayColor.setHsv(h,s/2,v/2,200);
        m_overlayColor= QColor::fromHsv(qrand() % 255, 200, 128, 200);
        m_overlayColor = m_overlayColor.light(100);
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



    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)override
    {
        QPen boundingboxpen( QBrush(m_overlayColor), 1,Qt::PenStyle::DashLine );
        if(isSelected())
            boundingboxpen.setColor(m_overlayColor.light(150));
        //QGraphicsPixmapItem::paint(painter, option, widget); //Makes the thing crash on drag because of null-ed out option and widget I guess!
        //#TODO: Add our own overlayed stuff here!

        painter->drawPixmap(boundingRect(), pixmap(), boundingRect());

        //We want to draw the bounding box!
        QPainterPath path = shape();
        painter->setPen(boundingboxpen);

        //So it doesn't blend in the part's image, we set this to difference!
        painter->setCompositionMode(QPainter::CompositionMode_ColorDodge);
        painter->drawPath(path);
    }

    virtual int type() const override
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }

    inline const QColor & overlayColor()const {return m_overlayColor;}
    inline QColor & overlayColor() {return m_overlayColor;}

    inline int partID()const{return m_partid;}

public:

    void onPosChanged()
    {
//        setToolTip(QString("(%1, %2)")
//                  .arg(pos().x()).arg(pos().y()));
    }

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override
    {
        QGraphicsPixmapItem::dropEvent(event);
    }

    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override
    {
        QGraphicsPixmapItem::dropEvent(event);
    }

    void dropEvent(QGraphicsSceneDragDropEvent *event) override
    {
        QGraphicsPixmapItem::dropEvent(event);
    }



    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)override
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
        else if(ev->button() == Qt::MouseButton::RightButton)
        {
            QScopedPointer<QMenu> pmenu( new QMenu() );
            pmenu->addAction( tr("Change Image.."), [&]()
            {
                pmenu->close();

            });

        }
        QGraphicsPixmapItem::mouseReleaseEvent(ev);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *ev)override
     {
        if(ev->button() == Qt::MouseButton::LeftButton)
        {
            setCursor(Qt::ClosedHandCursor);
            m_tooltip->setVisible(true);
            m_xAxisGuide->setVisible(true);
            m_yAxisGuide->setVisible(true);
        }
        else if(ev->button() == Qt::MouseButton::RightButton)
        {

        }
        QGraphicsPixmapItem::mousePressEvent(ev);
     }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)override
    {
        //Select the part when double clicked!
        emit beingDoubleClicked();
        QGraphicsPixmapItem::mouseDoubleClickEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override
    {
        if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() < QApplication::startDragDistance())
            return;

        UpdateTooltip();
        setCursor(Qt::OpenHandCursor);
        QGraphicsPixmapItem::mouseMoveEvent(event);
    }

    void UpdateTooltip()
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

    // QGraphicsItem interface
public:
//    virtual QRectF boundingRect() const override
//    {
//        return QRectF(pos(), pixmap().size());
//    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)override
    {
        if (change != ItemPositionChange)
            return QGraphicsPixmapItem::itemChange(change, value);

        QPointF newpos = value.toPointF();
        //Restrict it so it only moves within the allowed range!
        qreal xpos = qBound<qreal>( 0.0, newpos.x(), FrameEditorSceneWidth-1 );
        qreal ypos = qBound<qreal>( 0.0, newpos.y(), FrameEditorSceneHeight-1 );
        return QPointF(qRound(xpos), qRound(ypos)); //The pos has to be an integer value!

    }

    virtual bool collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const override
    {
        return QGraphicsPixmapItem::collidesWithItem(other,mode);
    }
    virtual bool collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const override
    {
        return QGraphicsPixmapItem::collidesWithPath(path,mode);
    }

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override
    {
        UpdateTooltip();
        m_tooltip->setVisible(true);
        QGraphicsPixmapItem::hoverEnterEvent(event);
    }
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override
    {
        m_tooltip->setVisible(false);
        QGraphicsPixmapItem::hoverLeaveEvent(event);
    }
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        QGraphicsPixmapItem::keyPressEvent(event);
    }
    virtual void keyReleaseEvent(QKeyEvent *event) override
    {
        QGraphicsPixmapItem::keyReleaseEvent(event);
    }

signals:
    void wasDragged();
    void beingDoubleClicked();
    void updateTooltipPos(const QString & txt);
    void posChanged(const QPointF  & newpos);

private:
    QColor  m_overlayColor; //color for drawing the bounding box and text for this part!
    int     m_partid;
    QPointer<FrameEditor> m_frmedit;
    QPointer<QGraphicsTextItem> m_tooltip;
    QGraphicsLineItem* m_xAxisGuide;
    QGraphicsLineItem* m_yAxisGuide;
};


//
//  FrameEditor
//
class FrameEditor : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit FrameEditor(MFrame * frm, QObject *parent = nullptr)
        :QGraphicsScene(parent), m_pfrm(frm), m_gridsz(8)
    {
        //setBackgroundBrush(QBrush(QColor::fromHsv(195, 6, 25)));
    }

    inline QGraphicsScene & getScene() {return *this;}
    inline const QGraphicsScene & getScene()const {return *this;}


    void initScene()
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
            FramePart  * gfx = new FramePart( QPixmap::fromImage(ppart->drawPart()), i, this );
            m_parts.push_back(gfx);
            addItem(gfx);
            gfx->setPos( ppart->getPartData().getXOffset(), ppart->getPartData().getYOffset() );
            connect( gfx, &FramePart::wasDragged, this, &FrameEditor::onPartMoved );
        }

        //#TODO: Add stuff to indicate the middle of the sprite!
    }

signals:
    void partMoved();

public slots:

    void onPartMoved()
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

    void partListChanged(MFrame *pfrm)
    {
        m_pfrm = pfrm;
        initScene();
    }

    void updateParts()
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

    // QGraphicsScene interface
protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override
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

        for (qreal x = left; x < rect.right(); x += m_gridsz)
            linesx.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += m_gridsz)
            linesy.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setPen(penv);
        painter->drawLines(linesx.data(), linesx.size());

        painter->setPen(penh);
        painter->drawLines(linesy.data(), linesy.size());


        //Mark limits
        QColor limitcolor(128,128,0);
        QPen limitpen(QBrush(limitcolor), 2 );
        painter->setPen(limitpen);
        painter->drawLine( 0, FrameEditorSceneHeight, FrameEditorSceneWidth, FrameEditorSceneHeight);
        painter->drawLine( FrameEditorSceneWidth, 0, FrameEditorSceneWidth, FrameEditorSceneHeight);

        painter->setPen(QPen(QBrush(limitcolor.light(200)), 2));
        painter->drawText(FrameEditorSceneWidth - 48, 12, QString("MAX X:%1").arg(FrameEditorSceneWidth));
        painter->drawText(0, FrameEditorSceneHeight, QString("MAX Y:%1").arg(FrameEditorSceneHeight));
    }

private:
    MFrame *                    m_pfrm;
    QGraphicsScene              m_scene;
    QList<QPointer<FramePart>>  m_parts;
    int                         m_gridsz;
};

#endif // FRAMEEDITOR_HPP
