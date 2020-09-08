#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>
#include <QSpinBox>
#include <QTimer>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/paletteeditor.hpp>


void MainWindow::DisplayMFramePage(Sprite *spr, MFrame * frm)
{
    Q_ASSERT(spr && frm);
    qDebug() << "MainWindow::DisplayMFramePage(): Showing frame page!\n";
    ui->tblframeparts->setModel(frm->getModel());
    ui->tblframeparts->setItemDelegate(&(frm->itemDelegate()));
    ui->tblframeparts->setEditTriggers(QTableView::EditTrigger::AllEditTriggers);
    ui->tblframeparts->setSizeAdjustPolicy(QTableView::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
    ShowATab(ui->tabFrame);
    ui->tblframeparts->resizeRowsToContents();
    ui->tblframeparts->resizeColumnsToContents();
    ui->tblframeparts->setCurrentIndex( ui->tblframeparts->model()->index(0, 0, QModelIndex()) );

    //Setup the frame editor in the viewport!
    m_frmeditor.reset( new FrameEditor(frm) );
    ui->gvFrame->setScene( &m_frmeditor->getScene() );
    m_frmeditor->initScene(ui->chkColorPartOutlines->isChecked(),
                           ui->chkFrmMiddleMarker->isChecked(),
                           ui->chkFrmTransparency->isChecked());
    ui->gvFrame->repaint();

    if(ui->tblframeparts->currentIndex().isValid())
        setupFrameEditPageForPart( frm, static_cast<MFramePart*>(ui->tblframeparts->currentIndex().internalPointer()) );

    //Setup the callbacks
    connect(frm->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &,const QModelIndex &, const QVector<int>&)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsInserted, [&](const QModelIndex & /*parent*/, int /*first*/, int /*last*/)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsMoved, [&](const QModelIndex & /*parent*/,
                                                                int /*start*/,
                                                                int /*end*/,
                                                                const QModelIndex & /*destination*/,
                                                                int /*row*/)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsRemoved, [&](const QModelIndex & /*parent*/,
                                                                int /*first*/,
                                                                int /*last*/)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });

    connect(ui->spbFrmZoom, qOverload<int>(&QSpinBox::valueChanged), [&](int val)->void
    {
        qreal sc = val * 0.01; //scale the value from 0 to 1 +
        ui->gvFrame->setTransform(QTransform::fromScale(sc, sc));
    });

    connect(m_frmeditor.data(), &FrameEditor::zoom, [&](int val)->void
    {
        ui->spbFrmZoom->setValue(val + ui->spbFrmZoom->value());
    });

    //Init checkboxes state
    connect(ui->chkColorPartOutlines, &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawOutlines);
    connect(ui->chkFrmMiddleMarker,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
    connect(ui->chkFrmTransparency,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);

    //Map model's columns to some of the controls
    m_frmdatmapper.reset(new QDataWidgetMapper);
    m_frmdatmapper->setModel(frm->getModel());
    m_frmdatmapper->addMapping(ui->spbFrmPartXOffset,  static_cast<int>(eFramesColumnsType::direct_XOffset) );
    m_frmdatmapper->addMapping(ui->spbFrmPartYOffset,  static_cast<int>(eFramesColumnsType::direct_YOffset) );
    m_frmdatmapper->addMapping(ui->btnFrmVFlip,        static_cast<int>(eFramesColumnsType::direct_VFlip) );
    m_frmdatmapper->addMapping(ui->btnFrmHFlip,        static_cast<int>(eFramesColumnsType::direct_HFlip) );
    m_frmdatmapper->toFirst();
    connect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}


// *********************************
//  Frame Tab
// *********************************
void MainWindow::on_btnFrmRmPart_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }
    MFramePart* ppart = static_cast<MFramePart*>(ind.internalPointer());
    MFrame    * pfrm  = static_cast<MFrame*>(ppart->parentNode());
    Q_ASSERT(ppart && pfrm);

    ui->tblframeparts->setCurrentIndex(QModelIndex());
    if(pfrm->removeChildrenNodes(ind.row(), 1))
        ShowStatusMessage(tr("Removed part!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmAdPart_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();

    Q_ASSERT(curframe);
    int insertpos = 0;

    if(ind.isValid())
        insertpos = ind.row();
    else
        insertpos = (curframe->nodeChildCount() > 0)? (curframe->nodeChildCount() - 1) : 0;

    if(curframe->getModel()->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended part!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmMvUp_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    int destrow = (ind.row() > 0)? ind.row() - 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curframe->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Part moved up!"));
        else
            ShowStatusErrorMessage(tr("Failed to move part!"));
    }

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmMvDown_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    int destrow = (ind.row() < curframe->nodeChildCount()-2 )? ind.row() + 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curframe->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Part moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move part!"));
    }

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmDup_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    MFramePart tmppart = *(static_cast<MFramePart*>(ind.internalPointer()));

    int insertpos = ind.row();
    if(curframe->getModel()->insertRow(insertpos))
    {
        MFramePart * pnewfrm = static_cast<MFramePart *>(curframe->getItem( curframe->getModel()->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = tmppart;
        ShowStatusMessage(tr("Duplicated part!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_cmbFrmQuickPrio_currentIndexChanged(int index)
{
    Q_ASSERT(false); //#TODO: Make this work!

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmExport_clicked()
{
    const MFrame * pfrm = m_frmeditor->getFrame();
    if(!pfrm)
    {
        ShowStatusErrorMessage(tr("Couldn't export, no frame loaded!"));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;


    QImage img( qMove(pfrm->AssembleFrame(0, 0, pfrm->calcFrameBounds() )) );
    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported assembled frame to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}

