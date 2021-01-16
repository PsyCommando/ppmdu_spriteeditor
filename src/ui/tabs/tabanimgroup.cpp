#include "tabanimgroup.hpp"
#include "ui_tabanimgroup.h"
#include <src/utility/program_settings.hpp>

const QList<QString> TabAnimGroup::NameGrpSlotsFacing
{
    {"Down"},
    {"Down-Right"},
    {"Right"},
    {"Up-Right"},
    {"Up"},
    {"Up-Left"},
    {"Left"},
    {"Down-Left"},
};

TabAnimGroup::TabAnimGroup(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabAnimGroup)
{
    ui->setupUi(this);
    ui->gvSequencePreview->setScene(&m_previewScene.getAnimScene());
}

TabAnimGroup::~TabAnimGroup()
{
    delete ui;
}

void TabAnimGroup::PrepareForNewContainer()
{
    OnHideTab();
}

void TabAnimGroup::OnShowTab(QPersistentModelIndex element)
{
    if(!element.isValid())
        return;
    m_curAnimGroup = element;
    AnimGroup * pgrp = reinterpret_cast<AnimGroup*>(element.internalPointer());

    SetupModels(pgrp);

    ConnectScene();

    BaseSpriteTab::OnShowTab(element);
}

void TabAnimGroup::OnHideTab()
{
    ui->tblSequences->clearSelection();
    ui->tblAnimGroup->clearSelection();
    ClearPreview();
    ClearModels();
    m_curAnimGroup = QModelIndex();
    BaseSpriteTab::OnHideTab();
}

void TabAnimGroup::OnDestruction()
{
    OnHideTab();
    DisconnectScene();
    BaseSpriteTab::OnDestruction();
}

void TabAnimGroup::OnItemRemoval(const QModelIndex &item)
{
    //Just re-draw everything, since animations depend on pretty much everything..
    if(item.internalPointer() != m_curAnimGroup.internalPointer())
        OnShowTab(m_curAnimGroup);
    else
        OnHideTab();
    BaseSpriteTab::OnItemRemoval(item);
}

void TabAnimGroup::ConnectScene()
{

}

void TabAnimGroup::DisconnectScene()
{

}

void TabAnimGroup::SetupModels(AnimGroup * pgrp)
{
    Sprite * spr = currentSprite();
    m_pGroupModel.reset(new AnimGroupModel(pgrp, spr));
    m_pAnimPicker.reset(new AnimSequencesListPickerModel(&spr->getAnimSequences(), spr));

    if(isGroupUsingFacingSlots())
        m_pGroupModel->setSlotName(NameGrpSlotsFacing);

    ui->tblAnimGroup->setModel(m_pGroupModel.data());
    ui->tblSequences->setModel(m_pAnimPicker.data());

    ui->tblAnimGroup->update();
    ui->tblSequences->update();
}

void TabAnimGroup::ClearModels()
{
    ui->tblAnimGroup->setModel(nullptr);
    ui->tblSequences->setModel(nullptr);
    m_pGroupModel.reset();
    m_pAnimPicker.reset();
}

void TabAnimGroup::SetupPreview(const AnimSequence *pseq)
{
    const Sprite * pspr = currentSprite();
    Q_ASSERT(pspr);
    m_previewScene.InstallAnimPreview(ui->gvSequencePreview, pspr, pseq);
    m_previewScene.setCurrentFrame(0);
    if(ProgramSettings::Instance().isAutoplayEnabled())
        m_previewScene.beginAnimationPlayback();
}

void TabAnimGroup::ClearPreview()
{
    m_previewScene.endAnimationPlayback();
    m_previewScene.UninstallAnimPreview(ui->gvSequencePreview);
}

void TabAnimGroup::SelectGroupSlot(const QModelIndex &slot)
{
    if(!slot.isValid())
    {
        ui->tblAnimGroup->clearSelection();
        return;
    }
    ui->tblAnimGroup->setCurrentIndex(slot);

    //When we select a group slot, also select the sequence in the sequence list!
    Sprite * sprite = currentSprite();
    Q_ASSERT(sprite);
    AnimGroup * grp = getCurrentGroup();
    Q_ASSERT(grp);
    fmt::animseqid_t id = grp->getAnimSlotRef(slot.row());
    SelectAnimSequence(m_pAnimPicker->index(id, 0, QModelIndex()));
}

void TabAnimGroup::SelectAnimSequence(const QModelIndex &seq)
{
    if(!seq.isValid())
    {
        ui->tblSequences->clearSelection();
        return;
    }
    ui->tblSequences->setCurrentIndex(seq);

    AnimSequence * pseq = static_cast<AnimSequence*>(seq.internalPointer());
    Q_ASSERT(pseq);
    SetupPreview(pseq);
}

AnimGroup *TabAnimGroup::getCurrentGroup()
{
    return static_cast<AnimGroup*>(m_curAnimGroup.internalPointer());
}

const AnimGroup *TabAnimGroup::getCurentGroup() const
{
    return static_cast<const AnimGroup*>(m_curAnimGroup.internalPointer());
}

//#TODO: Replace this with a data file instead maybe?
bool TabAnimGroup::isGroupUsingFacingSlots() const
{
    using namespace fmt;
    const Sprite * spr = currentSprite();
    eSpriteType sprt = spr->type();
    return sprt == eSpriteType::Character || sprt == eSpriteType::Prop;
}

void TabAnimGroup::on_btnPlay_clicked()
{
    m_previewScene.beginAnimationPlayback();
}

void TabAnimGroup::on_btnStop_clicked()
{
    m_previewScene.endAnimationPlayback();
}

void TabAnimGroup::on_chkLoop_toggled(bool checked)
{
    m_previewScene.setShouldLoop(checked);
}

