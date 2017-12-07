#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QSpinBox>
#include <QDebug>


void MainWindow::DisplayAnimTablePage(Sprite * spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabAnimTable);
    ui->tvAnimTbl->setModel(spr->getAnimTable().getModel());
    ui->tvAnimTblAnimSeqs->setModel(nullptr);
    ui->lvAnimTblAnimSeqList->setModel(spr->getAnimSequences().getPickerModel());

    connect( spr->getAnimTable().getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    {
        ui->tvAnimTbl->repaint();
        ui->tvAnimTblAnimSeqs->repaint();
    });

    connect(ui->tvAnimTbl, &QTableView::clicked, this, &MainWindow::OnAnimTableItemActivate);
    connect(ui->tvAnimTbl, &QTableView::activated, this, &MainWindow::OnAnimTableItemActivate);


    connect(ui->tvAnimTblAnimSeqs, &QTableView::clicked, this, &MainWindow::OnAmimTableGroupListItemActivate);
    connect(ui->tvAnimTblAnimSeqs, &QTableView::activated, this, &MainWindow::OnAmimTableGroupListItemActivate);


    connect(ui->lvAnimTblAnimSeqList, &QTreeView::activated, this, &MainWindow::OnAmimTableSequenceListItemActivate);


//    connect(ui->tvAnimTbl, &QTableView::activated, [spr,this](const QModelIndex &index)
//    {
//        Q_ASSERT(spr);
//        AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(index)));
//        Q_ASSERT(grp);
//        //Anim table entries don't have models

//        ui->tvAnimTblAnimSeqs->setModel(grp->getModel());
//        ui->tvAnimTblAnimSeqs->repaint();
//    });

//    connect(ui->tvAnimTbl, &QTableView::clicked, [spr,this](const QModelIndex &index)
//    {
//        Q_ASSERT(spr);
//        AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(index)));
//        Q_ASSERT(grp);
//        ui->tvAnimTblAnimSeqs->setModel(grp->getModel());
//    });

//    connect(ui->tvAnimTblAnimSeqs, &QTableView::clicked, [&](const QModelIndex & index)
//    {
//        if( ui->chkAnimTblAutoPlay->isChecked() )
//        {
//            Q_ASSERT(spr);
//            AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
//            Q_ASSERT(grp);
//            AnimSequence * seq = spr->getAnimSequence( grp->seqSlots()[index.row()] );
//            InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
//        }
//    });
//    connect(ui->lvAnimTblAnimSeqList, &QTreeView::clicked, [&](const QModelIndex & index)
//    {
//        if( ui->chkAnimTblAutoPlay->isChecked() )
//        {
//            Q_ASSERT(spr);
//            InstallAnimPreview(ui->gvAnimTablePreview, spr, spr->getAnimSequence(index.row()));
//        }
//    });
}

void MainWindow::OnAnimTableItemActivate(const QModelIndex &index)
{
    Sprite * spr = currentSprite();
    Q_ASSERT(spr);
    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(index)));
    Q_ASSERT(grp);

    ui->tvAnimTblAnimSeqs->setModel(grp->getModel());
    ui->tvAnimTblAnimSeqs->repaint();
    ui->tvAnimTblAnimSeqs->scrollToTop();
}

//When someone clicks/activates an item in the current animation group
void MainWindow::OnAmimTableGroupListItemActivate(const QModelIndex &index)
{
    //If we have more then one item selected just ignore it..
    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    if( lst.length() > 1 )
        return;

    //Don't do anything if the option isn't checked!
    if( !ui->chkAnimTblAutoSelectSeq->isChecked() )
        return;

    Sprite * spr = currentSprite();
    Q_ASSERT(spr);
    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
    Q_ASSERT(grp);

    if(index.row() >= 0 && index.row() < grp->seqSlots().size())
    {
        fmt::AnimDB::animseqid_t seqid = grp->seqSlots()[index.row()];
        if( ui->lvAnimTblAnimSeqList->model()->hasIndex(seqid,0) )
            ui->lvAnimTblAnimSeqList->setCurrentIndex( ui->lvAnimTblAnimSeqList->model()->index(seqid,0) );
        else
            qDebug() << "MainWindow::OnAmimTableGroupListItemActivate(): There are no animation sequences with the ID " << (int)seqid <<"!! Can't select from available animation sequences!\n";
    }
//    if( ui->chkAnimTblAutoPlay->isChecked() )
//    {
//        AnimSequence * seq = spr->getAnimSequence( grp->seqSlots()[index.row()] );
//        InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
//    }
}

//When selecting a sequence from the available sequences list
void MainWindow::OnAmimTableSequenceListItemActivate(const QModelIndex &index)
{
    //If we have more then one item selected just ignore it..
    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    if( lst.length() > 1 )
        return;

    Sprite * spr = currentSprite();
    Q_ASSERT(spr);

    if( ui->chkAnimTblAutoPlay->isChecked() )
    {
        Q_ASSERT(spr);
        InstallAnimPreview(ui->gvAnimTablePreview, spr, spr->getAnimSequence(index.row()));
    }
}

