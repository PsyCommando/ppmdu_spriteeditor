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
#include <src/data/sprite/models/effect_set_model.hpp>
#include <src/ui/editor/frame/frame_editor_marker.hpp>
#include <src/ui/editor/frame/frame_editor_editable.hpp>
#include <src/ui/editor/frame/frame_editor_shared.hpp>

//===================================================================
//  FrameEditor
//===================================================================
// Scene for rendering the assembly of the various images making up a frame
class FrameEditor : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit FrameEditor(MFrame * frm, Sprite * pspr, MFramePartModel * ppartmdl, EffectSetModel * psetmdl, QObject *parent = nullptr);
    void initScene();
    void initScene(bool bdrawoutline, bool bdrawmarker, bool btransparency);
    void deInitScene();

    inline QGraphicsScene       & getScene()        {return *this;}
    inline const QGraphicsScene & getScene()const   {return *this;}

    inline MFrame       * getFrame()        {return m_pfrm;}
    inline const MFrame * getFrame()const   {return m_pfrm;}

    //Returns the middle XY coordinate of the center of the scene
    QPointF getSceneCenter()const;

    inline eEditorMode getEditorMode()const {return m_mode;}

private:
    void drawGrid(QPainter * painter, const QRectF & rect);
    void drawLimits(QPainter * painter, const QRectF & rect);

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
    //void onPartMoved();
    void onItemDragBegin(EditableItem * item);
    void onItemDragEnd(EditableItem * item);
    void OnSelectionChanged();

    //Slots we receive from the TableView
    void OnViewPartsSelected(const QModelIndexList & selected);
    void OnViewMarkersSelected(const QModelIndexList & selected);

    //Settings
    void setDisplayNDSMode(bool basnds);
    void setDrawMiddleGuide(bool bdraw);
    void setDrawOutlines(bool bdraw);
    void setTransparencyEnabled(bool benabled);
    void setEditorMode(eEditorMode mode);
    void setGridSnap(bool bsnapon);

    void partListChanged(MFrame *pfrm);
    //void updateParts();
    void updateScene();


    void deleteItem(EditableItem * item);

    void attachRulers(EditableItem * item);
    void clearRulers();

    void selectMarker(QModelIndex marker);

signals:
    void markerMoved(); //The position markers were moved on the editor!
    void partMoved();
    void zoom(int);
    void mousePosUpdate(const QPointF & mousepos);

    //List items that are now selected
    void selectionChanged(QList<EditableItem*> items);

    void markerWasSelected(EditableItem * pmarker);

    void editorModeChanged(eEditorMode mode);

private:
    QRectF                              m_bgarea;

    QList<QPointer<FramePart>>          m_parts;        //Parts of the frame
    QPointer<MFramePartModel>           m_model;        //Model to display the parts for the frame

    QPointer<EffectSetModel>            m_attachModel;  //Model for displaying attachment points for effects
    QList<QPointer<AttachMarker>>       m_markers;      //Attachment points for effects

    MFrame *                            m_pfrm              {nullptr};  //Frame currently being edited
    Sprite *                            m_psprite           {nullptr};  //Sprite owning the frame being edited

    QScopedPointer<QGraphicsLineItem>   m_xAxisGuide        {nullptr};  //Ruler for the object being moved
    QScopedPointer<QGraphicsLineItem>   m_yAxisGuide        {nullptr};  //Ruler for the object being moved
    EditableItem *                      m_rulerItem         {nullptr};  //Item the rulers are following

    int                                 m_midmarkZ          {99};       //Z-level of the middle markers
    int                                 m_gridZ             {0};        //Z-level for the grids to be drawn on
    int                                 m_gridsz            {1};        //len of a side of the squares that make up the grid
    bool                                m_bDrawMiddleMarker {true};
    bool                                m_bTransparency     {false};
    bool                                m_bDrawOutline      {true};
    eEditorMode                         m_mode              {eEditorMode::FrameParts}; //Editor mode, decides what's rendered and what's editable
    bool                                m_bNDSMode          {true};     //If true the sprite will be displayed relative to its in-memory nds offset (aka, with Y-wrap)
};

#endif // FRAME_EDITOR_HPP
