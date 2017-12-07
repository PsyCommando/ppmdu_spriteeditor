#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QSpinBox>


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

    connect(ui->tvAnimTblAnimSeqs, &QTableView::clicked, [&](const QModelIndex & index)
    {
        if( ui->chkAnimTblAutoPlay->isChecked() )
        {
            Q_ASSERT(spr);
            AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
            Q_ASSERT(grp);
            AnimSequence * seq = spr->getAnimSequence( grp->seqSlots()[index.row()] );
            InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
        }
    });

    connect(ui->lvAnimTblAnimSeqList, &QTreeView::clicked, [&](const QModelIndex & index)
    {
        if( ui->chkAnimTblAutoPlay->isChecked() )
        {
            Q_ASSERT(spr);
            InstallAnimPreview(ui->gvAnimTablePreview, spr, spr->getAnimSequence(index.row()));
        }
    });
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

//
//
//
void MainWindow::on_btnAnimTblMoveSeq_clicked()
{

}

/*
 * When the user picks an animation group from the list
*/
void MainWindow::on_tvAnimTbl_activated(const QModelIndex &index)
{
}