//
//
//
void MainWindow::on_btnAnimTblReplaceSeq_pressed()
{
    QModelIndexList lstdest =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    QModelIndexList lstsrc  =  ui->lvAnimTblAnimSeqList->selectionModel()->selectedIndexes();

    //Don't do anything if we got nothing selected in either.
    if( lstsrc.empty() || lstdest.empty() )
        return;

    //Make sure both the src and destination are the same length
    if( lstdest.length() != lstsrc.length() )
    {
        Warn(tr("Replace failed!"), tr("You must select the same ammount of items in both lists to replace multiple items!"));
        return;
    }

    Sprite * spr = currentSprite();
    Q_ASSERT(spr);
    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
    Q_ASSERT(grp);

    //Put the sequence IDs in order from the source to the group's slots
    for( int cnt = 0; cnt < lstdest.length(); ++cnt )
         grp->getModel()->setData(lstdest.at(cnt), lstsrc.at(cnt).row(), Qt::EditRole);

    ShowStatusMessage(QString("Replaced %1 sequences!").arg(lstdest.length()));
}

void MainWindow::on_btnAnimTblMoveSeq_pressed()
{
    QModelIndexList lstdest =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    QModelIndexList lstsrc  =  ui->lvAnimTblAnimSeqList->selectionModel()->selectedIndexes();

    //Don't do anything if we got nothing to move
    if( lstsrc.empty() )
        return;

    Sprite * spr = currentSprite();
    Q_ASSERT(spr);
    AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
    Q_ASSERT(grp);

    //Get insert position
    int insertpos = (lstdest.empty())? grp->seqSlots().size() : insertpos = lstdest.first().row();

    //Insert rows
    grp->getModel()->insertRows( insertpos, lstsrc.size(), QModelIndex() );

    //Put the sequence IDs in order from the source to the group's slots
    for( int cnt = 0; cnt < lstsrc.size(); ++cnt )
    {
         grp->getModel()->setData( grp->getModel()->index(insertpos + cnt,0, QModelIndex()),
                                   lstsrc.at(cnt).row(),
                                   Qt::EditRole);
    }

    ShowStatusMessage(QString("Inserted %1 sequences!").arg(lstdest.length()));
}

void MainWindow::on_btnAnimTblMoveSeq_clicked()
{

}

void MainWindow::on_btnAnimTblAddAnim_pressed()
{
    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();
    int insertpos = (lst.empty())? ui->tvAnimTbl->model()->rowCount() : lst.first().row();
    //Do the insertion, and set the data
    ui->tvAnimTbl->model()->insertRows( insertpos, 1 );
    ui->tvAnimTbl->model()->setData( ui->tvAnimTbl->model()->index(insertpos,0), insertpos );
    ShowStatusMessage("Inserted a new animation!");
}

void MainWindow::on_btnAnimTblRemAnim_pressed()
{
    QModelIndexList lst = ui->tvAnimTbl->selectionModel()->selectedIndexes();
    if(lst.empty())
    {
        Warn(tr("Removal failed!"), tr("You must select an animation to remove first!"));
        return;
    }

//    while( !ui->tvAnimTbl->selectionModel()->selectedIndexes().empty() )
        ui->tvAnimTbl->model()->removeRow( ui->tvAnimTbl->selectionModel()->selectedIndexes().first().row() );

    ShowStatusMessage(QString("Removed %1 animation(s)!").arg(lst.length()));
}

void MainWindow::on_btnAnimTblImportTemplate_pressed()
{

}

void MainWindow::on_btnAnimTblExportTemplate_pressed()
{

}

void MainWindow::on_btnAnimTblAddSeq_pressed()
{
    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    if(lst.empty())
    {
        //Add at the end
        ui->tvAnimTblAnimSeqs->model()->insertRow( ui->tvAnimTblAnimSeqs->model()->rowCount() );
    }
    else
    {
        //Add before first selected
        ui->tvAnimTblAnimSeqs->model()->insertRow( lst.first().row() );
    }

    ShowStatusMessage("Added slot!");
}

void MainWindow::on_btnAnimTblRemSeq_pressed()
{
    QModelIndexList lst =  ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes();
    if(lst.empty())
    {
        ShowStatusErrorMessage("Couldn't remove the slots. Nothing was selected!");
        return;
    }

    while( !ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes().empty() )
        ui->tvAnimTblAnimSeqs->model()->removeRow( ui->tvAnimTblAnimSeqs->selectionModel()->selectedIndexes().first().row() );

//    for( const QModelIndex & i : lst )
//        ui->tvAnimTblAnimSeqs->model()->removeRow(i.row());

    ShowStatusMessage(QString("Removed %1 slots!").arg(lst.length()));
}




/*
 * When the user picks an animation group from the list
*/
void MainWindow::on_tvAnimTbl_activated(const QModelIndex &index)
{
}

