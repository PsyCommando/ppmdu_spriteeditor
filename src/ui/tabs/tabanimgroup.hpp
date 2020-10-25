#ifndef TABANIMGROUP_HPP
#define TABANIMGROUP_HPP

#include <QWidget>
#include <QAbstractItemModel>
#include <src/ui/tabs/basespritetab.hpp>
#include <src/ui/rendering/sprite_scene.hpp>
#include <src/data/sprite/models/animgroup_model.hpp>
#include <src/data/sprite/models/animsequences_list_picker_model.hpp>

namespace Ui {
class TabAnimGroup;
}

class TabAnimGroup : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabAnimGroup(QWidget *parent = nullptr);
    ~TabAnimGroup();

    // BaseSpriteTab interface
public:
    void PrepareForNewContainer() override;
    void OnShowTab(QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction() override;
    void OnItemRemoval(const QModelIndex &item) override;

private:
    void ConnectScene();
    void DisconnectScene();

    void SetupModels(AnimGroup * pgrp);
    void ClearModels();

    void SetupPreview(const AnimSequence * pseq);
    void ClearPreview();

    void SelectGroupSlot(const QModelIndex & slot);
    void SelectAnimSequence(const QModelIndex & seq);

    AnimGroup * getCurrentGroup();
    const AnimGroup *getCurentGroup()const;

    //Whether the group currently loaded's group slots correspond to facing direction of the sprite
    bool isGroupUsingFacingSlots()const;

private slots:
    void on_btnPlay_clicked();

    void on_btnStop_clicked();

    void on_chkLoop_toggled(bool checked);

    void on_chkAutoPlay_toggled(bool checked);

    void on_btnAddSlot_clicked();

    void on_btnAddSeq_clicked();

    void on_btnRemoveSlot_clicked();

    void on_btnMoveSlotTop_clicked();

    void on_btnMoveSlotUp_clicked();

    void on_btnMoveSlotDown_clicked();

    void on_btnMoveSlotBottom_clicked();

    void on_tblSequences_activated(const QModelIndex &index);

    void on_tblSequences_clicked(const QModelIndex &index);

    void on_tblAnimGroup_activated(const QModelIndex &index);

    void on_tblAnimGroup_clicked(const QModelIndex &index);

private:
    Ui::TabAnimGroup *ui;
    QPersistentModelIndex                           m_curAnimGroup; //The anim group being edited
    QScopedPointer<AnimGroupModel>                  m_pGroupModel;
    QScopedPointer<AnimSequencesListPickerModel>    m_pAnimPicker;
    SpriteScene                                     m_previewScene;

    static const QList<QString> NameGrpSlotsFacing; //Names for the group slots for sprite using facing
};

#endif // TABANIMGROUP_HPP
