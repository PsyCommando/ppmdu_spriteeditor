#ifndef FRAME_EDITOR_PART_HPP
#define FRAME_EDITOR_PART_HPP
#include <src/ui/editor/frame/frame_editor_editable.hpp>
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
class MFramePart;
class FramePart : public EditableItem
{
    using parent_t = EditableItem;
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

    FramePart(FrameEditor * pfrmedit, const QPixmap & framepart, const QModelIndex & itemidx, QGraphicsItem *parent = nullptr);
    const QColor & overlayColor()const;
    QColor & overlayColor();

    MFramePart * getPart();
    const MFramePart * getPart()const;

    bool matchPart(const MFramePart* part)const;
    //QModelIndex partID()const;

    //Update the position of the item from the model's data
    virtual void updateOffset()override;

    //Commit the position of the editable item to the model
    virtual void commitOffset(QAbstractItemModel * model)override;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)override;
    virtual int type() const override;
    QString getItemDisplayName() const override;
private:
    void paintImage(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void setShowOutline(bool bshow);
    void setTransparencyEnabled(bool benabled);
    void onPosChanged();

private:
    QPixmap                 m_pixmap;
    QColor                  m_overlayColor; //color for drawing the bounding box and text for this part!
    //QPersistentModelIndex   m_partid;
    MFramePart *            m_pPart;
    bool                    m_bshowoutline{true};
    bool                    m_bTransparency{false};
};

#endif // FRAME_EDITOR_PART_HPP
