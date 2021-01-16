#include "tabanimtable.hpp"
#include "ui_tabanimtable.h"
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/animtable.hpp>
#include <src/data/sprite/models/animtable_delegate.hpp>
#include <src/extfmt/animtation_table_layout.hpp>
#include <src/utility/file_support.hpp>
#include <src/utility/program_settings.hpp>
#include <QFileDialog>
#include <QFile>

TabAnimTable::TabAnimTable(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabAnimTable)
{
    ui->setupUi(this);
}

TabAnimTable::~TabAnimTable()
{
    delete ui;
}

void TabAnimTable::OnShowTab(QPersistentModelIndex element)
{
    Sprite * spr = currentSprite();
    Q_ASSERT(spr);
    if(!setAnimTable(element, spr))
    {
        Q_ASSERT(false);
    }
    ui->tvAnimTbl->setModel(m_animTableModel.data());
    ui->tvAnimTbl->setItemDelegate(m_animTableDelegate.data());
    ui->tvAnimTbl->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    ui->tvAnimTbl->resizeColumnsToContents();
    m_previewrender.reset(new SpriteScene);
    ui->btnExtraOptions->setMenu(MakeExtraMenu());
    ConnectControls();
    BaseSpriteTab::OnShowTab(element);
}

void TabAnimTable::OnHideTab()
{
    OnDeselectAll();
    clearAnimTable();
    m_previewrender.reset();
    ui->btnExtraOptions->setMenu(nullptr);
    DisconnectControls();
    BaseSpriteTab::OnHideTab();
}

//Called before shared data is destroyed, so we get a chance to cleanup
void TabAnimTable::OnDestruction()
{
    clearAnimTable();
    m_previewrender.reset();
    BaseSpriteTab::OnDestruction();
}

void TabAnimTable::ConnectControls()
{
    //AnimTable * ptable = static_cast<AnimTable*>(m_animTable.internalPointer());
    connect(m_animTableModel.data(), &QAbstractItemModel::dataChanged, this, &TabAnimTable::OnModelDataChanged);
    connect(ui->tvAnimTbl,          &QTableView::clicked,   this, &TabAnimTable::OnAnimTableItemActivate);
    connect(ui->tvAnimTbl,          &QTableView::activated, this, &TabAnimTable::OnAnimTableItemActivate);
    connect(ui->lblPreviewSeqName,  &QLabel::linkActivated, [&](const QString&){OpenCurrentAnimSequence();});
}

void TabAnimTable::DisconnectControls()
{
    if(m_animTable.isValid())
    {
        //AnimTable * ptable = static_cast<AnimTable *>(m_animTable.internalPointer());
        disconnect(m_animTableModel.data(), &QAbstractItemModel::dataChanged, nullptr, nullptr);
    }
    disconnect(ui->tvAnimTbl,           &QTableView::clicked,   this, &TabAnimTable::OnAnimTableItemActivate);
    disconnect(ui->tvAnimTbl,           &QTableView::activated, this, &TabAnimTable::OnAnimTableItemActivate);
    disconnect(ui->lblPreviewSeqName,   &QLabel::linkActivated, nullptr, nullptr);
}

bool TabAnimTable::setAnimTable(QPersistentModelIndex table, Sprite *spr)
{
    if(!table.isValid())
        return false;
    clearAnimTable();
    AnimTable * ptable = static_cast<AnimTable*>(table.internalPointer());
    m_animTable = table;
    m_animTableModel.reset(new AnimTableModel(ptable, spr));
    m_animTableDelegate.reset(new AnimTableDelegate(ptable));
    return true;
}

void TabAnimTable::clearAnimTable()
{
    OnDeselectAll();
    m_previewedGroupRef = QModelIndex();
    m_animTable = QModelIndex();
}

void TabAnimTable::OnSelectGroup(const QItemSelection &selection)
{
    if(selection.empty())
        OnDeselectAll();
    else
    {
        QModelIndex index = selection.first().indexes().first(); //First item is the one to be previewed!
        m_previewedGroupRef = index;
        const AnimGroup* pgrp = currentAnimGroup();
        if(!pgrp)
        {
            ClearPreview();
            return; //Nothing valid to preview here!!
        }

        //Enable Preview
        ui->btnPlay->setEnabled(true);
        ui->btnStop->setEnabled(true);

        //Grow the slider to the appropriate length
        //const AnimGroup * pgrp = reinterpret_cast<const AnimGroup *>(m_previewedGroup.internalPointer());
        ui->sldrSubSequence->blockSignals(true);
        ui->sldrSubSequence->setRange(0, pgrp->nodeChildCount()-1);
        ui->sldrSubSequence->blockSignals(false);
        ui->sldrSubSequence->setValue(0);
        ui->sldrSubSequence->update();

        //Preview first sequence
        PreviewGroupAnimSequence(0, ProgramSettings::Instance().isAutoplayEnabled());

        //Enable Controls
        ui->grpPreviewOptions->setEnabled(true);
        ui->btnMoveGroupUp->setEnabled(true);
        ui->btnMoveGroupDown->setEnabled(true);
    }
}

