#include "tabframes.hpp"
#include "ui_tabframes.h"

#include <src/ui/windows/diagsingleimgcropper.hpp>
#include <src/ui/windows/dialogabout.hpp>
#include <src/ui/mainwindow.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/models/framepart_delegate.hpp>
#include <src/utility/file_support.hpp>
#include <src/utility/graphics_util.hpp>

#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QImage>

const QString TabFrames::TabAttachmentPointsName{tr("Attachment Points")};

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
    connect(m_frmModel.data(),      &QAbstractItemModel::dataChanged, this, &TabFrames::OnDataChanged);
    if(m_attachModel)
        connect(m_attachModel.data(),   &QAbstractItemModel::dataChanged, this, &TabFrames::OnOffsetChanged);

    connect(m_frmeditor.data(), &FrameEditor::zoom,             this,               &TabFrames::OnFrameEditorZoom);
    connect(m_frmeditor.data(), &FrameEditor::selectionChanged, this,               &TabFrames::OnEditorSelectionChanged);
    connect(this,               &TabFrames::partSelected,       m_frmeditor.data(), &FrameEditor::OnViewPartsSelected);
    connect(this,               &TabFrames::markerSelected,     m_frmeditor.data(), &FrameEditor::OnViewMarkersSelected);
    connect(m_frmeditor.data(), &FrameEditor::mousePosUpdate,   getMainWindow(),    &MainWindow::updateCoordinateBar);

    //Init checkboxes state
    connect(ui->spbFrmZoom,             qOverload<int>(&QSpinBox::valueChanged), this, &TabFrames::On_spbFrmZoom_ValueChanged );
    connect(ui->chkColorPartOutlines,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawOutlines);
    connect(ui->chkFrmMiddleMarker,     &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
    connect(ui->chkFrmTransparency,     &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);
    connect(ui->chkDisplayMode,         &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDisplayNDSMode);

    //Connect mapper
    connect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);

    //Connect offset table
    connect(ui->tvAttachments, &QTableView::activated, this, &TabFrames::OnOffsetSelected);
}

void TabFrames::DisconnectSignals()
{
    disconnect(ui->spbFrmZoom, qOverload<int>(&QSpinBox::valueChanged), this, &TabFrames::On_spbFrmZoom_ValueChanged );

    MFrame * frm = static_cast<MFrame*>(m_frame.internalPointer());
    if(frm)
    {
        disconnect(m_frmModel.data(),       &QAbstractItemModel::dataChanged, this, &TabFrames::OnDataChanged);
        if(m_attachModel)
            disconnect(m_attachModel.data(),    &QAbstractItemModel::dataChanged, this, &TabFrames::OnOffsetChanged);
    }

    if(m_frmeditor)
    {
        disconnect(m_frmeditor.data(),      &FrameEditor::zoom,             this,               &TabFrames::OnFrameEditorZoom);
        disconnect(m_frmeditor.data(),      &FrameEditor::selectionChanged, this,               &TabFrames::OnEditorSelectionChanged);
        disconnect(this,                    &TabFrames::partSelected,       m_frmeditor.data(), &FrameEditor::OnViewPartsSelected);
        disconnect(this,                    &TabFrames::markerSelected,     m_frmeditor.data(), &FrameEditor::OnViewMarkersSelected);
        disconnect(m_frmeditor.data(),      &FrameEditor::mousePosUpdate,   getMainWindow(),    &MainWindow::updateCoordinateBar);
        disconnect(ui->spbFrmZoom,          qOverload<int>(&QSpinBox::valueChanged), this,      &TabFrames::On_spbFrmZoom_ValueChanged );
        disconnect(ui->chkColorPartOutlines,&QCheckBox::toggled,            m_frmeditor.data(), &FrameEditor::setDrawOutlines);
        disconnect(ui->chkFrmMiddleMarker,  &QCheckBox::toggled,            m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
        disconnect(ui->chkFrmTransparency,  &QCheckBox::toggled,            m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);
        disconnect(ui->chkDisplayMode,      &QCheckBox::toggled,            m_frmeditor.data(), &FrameEditor::setDisplayNDSMode);
    }

    if(m_frmdatmapper)
        disconnect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);

    disconnect(ui->tvAttachments, &QTableView::activated, this, &TabFrames::OnOffsetSelected);
}