void TabAnimGroup::on_chkAutoPlay_toggled(bool /*checked*/)
{

}

void TabAnimGroup::on_btnAddSlot_clicked()
{
    //In this case, we only insert at the selection, without replacing anything!
    QModelIndexList tobeinserted = ui->tblSequences->selectionModel()->selectedRows();
    if(tobeinserted.empty())
        return;

    //Get the anim slot selected and insert there if there's any selection or at the end of the list!
    int insertrow = m_pAnimPicker->rowCount(QModelIndex());
    QModelIndexList slotselected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(slotselected.empty())
        insertrow = slotselected.first().row();

    int cntinsert = 0;
    for(const QModelIndex & idx : tobeinserted)
    {
        AnimSequence * pseq = static_cast<AnimSequence*>(idx.internalPointer());
        m_pGroupModel->insertRow(insertrow + cntinsert);
        m_pGroupModel->setData(m_pGroupModel->index(insertrow  + cntinsert, 0, QModelIndex()),
                               pseq->nodeIndex(),
                               Qt::EditRole);
        ++cntinsert;
    }
}

void TabAnimGroup::on_btnAddSeq_clicked()
{
    //In this case we replace on insertion instead of just inserting at the selected slot(s)
    QModelIndexList toinsert = ui->tblSequences->selectionModel()->selectedRows();
    if(toinsert.empty())
        return; //No sequence selected, do nothing

    QModelIndexList slotselected = ui->tblAnimGroup->selectionModel()->selectedRows();
    //Just replace the one slot with the selected sequence
    if(slotselected.size() == 1 && toinsert.size() == 1)
    {
        AnimSequence * pseq = static_cast<AnimSequence*>(toinsert.first().internalPointer());
        m_pGroupModel->setData(m_pGroupModel->index(slotselected.first().row(), 0, QModelIndex()), pseq->nodeIndex(), Qt::EditRole);
    }
    //set all slots to the one selected anim sequence
    else if(slotselected.size() > 1 && toinsert.size() == 1)
    {

        AnimSequence * pseq = static_cast<AnimSequence*>(toinsert.first().internalPointer());
        for(QModelIndex & slotidx : slotselected)
            m_pGroupModel->setData(slotidx, pseq->nodeIndex(), Qt::EditRole);
    }
    //Replace as many selected sequences as possible, and add any leftover right after
    // Or insert at the end if no slots selected!
    else
    {
        int cntinsert = 0;
        int insertbeg = m_pAnimPicker->rowCount(QModelIndex());
        int insertend = m_pAnimPicker->rowCount(QModelIndex());
        if(!slotselected.empty())
        {
            insertbeg = slotselected.first().row();
            insertend = slotselected.last().row();
        }
        for(QModelIndex & idx : toinsert)
        {
            AnimSequence * pseq = static_cast<AnimSequence*>(idx.internalPointer());
            //If we reached the end of the selected slots, add more slots!
            if(insertbeg + cntinsert >= insertend)
                m_pGroupModel->insertRow(insertbeg + cntinsert);
            m_pGroupModel->setData(m_pGroupModel->index(insertbeg + cntinsert, 0, QModelIndex()), pseq->nodeIndex(), Qt::EditRole);
            ++cntinsert;
        }
    }

}

void TabAnimGroup::on_btnRemoveSlot_clicked()
{
    QModelIndexList selected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(selected.empty())
        return;
    m_pGroupModel->removeRows(selected);
}

void TabAnimGroup::on_btnMoveSlotTop_clicked()
{
    QModelIndexList selected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(selected.empty())
        return;
    m_pGroupModel->moveRows(selected, 0);
}

void TabAnimGroup::on_btnMoveSlotUp_clicked()
{
    QModelIndexList selected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(selected.empty())
        return;
    int begrow = selected.first().row();
    if(begrow == 1)
        return; //Can't move anything up!
    m_pGroupModel->moveRows(selected, begrow-1);
}

void TabAnimGroup::on_btnMoveSlotDown_clicked()
{
    QModelIndexList selected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(selected.empty())
        return;
    int endrow = selected.first().row();
    if(endrow >= (m_pGroupModel->rowCount(QModelIndex())-1))
        return; //Can't move anything up!
    m_pGroupModel->moveRows(selected, endrow+1);
}

void TabAnimGroup::on_btnMoveSlotBottom_clicked()
{
    QModelIndexList selected = ui->tblAnimGroup->selectionModel()->selectedRows();
    if(selected.empty())
        return;
    m_pGroupModel->moveRows(selected, m_pGroupModel->rowCount(QModelIndex()));
}

void TabAnimGroup::on_tblSequences_activated(const QModelIndex &index)
{
    SelectAnimSequence(index);
}

void TabAnimGroup::on_tblSequences_clicked(const QModelIndex &index)
{
    SelectAnimSequence(index);
}

void TabAnimGroup::on_tblAnimGroup_activated(const QModelIndex &index)
{
    on_tblAnimGroup_clicked(index);
}

void TabAnimGroup::on_tblAnimGroup_clicked(const QModelIndex &index)
{
    QItemSelectionModel * pmod = ui->tblAnimGroup->selectionModel();
    if(pmod->selectedRows().size() == 1) //Select the anim preview only when we're selecting a single thing
        SelectGroupSlot(index);
}

void TabAnimGroup::on_chkDisplayFrameBorder_toggled(bool checked)
{
    if(checked)
        ui->gvSequencePreview->setBackgroundBrush(QBrush(QColor("black")));
    else
        ui->gvSequencePreview->setBackgroundBrush(QBrush(m_previewScene.getSpriteBGColor()));
}