void TabAnimTable::OnDeselectAll()
{
    m_previewedGroupRef = QModelIndex();
    ClearPreview();

    //Disable Preview
    ui->btnPlay->setEnabled(false);
    ui->btnStop->setEnabled(false);

    //Clear the slider
    ui->sldrSubSequence->blockSignals(true);
    ui->sldrSubSequence->setRange(0, 1);
    ui->sldrSubSequence->blockSignals(false);
    ui->sldrSubSequence->setValue(0);
    ui->sldrSubSequence->update();

    //Disable buttons
    ui->grpPreviewOptions->setEnabled(false);
    ui->btnMoveGroupUp->setEnabled(false);
    ui->btnMoveGroupDown->setEnabled(false);
}

void TabAnimTable::PreviewGroupAnimSequence(int idxsubseq, bool bplaynow)
{
    if(!m_previewedGroupRef.isValid())
    {
        qWarning() << "TabAnimTable::PreviewGroupAnimSequence(): Can't preview, invalid group preview index!!";
        return;
    }
    const AnimGroup * grp = currentAnimGroup();
    if(grp)
    {
       Sprite* spr = currentSprite();
       //Setup first sequence in group if any as preview seq
       if(grp->nodeChildCount() > 0)
       {
            const AnimSequence * seq = spr->getAnimSequence(grp->getAnimSlotRef(idxsubseq));
            ui->lblPreviewSeqName->setText(tr("Group slot: %1 => [Sequence: %2]()").arg(idxsubseq).arg(seq->nodeIndex()));
            ui->lblPreviewSeqName->setEnabled(true);
            ui->lblPreviewSeqName->setOpenExternalLinks(true);
            m_previewrender->InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
            if(bplaynow)
                m_previewrender->beginAnimationPlayback();
            m_previewrender->setShouldLoop(ui->chkPreviewLoop->isChecked());
       }
       return;
    }
    //If no group, or group has no subsequences
    ClearPreview();
}

void TabAnimTable::ClearPreview()
{
    ui->lblPreviewSeqName->setText("");
    ui->lblPreviewSeqName->setEnabled(false);
    if(m_previewrender)
    {
        m_previewrender->endAnimationPlayback();
        m_previewrender->UninstallAnimPreview(ui->gvAnimTablePreview);
    }
}

void TabAnimTable::OpenCurrentAnimSequence()
{
    if(!m_previewedGroupRef.isValid())
        return;
    const AnimGroup * curgrp = currentAnimGroup();
    if(!curgrp)
        return;

    fmt::animseqid_t seqid = curgrp->getAnimSlotRef(ui->sldrSubSequence->value());
    Sprite * spr = currentSprite();
    getMainWindow()->selectTreeViewNode(spr->getAnimSequence(seqid));
}

void TabAnimTable::OpenCurrentGroup()
{
    if(!m_previewedGroupRef.isValid())
        return;
    const TreeNode * pgrp = currentAnimGroup();
    getMainWindow()->selectTreeViewNode(pgrp);
}

void TabAnimTable::PrepareForNewContainer()
{
    OnHideTab();
}

void TabAnimTable::OnAnimTableItemActivate(const QModelIndex &)
{
    //Setup selection
    QItemSelectionModel * sel = ui->tvAnimTbl->selectionModel();
    Q_ASSERT(sel);
    OnSelectGroup(sel->selection());
}

void TabAnimTable::OnModelDataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    ui->tvAnimTbl->repaint();
    //Refresh preview
    OnSelectGroup(ui->tvAnimTbl->selectionModel()->selection());
}

AnimGroup * TabAnimTable::currentAnimGroup()
{
    Sprite *      spr     = currentSprite();
    AnimTableSlot *grpref  = dynamic_cast<AnimTableSlot*>(m_animTableModel->getItem(m_previewedGroupRef));
    return spr->getAnimGroup(grpref->getGroupRef());
}

const AnimGroup * TabAnimTable::currentAnimGroup()const
{
    return const_cast<TabAnimTable*>(this)->currentAnimGroup();
}

QMenu *TabAnimTable::MakeExtraMenu()
{
    QMenu * pm = new QMenu(ui->btnExtraOptions);
    pm->addAction(ui->actionImport_animation_table);
    pm->addAction(ui->actionExport_animation_table);
    return pm;
}

/*
 * When the user picks an animation group from the list
*/

void TabAnimTable::on_btnPlay_clicked()
{
    PreviewGroupAnimSequence(ui->sldrSubSequence->value(), true);
}