bool TabFrames::setFrame(QPersistentModelIndex element, Sprite *spr)
{
    if(!element.isValid())
        return false;
    MFrame * frm = reinterpret_cast<MFrame*>(element.internalPointer());
    if(!frm)
        return false;
    Q_ASSERT(spr && frm);

    //Remove optional tab first
    int postab = ui->tabwFramesEdit->indexOf(ui->tabAttachPoints);
    if(postab != -1)
        ui->tabwFramesEdit->removeTab(postab);

    m_frame = element;
    m_frmModel      .reset(new MFramePartModel(frm, spr));
    m_frmDelegate   .reset(new MFramePartDelegate(frm));
    if(spr->hasEfxOffsets())
    {
        m_attachModel.reset(new EffectSetModel(spr->getAttachMarkers(frm->nodeIndex()), spr));
        m_attachDele.reset(new EffectSetDelegate(spr->getAttachMarkers(frm->nodeIndex())));
        ui->tabwFramesEdit->insertTab(0, ui->tabAttachPoints, TabAttachmentPointsName);
    }
    //ui->tblframeparts->openPersistentEditor();
    return true;
}

void TabFrames::clearFrame()
{
    m_frame = QModelIndex();
    m_frmModel.reset();
    m_frmDelegate.reset();
    m_attachModel.reset();
    m_attachDele.reset();
}

void TabFrames::updateListAndEditor()
{
    Q_ASSERT(m_frmeditor);
    m_frmeditor->updateScene();
    ui->gvFrame->update();
    ui->tblframeparts->update();
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
        m_frmeditor->updateScene();
    ui->gvFrame->update();

    ui->tblframeparts->update();
}

void TabFrames::OnOffsetChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    if(m_frmeditor)
        m_frmeditor->updateScene();
    ui->gvFrame->update();

    ui->tvAttachments->update();
}

void TabFrames::OnShowTab(QPersistentModelIndex element)
{
    Sprite * spr = currentSprite();
    if(!setFrame(element, spr))
    {
        throw ExBadFrame("TabFrames::OnShowTab(): Bad frame index!");
    }
    MFrame * frm = static_cast<MFrame*>(element.internalPointer());
    setupFramePartTable();
    qDebug() << "MainWindow::DisplayMFramePage(): Showing frame page!\n";

    //Setup the frame editor in the viewport!
    m_frmeditor.reset( new FrameEditor(frm, spr, m_frmModel.data(), m_attachModel.data()) );
    ui->gvFrame->setScene( &m_frmeditor->getScene() );
    m_frmeditor->initScene(ui->chkColorPartOutlines->isChecked(),
                           ui->chkFrmMiddleMarker->isChecked(),
                           ui->chkFrmTransparency->isChecked());
    ui->gvFrame->repaint();
    setupMappedControls();
    setupAttachTable();
    ConnectSignals();
    selectPart(ui->tblframeparts->model()->index(0, 0, QModelIndex()));
    if(spr->is256Colors())
        ui->tblframeparts->hideColumn(static_cast<int>(eFramePartColumnsType::PaletteID));
    else
        ui->tblframeparts->showColumn(static_cast<int>(eFramePartColumnsType::PaletteID));
    BaseSpriteTab::OnShowTab(element);
}

