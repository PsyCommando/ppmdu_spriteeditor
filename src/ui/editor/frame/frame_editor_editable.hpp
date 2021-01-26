#ifndef FRAME_EDITOR_EDITABLE_HPP
#define FRAME_EDITOR_EDITABLE_HPP
#include <src/data/treeenodetypes.hpp>
#include <src/ui/editor/frame/frame_editor_shared.hpp>
#include <QGraphicsItem>
#include <QPersistentModelIndex>
#include <QPointer>

/*
 * Base class for an editable graphic item
 * used in the frame editor.
 */
class FrameEditor;
class EditableItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem);
    static const int XMaxThresholdTooltip;
    static const int XMinThresholdTooltip;
    static const int YMaxThresholdTooltip;
    static const int YMinThresholdTooltip;

    static const int DefXTooltip;
    static const int DefYTooltip;
    static const int DefZTooltip;
    static const int FarRightXTooltip;
    static const int FarLeftXTooltip;
    static const QColor DefToolTipColor;

public:
    EditableItem(FrameEditor *pfrmedit, const QModelIndex & itemidx, QGraphicsItem * parent = nullptr);
    virtual ~EditableItem();

    QModelIndex getItemIndex()const;
    virtual void setItemIndex(const QModelIndex &idx);

    eTreeElemDataType getDataType()const;
    virtual QString getItemDisplayName()const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)override;

    //Returns the offset of the middle of the object, from its top left corner
    virtual QPointF getRelativeMiddle()const;

    //Returns the position of the center of the item in absolute coordinates. Exactly like pos(), but offset the pos so its relative to the center of the item
    virtual QPointF getCenterPoint()const;

    //Update the position of the item from the model's data
    virtual void updateOffset(){}

    //Commit the position of the editable item to the model
    virtual void commitOffset(QAbstractItemModel * model){Q_UNUSED(model);}

protected:
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
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

    FrameEditor * getFrameEditor();
    const FrameEditor *getFrameEditor()const;

signals:
    void beingDoubleClicked();
    void updateTooltipPos(const QString & txt);
    void posChanged(const QPointF & newpos);

    void dragBegin(EditableItem * item);
    void dragEnd(EditableItem * item);

protected:
    QPointer<FrameEditor>       m_frmedit;
    QPointer<QGraphicsTextItem> m_tooltip;
    QPersistentModelIndex       m_itemidx;
    QColor                      m_BoundingBoxColor;
    int                         m_xScnCntr {0}; //Middle x coordinate of the scene
    int                         m_yScnCntr {0}; //Middle y coordinate of the scene
};

#endif // FRAME_EDITOR_EDITABLE_HPP
