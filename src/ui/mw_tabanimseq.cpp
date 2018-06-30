#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QSpinBox>
#include <QGraphicsView>

void MainWindow::DisplayAnimSequencePage(Sprite *spr, AnimSequence * aniseq)
{
    Q_ASSERT(spr && aniseq);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Showing anim sequence page!\n";
    ShowATab(ui->tabSequence);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Instanciated anim viewer!\n";

    ui->tblseqfrmlst->setModel(aniseq->getModel());
    ui->tblseqfrmlst->setItemDelegate(aniseq->getDelegate());
    ui->tblseqfrmlst->resizeRowsToContents();
    ui->tblseqfrmlst->resizeColumnsToContents();

    InstallAnimPreview(ui->gvAnimSeqViewport, spr, aniseq);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Scene set!\n";
}

void MainWindow::InstallAnimPreview(QGraphicsView * viewport, Sprite *spr, AnimSequence * aniseq)
{
    qDebug() << "MainWindow::InstallAnimPreview(): Displaying animation..\n";
    m_previewrender.setScene(spr, aniseq->nodeIndex());
    viewport->setScene(&m_previewrender.getAnimScene());
    viewport->centerOn(m_previewrender.getAnimSprite());
    m_previewrender.getAnimSprite()->setScale(2.0);

    connect( aniseq->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    {
        m_previewrender.reloadAnim();
        viewport->setAutoFillBackground(true);
        viewport->setBackgroundBrush(QBrush(m_previewrender.getSpriteBGColor()));
        viewport->update();
    });
}



// *********************************
//  Anim Sequence Tab
// *********************************
void MainWindow::on_btnSeqAddFrm_clicked()
{
    QModelIndex     ind       = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    int insertpos = 0;

    if(ind.isValid())
        insertpos = ind.row();
    else
        insertpos = (curseq->nodeChildCount() > 0)? (curseq->nodeChildCount() - 1) : 0;

    if(curseq->getModel()->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqRemFrm_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No frame selected!"));
        return;
    }
    AnimFrame       * pafrm = static_cast<AnimFrame*>(ind.internalPointer());
    AnimSequence    * pseq  = static_cast<AnimSequence*>(pafrm->parentNode());
    Q_ASSERT(pseq && pafrm);

    ui->tblseqfrmlst->setCurrentIndex(QModelIndex());
    if(pseq->removeChildrenNodes(ind.row(), 1))
        ShowStatusMessage(tr("Removed animation frame!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqMvUp_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() > 0)? ind.row() - 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curseq->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqMvDown_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() < curseq->nodeChildCount()-1 )? ind.row() + 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curseq->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqDup_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    AnimFrame tmpfrm = *(static_cast<AnimFrame*>(ind.internalPointer()));

    int insertpos = ind.row();
    if(curseq->getModel()->insertRow(insertpos))
    {
        AnimFrame * pnewfrm = static_cast<AnimFrame *>(curseq->getItem( curseq->getModel()->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = tmpfrm;
        ShowStatusMessage(tr("Duplicated animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqExport_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Images Sequence : Pick name+path first image!"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
    {
        return;
    }

    int rmpast = filename.size() - filename.lastIndexOf('.');
    if( rmpast > 0 && rmpast < filename.size() )
        filename.chop(rmpast);
    QVector<QImage> sequence = m_previewrender.DumpSequence();

    if(sequence.isEmpty())
    {
        ShowStatusErrorMessage(tr("Error: No sequence was loaded for export!"));
    }

    int cntimg = 0;
    for(; cntimg < sequence.size(); ++cntimg )
        sequence[cntimg].save( QString("%1_%2.png").arg(filename).arg(cntimg) );

    ShowStatusMessage(QString(tr("Exported %1 images!")).arg(cntimg));
}

void MainWindow::on_tblseqfrmlst_activated(const QModelIndex &index)
{
    //When clicking on a frame in the frame list, stop the anim preview
    m_previewrender.stopAnimUpdates();
}