void TabFrames::OnHideTab()
{
    DisconnectSignals();

    //Clear table view
    ui->tblframeparts->setItemDelegate(nullptr);
    ui->tblframeparts->setModel(nullptr);

    ui->tvAttachments->clearSelection();
    ui->tvAttachments->setModel(nullptr);
    ui->tvAttachments->setItemDelegate(nullptr);

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
    clearFrame();

    BaseSpriteTab::OnHideTab();
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
    if(m_frmModel->removeRow(ind.row()))
        ShowStatusMessage(tr("Removed part!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    updateListAndEditor();
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

    //Do the insertion
    const int oldtilelen = curframe->calcCharBlocksLen();
    if(m_frmModel->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended part!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));


    if(insertpos == curframe->nodeChildCount() -1)
    {
        //If inserted at the end, set the old tile len as the tilenumber for the inserted frame
        QModelIndex newidx = m_frmModel->index(insertpos, static_cast<int>(eFramePartColumnsType::BlockNum), QModelIndex());
        m_frmModel->setData(newidx, oldtilelen, Qt::EditRole);
    }
    else
    {
        //If inserted anywhere else, recalculate tile usage for all parts
        curframe->optimizeCharBlocksUsage();
    }

    updateListAndEditor();
}

void TabFrames::_MovePart(bool up)
{
    MFrame * curframe = currentFrame();
    if(!curframe){Q_ASSERT(false); throw BaseException("TabFrames::_MovePart(): No currently active frame!");}
    const QItemSelectionModel * selmodel = ui->tblframeparts->selectionModel();
    const QModelIndexList       selected = selmodel->selectedRows();

    if(!selmodel->hasSelection())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }
    //Don't move if first item is at top already
    if(up && selected.front().row() == 0)
    {
        ShowStatusErrorMessage(tr("Can't move selection up!"));
        return;
    }
    //Don't move if last item is at end already
    if(!up && selected.back().row() == (m_frmModel->rowCount()-1))
    {
        ShowStatusErrorMessage(tr("Can't move selection down!"));
        return;
    }

    const int destRow = up? selected.front().row() - 1 : selected.back().row() + 1;
    if(m_frmModel->moveRows(selected, destRow))
        ShowStatusMessage(tr("Part moved!"));
    else
        ShowStatusErrorMessage(tr("Failed to move part due to internal issue!"));

    //Gotta re-calc each times things move
    curframe->optimizeCharBlocksUsage();
    updateListAndEditor();
}

void TabFrames::updatePartImgSource()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    if(!ind.isValid())
        return;

    MFramePart* ppart = static_cast<MFramePart*>(ind.internalPointer());
    if(ppart->isPartReference())
    {
        ui->cmbPartImage->setEnabled(false);
        ui->cmbBlock->setEnabled(true);
        //#TODO: Move me into some function plz!
        {
            Sprite * spr = currentSprite();

            //Fill up the currently used blocks, or in effect sprites the content of all image data
            if(spr->type() == fmt::eSpriteType::Effect)
            {
                FillComboBoxWithSpriteBlocks(spr, *ui->cmbBlock);
            }
            else
            {
                FillComboBoxWithFrameBlocks(spr, currentFrame(), *ui->cmbBlock);
            }
        }
    }
    else
    {
        ui->cmbPartImage->setEnabled(true);
        ui->cmbBlock->setEnabled(false);
    }

}

void TabFrames::on_btnFrmMvUp_clicked()
{
    _MovePart(true);
}