void TabAnimTable::on_btnStop_clicked()
{
    if(!m_previewrender)
        return;
    m_previewrender->endAnimationPlayback();
    m_previewrender->setCurFrm(0);
}

void TabAnimTable::on_sldrSubSequence_valueChanged(int value)
{
    PreviewGroupAnimSequence(value, ProgramSettings::Instance().isAutoplayEnabled());
}

void TabAnimTable::on_chkAutoPlay_toggled(bool checked)
{
    //Get the selected sequences, and only preview the first one
    QModelIndexList lstGroups = ui->tvAnimTbl->selectionModel()->selectedIndexes();
    if(lstGroups.empty())
        return; //Nothing to do here

    Sprite * spr = currentSprite();
    Q_ASSERT(spr);

    if(!m_previewrender)
        return;
    if(checked)
    {
        const AnimGroup * grp = currentAnimGroup();
        if(grp)
        {
            on_btnPlay_clicked();
            return;
        }
    }
    m_previewrender->endAnimationPlayback();
    m_previewrender->UninstallAnimPreview(ui->gvAnimTablePreview);
}

void TabAnimTable::on_chkPreviewLoop_toggled(bool checked)
{
    if(!m_previewrender)
        return;
    m_previewrender->setShouldLoop(checked);
}

void TabAnimTable::on_btnMoveGroupUp_clicked()
{
    QModelIndexList lstGroups = ui->tvAnimTbl->selectionModel()->selectedRows();
    if(lstGroups.empty())
    {
        ShowStatusErrorMessage(tr("Nothing to move up!"));
        return;
    }
    int topmostSelectedRow = m_animTableModel->rowCount();
    for(const QModelIndex & idx : lstGroups)
    {
        if(idx.row() < topmostSelectedRow)
            topmostSelectedRow = idx.row();
    }
    m_animTableModel->moveRows(lstGroups, topmostSelectedRow > 0? topmostSelectedRow - 1 : 0);
    ui->tvAnimTbl->update();
    ui->tvAnimTbl->repaint();
}

void TabAnimTable::on_btnMoveGroupDown_clicked()
{
    QModelIndexList lstGroups = ui->tvAnimTbl->selectionModel()->selectedRows();
    if(lstGroups.empty())
    {
        ShowStatusErrorMessage(tr("Nothing to move down!"));
        return;
    }
    int bottommostSelectedRow = 0;
    for(const QModelIndex & idx : lstGroups)
    {
        if(idx.row() > bottommostSelectedRow)
            bottommostSelectedRow = idx.row();
    }
    if(bottommostSelectedRow + 1 >= m_animTableModel->rowCount())
        return; //Don't bother moving down when we're the last entry

    m_animTableModel->moveRows(lstGroups, bottommostSelectedRow + 2); //Moving down place before the destination, so +2
    ui->tvAnimTbl->update();
    ui->tvAnimTbl->repaint();
}

void TabAnimTable::on_btnAddGroup_clicked()
{
    QModelIndexList lstGroups = ui->tvAnimTbl->selectionModel()->selectedRows();
    if(lstGroups.empty())
    {
        //Append at the end!
        m_animTableModel->insertRow(m_animTableModel->rowCount());
        ui->tvAnimTbl->selectRow(m_animTableModel->rowCount() - 1);
    }
    else
    {
        //Append after selected!
        m_animTableModel->insertRow(lstGroups.last().row());
        ui->tvAnimTbl->selectRow(lstGroups.last().row() + 1);
    }
    ui->tvAnimTbl->update();
    ui->tvAnimTbl->repaint();
}

void TabAnimTable::on_btnRemoveGroup_clicked()
{
    QModelIndexList lstGroups = ui->tvAnimTbl->selectionModel()->selectedRows();
    if(lstGroups.empty())
    {
        ShowStatusErrorMessage(tr("Nothing selected to remove!"));
        return;
    }
    m_animTableModel->removeRows(lstGroups);
    OnDeselectAll();
    ui->tvAnimTbl->clearSelection();
    ui->tvAnimTbl->update();
    ui->tvAnimTbl->repaint();
}

void TabAnimTable::on_actionImport_animation_table_triggered()
{
    //open file open dialog
    const QString fpath = GetXMLOpenFile(tr("Import animation table template.."), this);

    if(fpath.isEmpty())
        return; //if nothing selected cancel

    //add missing rows/remove extra rows + names
    ImportAnimTableLayout(m_animTableModel.data(), fpath);
}

void TabAnimTable::on_actionExport_animation_table_triggered()
{
    //open save file dialog
    const QString fpath = GetXMLSaveFile(tr("Export animation table template.."), this);

    if(fpath.isEmpty())
        return; //if nothing selected cancel

    //save layout to xml
    ExportAnimTableLayout(m_animTableModel.data(), fpath);
}
