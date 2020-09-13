#include "tabframes.hpp"
#include "ui_tabframes.h"

#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framesdelegate.hpp>

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QImage>

TabFrames::TabFrames(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabFrames)
{
    ui->setupUi(this);
}

TabFrames::~TabFrames()
{
    delete ui;
}

void TabFrames::ConnectSignals()
{
    MFrame * frm = static_cast<MFrame*>(m_frame.internalPointer());
    Q_ASSERT(frm);
    //Setup the callbacks
    connect(frm->getModel(), &QAbstractItemModel::dataChanged, this, &TabFrames::OnDataChanged);
    connect(ui->spbFrmZoom, qOverload<int>(&QSpinBox::valueChanged), this, &TabFrames::On_spbFrmZoom_ValueChanged );
    connect(m_frmeditor.data(), &FrameEditor::zoom, this, &TabFrames::OnFrameEditorZoom);
    connect(m_frmeditor.data(), &FrameEditor::selectionChanged, this, &TabFrames::OnSelectionChanged);

    //Init checkboxes state
    connect(ui->chkColorPartOutlines, &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawOutlines);
    connect(ui->chkFrmMiddleMarker,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
    connect(ui->chkFrmTransparency,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);

    //Connect mapper
    connect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}

void TabFrames::DisconnectSignals()
{
    disconnect(ui->spbFrmZoom, qOverload<int>(&QSpinBox::valueChanged), this, &TabFrames::On_spbFrmZoom_ValueChanged );

    MFrame * frm = static_cast<MFrame*>(m_frame.internalPointer());
    if(frm)
        disconnect(frm->getModel(), &QAbstractItemModel::dataChanged, this, &TabFrames::OnDataChanged);

    if(m_frmeditor)
    {
        disconnect(m_frmeditor.data(), &FrameEditor::zoom, this, &TabFrames::OnFrameEditorZoom);
        disconnect(ui->chkColorPartOutlines, &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawOutlines);
        disconnect(ui->chkFrmMiddleMarker,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
        disconnect(ui->chkFrmTransparency,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);
    }

    if(m_frmdatmapper)
        disconnect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}

void TabFrames::OnFrameEditorZoom(int diff)
{
    ui->spbFrmZoom->setValue(diff + ui->spbFrmZoom->value());
}

void TabFrames::On_spbFrmZoom_ValueChanged(int val)
{
    qreal sc = val * 0.01; //scale the value from 0 to 1 +
    ui->gvFrame->setTransform(QTransform::fromScale(sc, sc));
}

void TabFrames::OnDataChanged(const QModelIndex &,const QModelIndex &, const QVector<int>&)
{
    if(m_frmeditor)
        m_frmeditor->updateParts();
    ui->gvFrame->update();
}

void TabFrames::OnShowTab(Sprite *spr, QPersistentModelIndex element)
{
    if(!element.isValid())
        throw ExBadFrame("TabFrames::OnShowTab(): Bad frame index!");
    MFrame * frm = static_cast<MFrame*>(element.internalPointer());
    if(!frm)
        throw ExBadFrame("TabFrames::OnShowTab(): Couldn't cast to frame!!");
    Q_ASSERT(spr && frm);

    //Keep track of our displayed frame
    m_frame = element;

    ui->spbFrmPartXOffset->setRange(0, fmt::step_t::XOFFSET_MAX);
    ui->spbFrmPartYOffset->setRange(0, fmt::step_t::YOFFSET_MAX);

    qDebug() << "MainWindow::DisplayMFramePage(): Showing frame page!\n";
    ui->tblframeparts->setModel(frm->getModel());
    ui->tblframeparts->setItemDelegate(&(frm->itemDelegate()));
    ui->tblframeparts->setEditTriggers(QTableView::EditTrigger::AllEditTriggers);
    ui->tblframeparts->setSizeAdjustPolicy(QTableView::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
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

    //Map model's columns to some of the controls
    m_frmdatmapper.reset(new QDataWidgetMapper);
    m_frmdatmapper->setModel(frm->getModel());
    m_frmdatmapper->addMapping(ui->spbFrmPartXOffset,  static_cast<int>(eFramesColumnsType::direct_XOffset) );
    m_frmdatmapper->addMapping(ui->spbFrmPartYOffset,  static_cast<int>(eFramesColumnsType::direct_YOffset) );
    m_frmdatmapper->addMapping(ui->btnFrmVFlip,        static_cast<int>(eFramesColumnsType::direct_VFlip) );
    m_frmdatmapper->addMapping(ui->btnFrmHFlip,        static_cast<int>(eFramesColumnsType::direct_HFlip) );
    m_frmdatmapper->toFirst();

    ConnectSignals();
    BaseSpriteTab::OnShowTab(spr, element);
}

void TabFrames::OnHideTab()
{
    DisconnectSignals();

    //Clear table view
    ui->tblframeparts->setItemDelegate(nullptr);
    ui->tblframeparts->setModel(nullptr);

    //Clear frame editor
    if(m_frmeditor)
        m_frmeditor->deInitScene();
    m_frmeditor.reset();
    ui->gvFrame->setScene(nullptr);

    //Clear mapper
    if(m_frmdatmapper)
        m_frmdatmapper->clearMapping();
    m_frmdatmapper.reset();

    //Clear current frame
    m_frame = QModelIndex();

    BaseSpriteTab::OnHideTab();
}

void TabFrames::setupFrameEditPageForPart(MFrame */*frm*/, MFramePart */*part*/)
{
//    if(!frm || !part)
//    {
//        qWarning("MainWindow::setupFrameEditPageForPart(): Got null frame or part! Skipping!");
//        HideAllTabs(); //Reset UI to try to avoid further issues
//        ui->tv_sprcontent->setCurrentIndex(QModelIndex()); //Reset currently selected sprite
//        ShowStatusErrorMessage("An error occured. UI reset! Please notify devs!");
//        return;
//    }
}



void TabFrames::OnDestruction()
{
    OnHideTab();
    BaseSpriteTab::OnDestruction();
}

void TabFrames::PrepareForNewContainer()
{
    OnHideTab();
    ui->tblframeparts->clearSelection();
    ui->tblframeparts->setModel(nullptr);
    ui->tblframeparts->reset();
}

void TabFrames::OnItemRemoval(const QModelIndex &item)
{
    BaseSpriteTab::OnItemRemoval(item);
    if(item == m_frame)
    {
        OnHideTab(); //Clear everything
    }
}


// *********************************
//  Frame Tab
// *********************************
void TabFrames::on_btnFrmRmPart_clicked()
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

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_btnFrmAdPart_clicked()
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

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_btnFrmMvUp_clicked()
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

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_btnFrmMvDown_clicked()
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

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_btnFrmDup_clicked()
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

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_cmbFrmQuickPrio_currentIndexChanged(int index)
{
    Q_ASSERT(false); //#TODO: Make this work!

    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void TabFrames::on_btnFrmExport_clicked()
{
    //#TODO: Move this outside the UI!!!
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


    QImage img(pfrm->AssembleFrame(0, 0, pfrm->calcFrameBounds()));
    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported assembled frame to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}

void TabFrames::OnSelectionChanged(QList<int> parts)
{
    ui->tblframeparts->blockSignals(true);
    try
    {
        ui->tblframeparts->clearSelection();
        if(!parts.empty())
        {
            //MFrame * frm = static_cast<MFrame*>(m_frame.internalPointer());
            //QItemSelection sel;
            for(int index : parts)
            {
        //        QModelIndex selectedpart = frm->index(index, 0, m_frame);
        //        sel.select(selectedpart,selectedpart);
                ui->tblframeparts->selectRow(index);
            }
        }
    }
    catch(...) //wish I had a finally block
    {
        ui->tblframeparts->blockSignals(false);
        throw;
    }
    ui->tblframeparts->blockSignals(false);
}