void TabFrames::on_btnFrmMvDown_clicked()
{
    _MovePart(false);
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

    int insertpos = ind.row() + 1; //We must insert references after
    if(m_frmModel->insertRow(insertpos))
    {
        MFramePart * pnewfrm = static_cast<MFramePart *>(m_frmModel->getItem( m_frmModel->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = tmppart;
        //A copy should be a -1 frame by default!!
        if(pnewfrm->getFrameIndex() != -1)
            pnewfrm->setFrameIndex(-1);
        ShowStatusMessage(tr("Created reference to original!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    updateListAndEditor();
}

void TabFrames::setupMappedControls()
{
    ui->spbPartXOffset->setRange(0, fmt::step_t::XOFFSET_MAX);
    ui->spbPartYOffset->setRange(0, fmt::step_t::YOFFSET_MAX);

    //Map model's columns to some of the controls
    m_frmdatmapper.reset(new QDataWidgetMapper);
    m_frmdatmapper->setModel(m_frmModel.data());
    m_frmdatmapper->setItemDelegate(m_frmDelegate.data());
    m_frmdatmapper->setSubmitPolicy(QDataWidgetMapper::SubmitPolicy::AutoSubmit);

    m_frmdatmapper->addMapping(ui->cmbPartImage,    static_cast<int>(eFramePartColumnsType::ImgID));
    FillComboBoxWithSpriteImages(currentSprite(), *ui->cmbPartImage);
    m_frmdatmapper->addMapping(ui->cmbPartPal,      static_cast<int>(eFramePartColumnsType::PaletteID));
    m_frmdatmapper->addMapping(ui->spbPartXOffset,  static_cast<int>(eFramePartColumnsType::XOffset));
    m_frmdatmapper->addMapping(ui->spbPartYOffset,  static_cast<int>(eFramePartColumnsType::YOffset));
    m_frmdatmapper->addMapping(ui->btnPartVFlip,    static_cast<int>(eFramePartColumnsType::VFlip));
    m_frmdatmapper->addMapping(ui->btnPartHFlip,    static_cast<int>(eFramePartColumnsType::HFlip));
    m_frmdatmapper->addMapping(ui->cmbPartPriority, static_cast<int>(eFramePartColumnsType::Priority));
    FillComboBoxWithFramePartPriorities(*ui->cmbPartPriority);
}

void TabFrames::setupFramePartTable()
{
    ui->tblframeparts->setModel(m_frmModel.data());
    ui->tblframeparts->setItemDelegate(m_frmDelegate.data());
    ui->tblframeparts->resizeRowsToContents();
    ui->tblframeparts->resizeColumnsToContents();
    ui->tblframeparts->horizontalHeader()->setStretchLastSection(true);
}

void TabFrames::setupAttachTable()
{
    ui->tvAttachments->setModel(m_attachModel.data());
    ui->tvAttachments->setItemDelegate(m_attachDele.data());
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
                        GetFileDialogDefaultExportPath(),
                        AllSupportedImagesFilesFilter());

    if(filename.isNull())
        return;
    UpdateFileDialogExportPath(filename);

    QImage img(pfrm->AssembleFrame(0, 0, pfrm->calcFrameBounds(), nullptr, true, currentSprite()));
    if(img.save(filename))
        ShowStatusMessage(QString(tr("Exported assembled frame to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}

void TabFrames::OnEditorSelectionChanged(QList<EditableItem *> parts)
{
    for(EditableItem * p : parts)
    {
        if(p->getDataType() == eTreeElemDataType::framepart)
        {
            QSignalBlocker blk(ui->tblframeparts);
            selectPart(p->getItemIndex());
            //ui->tblframeparts->selectRow(p->getItemIndex().row());
        }
        else if(p->getDataType() == eTreeElemDataType::effectOffset)
        {
            QSignalBlocker blk(ui->tvAttachments);
            ui->tvAttachments->selectRow(p->getItemIndex().row());
        }
    }
    ui->tblframeparts->update();
    ui->tvAttachments->update();
}

void TabFrames::OnOffsetSelected(QModelIndex selected)
{
    m_frmeditor->selectMarker(selected);
}

void TabFrames::on_btnEditAttachments_toggled(bool checked)
{
    m_frmeditor->setEditorMode(checked? eEditorMode::AttachmentPoints : eEditorMode::FrameParts);
}

void TabFrames::on_tblframeparts_clicked(const QModelIndex &index)
{
    selectPart(index);
}

void TabFrames::on_chkGridSnap_toggled(bool checked)
{
    m_frmeditor->setGridSnap(checked);
}

void TabFrames::on_tvAttachments_clicked(const QModelIndex &index)
{
    ui->tblframeparts->clearSelection();
    if(!index.isValid())
    {
        ui->tvAttachments->clearSelection();
        return;
    }
    emit markerSelected(ui->tvAttachments->selectionModel()->selectedRows());
}

void TabFrames::selectPart(const QModelIndex & index)
{
    ui->tvAttachments->clearSelection();
    if(!index.isValid())
    {
        ui->tblframeparts->clearSelection();
        m_frmdatmapper->setCurrentModelIndex(QModelIndex());
        ui->tabPartProps->setEnabled(false);
        emit partSelected(ui->tblframeparts->selectionModel()->selectedRows());
        return;
    }
    ui->tabwFramesEdit->setCurrentWidget(ui->tabPartProps);
    ui->tabPartProps->setEnabled(true);
    MFramePart * part = static_cast<MFramePart*>(index.internalPointer());
    FillComboBoxWithSpritePalettes(currentSprite(), *ui->cmbPartPal, part->isColorPal256());
    m_frmdatmapper->setCurrentModelIndex(index);
    ui->tblframeparts->setCurrentIndex(index);
    emit partSelected(ui->tblframeparts->selectionModel()->selectedRows());
}

void TabFrames::on_chkImgSourceRef_toggled(bool checked)
{
    QModelIndex index = ui->tblframeparts->selectionModel()->currentIndex();
    if(!index.isValid())
        return;
    MFramePart * part = static_cast<MFramePart*>(index.internalPointer());
    part->setIsReference(checked);
    updatePartImgSource();
}
