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

//extern const int FrameEditorSceneWidth  ;
//extern const int FrameEditorSceneHeight ;
//extern const int FrameEditorSceneXPos   ;
//extern const int FrameEditorSceneYPos   ;

//===================================================================
//  FramePart
//===================================================================
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
    void setShowOutline(bool bshow) {m_bshowoutline = bshow; update();}
    void setTransparencyEnabled(bool benabled) {m_bTransparency = benabled; update();}
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
    int     m_partid;
    bool    m_bshowoutline;
    bool    m_bTransparency;
    QPointer<FrameEditor> m_frmedit;
    QPointer<QGraphicsTextItem> m_tooltip;
    QGraphicsLineItem* m_xAxisGuide;
    QGraphicsLineItem* m_yAxisGuide;

};


//===================================================================
//  FrameEditor
//===================================================================
class FrameEditor : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit FrameEditor(MFrame * frm, QObject *parent = nullptr);
    void initScene();
    void initScene(bool bdrawoutline, bool bdrawmarker, bool btransparency);

    inline QGraphicsScene       & getScene()        {return *this;}
    inline const QGraphicsScene & getScene()const   {return *this;}

    inline MFrame       * getFrame()        {return m_pfrm;}
    inline const MFrame * getFrame()const   {return m_pfrm;}

public slots:
    void onPartMoved();
    void partListChanged(MFrame *pfrm);
    void updateParts();
    void setDrawMiddleGuide(bool bdraw);
    void setDrawOutlines(bool bdraw);
    void setTransparencyEnabled(bool benabled);

    // QGraphicsScene interface
protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void partMoved();
    void zoom(int);

private:
    MFrame *                            m_pfrm;
    QGraphicsScene                      m_scene;
    QList<QPointer<FramePart>>          m_parts;
    //QScopedPointer<QGraphicsLineItem>   m_middleMarker;
    int                                 m_midmarkZ;
    int                                 m_gridsz;
    QRectF                              m_bgarea;
    bool                                m_bDrawMiddleMarker;
    bool                                m_bTransparency;
    bool                                m_bDrawOutline;
};

#endif // FRAMEEDITOR_HPP
