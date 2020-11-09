#ifndef TABEFFECTS_HPP
#define TABEFFECTS_HPP
#include <src/ui/tabs/basespritetab.hpp>
#include <src/data/sprite/models/effect_set_model.hpp>
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/ui/rendering/frame_offset_editor.hpp>
#include <QWidget>
#include <QDataWidgetMapper>

namespace Ui {
class TabEffects;
}

class OffsetMarkerItem : public QGraphicsTextItem
{
public:
    OffsetMarkerItem(QGraphicsItem *parent = nullptr):QGraphicsTextItem(parent){}
    OffsetMarkerItem(const QString &text, QGraphicsItem *parent = nullptr):QGraphicsTextItem(text,parent){}

    // QGraphicsItem interface
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

class TabEffects : public BaseSpriteTab
{
    Q_OBJECT
public:
    explicit TabEffects(QWidget *parent = nullptr);
    ~TabEffects();

    // BaseSpriteTab interface
public:
    void OnShowTab(QPersistentModelIndex element)override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private slots:

    void on_chkPlayAnim_toggled(bool checked);

    void on_tvOffsets_activated(const QModelIndex &index);

    void on_tvOffsets_clicked(const QModelIndex &index);

private:
    void SetupOffsetSet(EffectOffsetSet * set);
    void SelectOffset(const QModelIndex & element);
    void UnselectOffset();
    void ConnectSignals();
    void DisconnectSignals();

private:
    Ui::TabEffects *ui;
    QPersistentModelIndex               m_currentSet;
    FrameOffsetEditor                   m_preview;
    QScopedPointer<EffectSetModel>      m_effectModel;
    QScopedPointer<QDataWidgetMapper>   m_offsetdatamapper;
    QScopedPointer<QGraphicsTextItem>   m_offsetMarker;
};

#endif // TABEFFECTS_HPP
