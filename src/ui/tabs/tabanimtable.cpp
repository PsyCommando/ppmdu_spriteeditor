#include "tabanimtable.hpp"
#include "ui_tabanimtable.h"
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/animtable.hpp>

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
    m_previewrender.reset(new SpriteScene);
    ConnectControls();
    BaseSpriteTab::OnShowTab(element);
}

void TabAnimTable::OnHideTab()
{
    clearAnimTable();
    m_previewrender.reset();
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
    connect(m_animTableModel.data(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    {
        ui->tvAnimTbl->repaint();
    });
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
    return true;
}

void TabAnimTable::clearAnimTable()
{
    OnDeselectAll();
    m_previewedGroup = QModelIndex();
    m_animTable = QModelIndex();
}

void TabAnimTable::OnSelectGroup(const QItemSelection &selection)
{
    if(selection.empty())
        OnDeselectAll();
    else
    {
        //First item is the one to be previewed!
         QModelIndex index = selection.first().indexes().first();
         m_previewedGroup = index;

         //Grow the slider to the appropriate length
         const AnimGroup * pgrp = reinterpret_cast<const AnimGroup *>(m_previewedGroup.internalPointer());
         ui->sldrSubSequence->blockSignals(true);
         ui->sldrSubSequence->setRange(0, pgrp->nodeChildCount()-1);
         ui->sldrSubSequence->blockSignals(false);
         ui->sldrSubSequence->setValue(0);
         ui->sldrSubSequence->update();

         //Preview first sequence
         PreviewGroupAnimSequence(0, ui->chkAutoPlay->isChecked());

         //Enable Controls
         ui->grpPreviewOptions->setEnabled(true);
         ui->btnMoveGroupUp->setEnabled(true);
         ui->btnMoveGroupDown->setEnabled(true);
         ui->btnExtraOptions->setEnabled(true);
    }
}

void TabAnimTable::OnDeselectAll()
{
    m_previewedGroup = QModelIndex();
    ClearPreview();

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
    ui->btnExtraOptions->setEnabled(false);
}

void TabAnimTable::PreviewGroupAnimSequence(int idxsubseq, bool bplaynow)
{
    if(!m_previewedGroup.isValid())
    {
        qWarning() << "TabAnimTable::PreviewGroupAnimSequence(): Can't preview, invalid group preview index!!";
        return;
    }
    const AnimGroup * grp = dynamic_cast<const AnimGroup*>(m_animTableModel->getItem(m_previewedGroup));
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
    ui->lblPreviewSeqName->setText("----");
    ui->lblPreviewSeqName->setEnabled(false);
    if(m_previewrender)
    {
        m_previewrender->endAnimationPlayback();
        m_previewrender->UninstallAnimPreview(ui->gvAnimTablePreview);
    }
}

void TabAnimTable::OpenCurrentAnimSequence()
{
    if(!m_previewedGroup.isValid())
        return;
    const AnimGroup * curgrp = dynamic_cast<const AnimGroup*>(m_animTableModel->getItem(m_previewedGroup));
    if(!curgrp)
        return;

    fmt::AnimDB::animseqid_t seqid = curgrp->getAnimSlotRef(ui->sldrSubSequence->value());// animslots.at(ui->sldrSubSequence->value());
    Sprite * spr = currentSprite();
    getMainWindow()->selectTreeViewNode(spr->getAnimSequence(seqid));
}

void TabAnimTable::OpenCurrentGroup()
{
    if(!m_previewedGroup.isValid())
        return;
    const TreeNode * pgrp = reinterpret_cast<const TreeNode*>(m_previewedGroup.internalPointer());
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

////When someone clicks/activates an item in the current animation group
//void TabAnimTable::OnAmimTableGroupListItemActivate(const QModelIndex &index)
//{
//    //If we have more then one item selected just ignore it..
//    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    if( lst.length() > 1 )
//        return;

//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);
//    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(m_animTableModel->getItem(ui->tvAnimTbl->currentIndex())));
//    Q_ASSERT(grp);

//    if(index.row() >= 0 && index.row() < grp->seqSlots().size())
//    {
//        fmt::AnimDB::animseqid_t seqid = grp->seqSlots()[index.row()];
//        if( ui->lvAnimTblAnimSeqList->model()->hasIndex(seqid,0) )
//        {
//            if( ui->chkAnimTblAutoSelectSeq->isChecked() )
//                ui->lvAnimTblAnimSeqList->setCurrentIndex( ui->lvAnimTblAnimSeqList->model()->index(seqid,0) );
//            UpdateAnimTblPreview(seqid);
//        }
//        else
//            qDebug() << "MainWindow::OnAmimTableGroupListItemActivate(): There are no animation sequences with the ID " << (int)seqid <<"!! Can't select from available animation sequences!\n";
//    }
//}

////When selecting a sequence from the available sequences list
//void TabAnimTable::OnAmimTableSequenceListItemActivate(const QModelIndex &index)
//{
//    //If we have more then one item selected just ignore it..
//    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    if( lst.length() > 1 )
//        return;

//    UpdateAnimTblPreview(index.row());
//}

//void TabAnimTable::UpdateAnimTblPreview(fmt::AnimDB::animseqid_t seqid)
//{
//    if(m_previewrender)
//        m_previewrender->Reset();
//    if( !ui->chkAnimTblAutoPlay->isChecked() )
//        return;

//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);
//    AnimSequence * seq = spr->getAnimSequence(seqid);

//    if(seq)
//    {
//        m_previewrender->InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
//        m_previewrender->beginAnimationPlayback();
//    }
//    else
//        qDebug() <<"MainWindow::UpdateAnimTblPreview(): Couldn't find the sequence " <<(int)seqid <<" for preview!";
//}

//
//
//
//void TabAnimTable::on_btnAnimTblReplaceSeq_pressed()
//{
//    QModelIndexList lstdest =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    QModelIndexList lstsrc  =  ui->lvAnimTblAnimSeqList->selectionModel()->selectedIndexes();

//    //Don't do anything if we got nothing selected in either.
//    if( lstsrc.empty() || lstdest.empty() )
//        return;

//    //Make sure both the src and destination are the same length
//    if( lstdest.length() != lstsrc.length() )
//    {
//        Warn(tr("Replace failed!"), tr("You must select the same ammount of items in both lists to replace multiple items!"));
//        return;
//    }

//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);
//    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(m_selAnimGroupModel->getItem(ui->tvAnimTbl->currentIndex())));
//    Q_ASSERT(grp);

//    //Put the sequence IDs in order from the source to the group's slots
//    for( int cnt = 0; cnt < lstdest.length(); ++cnt )
//         m_selAnimGroupModel->setData(lstdest.at(cnt), lstsrc.at(cnt).row(), Qt::EditRole);

//    ShowStatusMessage(QString("Replaced %1 sequences!").arg(lstdest.length()));
//}

//void TabAnimTable::on_btnAnimTblMoveSeq_pressed()
//{
//    QModelIndexList lstdest =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    QModelIndexList lstsrc  =  ui->lvAnimTblAnimSeqList->selectionModel()->selectedIndexes();

//    //Don't do anything if we got nothing to move
//    if( lstsrc.empty() )
//        return;

//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);
//    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(m_selAnimGroupModel->getItem(ui->tvAnimTbl->currentIndex())));
//    Q_ASSERT(grp);

//    //Get insert position
//    int insertpos = (lstdest.empty())? grp->seqSlots().size() : lstdest.first().row();

//    //Insert rows
//    m_selAnimGroupModel->insertRows( insertpos, lstsrc.size(), QModelIndex() );

//    //Put the sequence IDs in order from the source to the group's slots
//    for( int cnt = 0; cnt < lstsrc.size(); ++cnt )
//    {
//         m_selAnimGroupModel->setData( m_selAnimGroupModel->index(insertpos + cnt,0, QModelIndex()),
//                                   lstsrc.at(cnt).row(),
//                                   Qt::EditRole);
//    }

//    ShowStatusMessage(QString("Inserted %1 sequences!").arg(lstdest.length()));
//}

//void TabAnimTable::on_btnAnimTblMoveSeq_clicked()
//{

//}

//void TabAnimTable::on_btnAnimTblAddAnim_pressed()
//{
//    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();
//    int insertpos = (lst.empty())? 0 : lst.first().row(); //Insert at begining of list if no selection
//    //Do the insertion, and set the data
//    ui->tvAnimTbl->model()->insertRows( insertpos, 1 );
//    ui->tvAnimTbl->model()->setData( ui->tvAnimTbl->model()->index(insertpos,0), insertpos );
//    ShowStatusMessage("Inserted a new animation!");
//}

//void TabAnimTable::on_btnAnimTblAppendAnim_pressed()
//{
//    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();
//    int insertpos = (lst.empty())? ui->tvAnimTbl->model()->rowCount() : lst.first().row() + 1;
//    //Do the insertion, and set the data
//    ui->tvAnimTbl->model()->insertRows( insertpos, 1 );
//    ui->tvAnimTbl->model()->setData( ui->tvAnimTbl->model()->index(insertpos,0), insertpos );
//    ShowStatusMessage("Appended a new animation!");
//}

//void TabAnimTable::on_btnAnimTblRemAnim_pressed()
//{
//    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();
//    if(lst.empty())
//    {
//        Warn(tr("Removal failed!"), tr("You must select an animation to remove first!"));
//        return;
//    }

//    //#FIXME: Trying to remove all selected items on this list results in the entire list being wiped out
////    while( !ui->tvAnimTbl->selectionModel()->selectedIndexes().empty() )
//        ui->tvAnimTbl->model()->removeRow( ui->tvAnimTbl->selectionModel()->selectedIndexes().first().row() );

//    ShowStatusMessage(QString("Removed %1 animation(s)!").arg(lst.length()));
//}

//void TabAnimTable::on_btnAnimTblAnimMvUp_pressed()
//{
//    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();

//    for(const auto & sel : lst)
//        ui->tvAnimTbl->model()->moveRow(QModelIndex(), sel.row(), QModelIndex(), qMax(0, sel.row() - 1) );
//    ShowStatusMessage(QString("Moved up %1 animations!").arg(lst.length()));
//}

//void TabAnimTable::on_btnAnimTblAnimMvDown_pressed()
//{
//    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();

//    for(const auto & sel : lst)
//        ui->tvAnimTbl->model()->moveRow(QModelIndex(), sel.row(), QModelIndex(), qMin(ui->tvAnimTbl->model()->rowCount()-1, sel.row() + 1) );
//    ShowStatusMessage(QString("Moved down %1 animations!").arg(lst.length()));
//}

//void TabAnimTable::on_btnAnimTblImportTemplate_pressed()
//{

//}

//void TabAnimTable::on_btnAnimTblExportTemplate_pressed()
//{

//}

//void TabAnimTable::on_btnAnimTblAddSeq_pressed()
//{
//    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    if(lst.empty())
//    {
//        //Add at the end
//        ui->tvAnimTblAnimSeqs->model()->insertRow( ui->tvAnimTblAnimSeqs->model()->rowCount() );
//    }
//    else
//    {
//        //Add before first selected
//        ui->tvAnimTblAnimSeqs->model()->insertRow( lst.first().row() );
//    }

//    ShowStatusMessage("Added slot!");
//}

//void TabAnimTable::on_btnAnimTblRemSeq_pressed()
//{
//    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
//    if(lst.empty())
//    {
//        ShowStatusErrorMessage("Couldn't remove the slots. Nothing was selected!");
//        return;
//    }

//    //#FIXME: If the widget is set to a particular selection mode, this might delete all entries instead of only the selected ones..
//    while( !ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes().empty() )
//        ui->tvAnimTblAnimSeqs->model()->removeRow( ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes().first().row() );

//    ShowStatusMessage(QString("Removed %1 slots!").arg(lst.length()));
//}


//void TabAnimTable::on_btnAnimTblCvHeroAnims_pressed()
//{
//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);

//    //1. Check if we have at least 4 animations
//    if(spr->getAnimTable().nodeChildCount() < 4)
//    {
//    }
//    //2a. If we have 4 already, assume its a basic pokemon anim set, and reorganize it to fit the hero anim set, and add the missing anims each with 8 slots.
//    //2b. Otherwise, just create all the new animations, add 8 slots to each
//    //2c. If we have already all the needed animation slots, just pop-up a message and ask if the user wants to re-organize the animations in the hero set's order
//}

//void TabAnimTable::on_chkAnimTblAutoPlay_toggled(bool checked)
//{

//}


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
    PreviewGroupAnimSequence(value, ui->chkAutoPlay->isChecked());
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
        const AnimGroup * grp = dynamic_cast<const AnimGroup*>(m_animTableModel->getItem(m_previewedGroup));
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
    Q_ASSERT(false);
}

void TabAnimTable::on_btnMoveGroupDown_clicked()
{
    Q_ASSERT(false);
}
