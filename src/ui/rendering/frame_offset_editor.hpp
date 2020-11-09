#ifndef FRAMEOFFSETEDITOR_HPP
#define FRAMEOFFSETEDITOR_HPP

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <src/data/sprite/models/effect_set_model.hpp>
#include <src/data/sprite/sprite.hpp>

/*
 * A scene for editing effect position markers on sprite frames
*/
class FrameOffsetEditor : public QObject
{
    Q_OBJECT
public:
    FrameOffsetEditor(QObject * parent = nullptr);
    ~FrameOffsetEditor();

    /*
     * Returns the internal QGraphicScene object managing the preview scene!
    */
    inline QGraphicsScene       & getScene()        {return m_scene;}
    inline const QGraphicsScene & getScene()const   {return m_scene;}

    void InstallPreview(QGraphicsView *     pview,
                        EffectSetModel *    pmodel,
                        const Sprite *      spr,
                        fmt::frmid_t        frmid);
    void UninstallPreview(QGraphicsView * pview);

    void setSelected(const QModelIndex & index);
    QModelIndex getSelected()const;

    QColor getSpriteBGColor() const;

    void setFrameId(fmt::frmid_t id);
    fmt::frmid_t getFrameId()const;

    void Clear();
    void Update();

private:
    QGraphicsScene          m_scene;
    EffectSetModel *        m_model     {nullptr};
    const Sprite *          m_sprite    {nullptr};
    fmt::frmid_t            m_frmid     {-1};
    QPersistentModelIndex   m_selOffset;

    //Item for the scene
    QGraphicsPixmapItem* m_frameItem{nullptr};

    QList<QGraphicsTextItem*> m_posMarkers;
    QGraphicsTextItem*   m_posHead  {nullptr};
    QGraphicsTextItem*   m_posRHand {nullptr};
    QGraphicsTextItem*   m_posLHand {nullptr};
    QGraphicsTextItem*   m_posCenter{nullptr};
};

#endif // FRAMEOFFSETEDITOR_HPP
