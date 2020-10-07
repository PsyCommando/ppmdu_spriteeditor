#ifndef FRAME_EDITOR_HPP
#define FRAME_EDITOR_HPP
/*
frame_editor.hpp

Helper for managing the QGraphicsScene used for displaying and editing the parts of a frame!
*/
#include <QGraphicsScene>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/models/framepart_model.hpp>
#include <src/ui/editor/frame/frame_editor_part.hpp>

//===================================================================
//  FrameEditor
//===================================================================
// Scene for rendering the assembly of the various images making up a frame
class FrameEditor : public QGraphicsScene
{
    Q_OBJECT
public:

    explicit FrameEditor(MFrame * frm, Sprite * pspr, QObject *parent = nullptr);
    void initScene();
    void initScene(bool bdrawoutline, bool bdrawmarker, bool btransparency);
    void deInitScene();

    inline QGraphicsScene       & getScene()        {return *this;}
    inline const QGraphicsScene & getScene()const   {return *this;}

    inline MFrame       * getFrame()        {return m_pfrm;}
    inline const MFrame * getFrame()const   {return m_pfrm;}

    // QGraphicsScene interface
protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

public slots:
    void onPartMoved();
    void partListChanged(MFrame *pfrm);
    void updateParts();
    void setDrawMiddleGuide(bool bdraw);
    void setDrawOutlines(bool bdraw);
    void setTransparencyEnabled(bool benabled);

signals:
    void partMoved();
    void zoom(int);

    //When the actively selected part is changed this is emited with a list of ids of the parts that were now selected or an empty list if none are
    void selectionChanged(QList<int> partid);

private:
    QGraphicsScene                      m_scene;
    QList<QPointer<FramePart>>          m_parts;
    QRectF                              m_bgarea;
    QPointer<MFramePartModel>           m_model;
    MFrame *                            m_pfrm              {nullptr};
    Sprite *                            m_psprite           {nullptr};
    int                                 m_midmarkZ          {99};
    int                                 m_gridsz            {8};
    bool                                m_bDrawMiddleMarker {true};
    bool                                m_bTransparency     {false};
    bool                                m_bDrawOutline      {true};
};

#endif // FRAME_EDITOR_HPP
