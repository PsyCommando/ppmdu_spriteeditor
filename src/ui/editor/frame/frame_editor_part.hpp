#ifndef FRAME_EDITOR_PART_HPP
#define FRAME_EDITOR_PART_HPP
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QWidget>

//===================================================================
//  FramePart
//===================================================================
// A movable piece of a frame that is made up of a single image
class FrameEditor;
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

    FramePart(const QPixmap & framepart, int framepartid, FrameEditor * pfrmedit, QGraphicsItem *parent = nullptr);



    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)override;

    virtual int type() const override;

    const QColor & overlayColor()const;
    QColor & overlayColor();

    int partID()const;

public slots:
    void setShowOutline(bool bshow);
    void setTransparencyEnabled(bool benabled);
    void onPosChanged();

protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;



    void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)override;
    void mousePressEvent(QGraphicsSceneMouseEvent *ev)override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override;
    void UpdateTooltip();

    // QGraphicsItem interface
public:
//    virtual QRectF boundingRect() const override
//    {
//        return QRectF(pos(), pixmap().size());
//    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)override;
    virtual bool collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const override;
    virtual bool collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const override;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;



signals:
    void wasDragged();
    void beingDoubleClicked();
    void updateTooltipPos(const QString & txt);
    void posChanged(const QPointF  & newpos);

private:
    QColor  m_overlayColor; //color for drawing the bounding box and text for this part!
    int     m_partid{0};
    bool    m_bshowoutline{true};
    bool    m_bTransparency{false};
    QPointer<FrameEditor>       m_frmedit;
    QPointer<QGraphicsTextItem> m_tooltip;
    QGraphicsLineItem* m_xAxisGuide;
    QGraphicsLineItem* m_yAxisGuide;

    void paintImage(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paintBoundingBox(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void UpdateStatusBarPos(QGraphicsSceneMouseEvent *event);
};

#endif // FRAME_EDITOR_PART_HPP
