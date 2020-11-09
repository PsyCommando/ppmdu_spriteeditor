#ifndef FRAME_EDITOR_EDITABLE_HPP
#define FRAME_EDITOR_EDITABLE_HPP
//Base class for an editable element in the frame editor
#include <src/data/treeenodetypes.hpp>
#include <QGraphicsItem>
#include <QPersistentModelIndex>
#include <QPointer>

class FrameEditor;
class EditableItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem);
    static const int XMaxThresholdTooltip = 370;
    static const int XMinThresholdTooltip = 0;
    static const int YMaxThresholdTooltip = 255;
    static const int YMinThresholdTooltip = 15;

    static const int DefXTooltip = 0;
    static const int DefYTooltip = -25;
    static const int DefZTooltip = 99;
    static const int FarRightXTooltip = -80;
    static const int FarLeftXTooltip = -80;
    static const QColor DefToolTipColor;

public:
    EditableItem(FrameEditor *pfrmedit, QGraphicsItem * parent = nullptr);
    virtual ~EditableItem();

    QModelIndex getItemIndex()const {return m_itemidx;}
    virtual void setItemIndex(const QModelIndex &idx) {m_itemidx = idx;}

    eTreeElemDataType getDataType()const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)override;

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *ev)override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)override;
protected:
    virtual void SetupTooltip();
    virtual void UpdateTooltip();
    virtual void paintBoundingBox(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void changeMyPos(int x, int y); //Helper to handle emiting the correct signales and etc when moving ourselves
    virtual bool shouldShowBoundingBox() const;

signals:
    void wasDragged();
    void beingDoubleClicked();
    void updateTooltipPos(const QString & txt);
    void posChanged(const QPointF  & newpos);

protected:
    QPointer<FrameEditor>       m_frmedit;
    QPointer<QGraphicsTextItem> m_tooltip;
    QPersistentModelIndex       m_itemidx;
    QColor                      m_BoundingBoxColor;
};

#endif // FRAME_EDITOR_EDITABLE_HPP
