#include "tabanimsequence.hpp"
#include "ui_tabanimsequence.h"
#include <QFileDialog>
#include <QSettings>
#include <src/utility/file_support.hpp>
#include <src/utility/program_settings.hpp>

const QString DEFAULT_PLAYTIME_VALUE = "----- t";

TabAnimSequence::TabAnimSequence(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::tabAnimSequence)
{
    ui->setupUi(this);
    ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
}

TabAnimSequence::~TabAnimSequence()
{
    delete ui;
}

void TabAnimSequence::OnShowTab(QPersistentModelIndex element)
{
    if(element.isValid())
    {
        Sprite * pspr = currentSprite();
        if(pspr)
        {
            AnimSequence * aniseq = static_cast<AnimSequence*>(element.internalPointer());
            setupAnimSeq(element, pspr);
            ui->tblseqfrmlst->setModel(m_curSeqFramesModel.data());
            ui->tblseqfrmlst->setItemDelegate(m_curSeqFramesDelegate.data());
            ui->tblseqfrmlst->resizeRowsToContents();
            ui->tblseqfrmlst->resizeColumnsToContents();

            m_previewrender.InstallAnimPreview(ui->gvAnimSeqViewport, pspr, aniseq);
            ConnectSceneRenderer();
            OnPreviewRangeChanged(0, m_previewrender.getAnimationLength()); //Force set the preview range
            qDebug() << "TabAnimSequence::ShowTab(): Scene set!\n";
            if(ProgramSettings::Instance().isAutoplayEnabled())
                m_previewrender.beginAnimationPlayback();
        }
    }
    BaseSpriteTab::OnShowTab(element);
}

void TabAnimSequence::OnHideTab()
{
    m_previewrender.endAnimationPlayback();
    DisconnectSceneRenderer();
    m_previewrender.UninstallAnimPreview(ui->gvAnimSeqViewport);

    OnPreviewFrameChanged(0, QRectF()); //Reset the slider and spinbox to 0!
    ui->tblseqfrmlst->clearSelection();
    ui->tblseqfrmlst->setModel(nullptr);
    ui->tblseqfrmlst->setItemDelegate(nullptr);
    ui->tblseqfrmlst->reset();

    ui->lblAnimTime->setText(DEFAULT_PLAYTIME_VALUE);
    clearAnimSeq();
    qDebug() << "TabAnimSequence::HideTab(): Unset scene!\n";
    BaseSpriteTab::OnHideTab();
}

//Called before shared data is destroyed, so we get a chance to cleanup
void TabAnimSequence::OnDestruction()
{
    DisconnectSceneRenderer();
    m_previewrender.endAnimationPlayback();
    m_previewrender.Reset();
    m_previewrender.UninstallAnimPreview(ui->gvAnimSeqViewport);
    clearAnimSeq();
    BaseSpriteTab::OnDestruction();
}

void TabAnimSequence::OnItemRemoval(const QModelIndex &item)
{
    if(!item.isValid())
        return;

    //Removing our parent sprite! Or our animation sequence!
    if( item.internalPointer() == currentSprite() ||
        item.internalPointer() == currentAnimSequence())
        PrepareForNewContainer(); //Clear everything!

    BaseSpriteTab::OnItemRemoval(item);
}

void TabAnimSequence::PrepareForNewContainer()
{
    OnHideTab();
    ui->tblseqfrmlst->clearSelection();
    ui->tblseqfrmlst->setModel(nullptr);
    ui->tblseqfrmlst->setItemDelegate(nullptr);
    ui->tblseqfrmlst->reset();
}

void TabAnimSequence::setupAnimSeq(QPersistentModelIndex seq, Sprite *spr)
{
    if(!seq.isValid())
    {
        Q_ASSERT(false);
    }
    AnimSequence * pseq = reinterpret_cast<AnimSequence*>(seq.internalPointer());
    if(!pseq)
    {
        Q_ASSERT(false);
    }
    m_curAnimSeq = seq;
    m_curSeqFramesModel.reset(new AnimFramesModel(pseq, spr));
    m_curSeqFramesDelegate.reset(new AnimFrameDelegate(pseq, spr));
}

void TabAnimSequence::clearAnimSeq()
{
    m_curAnimSeq = QModelIndex();
    m_curSeqFramesModel.reset();
    m_curSeqFramesDelegate.reset();
}

void TabAnimSequence::RefreshTable()
{
    ui->tblseqfrmlst->resizeColumnsToContents();
    ui->tblseqfrmlst->resizeRowsToContents();
    ui->tblseqfrmlst->repaint();
    ui->tblseqfrmlst->update();
}

void TabAnimSequence::ConnectSceneRenderer()
{
    //Connect UI to scene
    //connect( ui->chkAnimSeqLoop,&QCheckBox::toggled,    &m_previewrender, &SpriteScene::setShouldLoop );
    //connect( ui->btnSeqPlay,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::beginAnimationPlayback );
    //connect( ui->btnSeqStop,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::endAnimationPlayback );
    //connect( ui->sldrAnimSeq,   &QSlider::sliderMoved,  &m_previewrender, &SpriteScene::setCurrentFrame);

    //Connect scene signals to UI
    connect( &m_previewrender, &SpriteScene::rangechanged, this, &TabAnimSequence::OnPreviewRangeChanged);
    connect( &m_previewrender, &SpriteScene::framechanged, this, &TabAnimSequence::OnPreviewFrameChanged);
    connect( &m_previewrender, &SpriteScene::tick,         this, &TabAnimSequence::OnPreviewTick);

    //Connect data update signals
    connect( m_curSeqFramesModel.data(), &AnimFramesModel::dataChanged, &m_previewrender, &SpriteScene::OnAnimDataChaged);
    //connect( m_curSeqFramesDelegate.data(), &AnimFrameDelegate::SlotChanged, &m_previewrender, &SpriteScene::OnAnimDataChaged);
}

void TabAnimSequence::DisconnectSceneRenderer()
{
    //Connect UI to scene
    //disconnect( ui->chkAnimSeqLoop,&QCheckBox::toggled,    &m_previewrender, &SpriteScene::setShouldLoop );
    //disconnect( ui->btnSeqPlay,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::beginAnimationPlayback );
    //disconnect( ui->btnSeqStop,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::endAnimationPlayback );
    //disconnect( ui->sldrAnimSeq,   &QSlider::sliderMoved,  &m_previewrender, &SpriteScene::setCurrentFrame);

    //Disconnect scene signals to UI
    disconnect( &m_previewrender, &SpriteScene::rangechanged, this, &TabAnimSequence::OnPreviewRangeChanged);
    disconnect( &m_previewrender, &SpriteScene::framechanged, this, &TabAnimSequence::OnPreviewFrameChanged);
    disconnect( &m_previewrender, &SpriteScene::tick,         this, &TabAnimSequence::OnPreviewTick);

    //Disconnect data update signals
    if(m_curSeqFramesModel)
        disconnect( m_curSeqFramesModel.data(), &AnimFramesModel::dataChanged, &m_previewrender, &SpriteScene::OnAnimDataChaged);
}

void TabAnimSequence::OnPreviewRangeChanged(int beg, int length)
{
    //Block signals so we don't end up with infinite recursion and stuff!
    ui->spinCurFrm->blockSignals(true);
    ui->spinCurFrm->setRange(0, length - 1); //since we start at 0, subtract 1
    ui->spinCurFrm->blockSignals(false);

    ui->sldrAnimSeq->blockSignals(true);
    ui->sldrAnimSeq->setRange(0, length - 1); //since we start at 0, subtract 1
    ui->sldrAnimSeq->blockSignals(false);
    UpdateTickCounter();
    ui->gvAnimSeqViewport->updateScene(QList{ui->gvAnimSeqViewport->sceneRect()});
}

void TabAnimSequence::OnPreviewFrameChanged(int curfrm, QRectF /*area*/)
{
    //Block signals so we don't end up with infinite recursion and stuff!
//    ui->spinCurFrm->blockSignals(true);
//    ui->spinCurFrm->setValue(curfrm);
//    ui->spinCurFrm->blockSignals(false);

//    ui->sldrAnimSeq->blockSignals(true);
//    ui->sldrAnimSeq->setValue(curfrm);
//    ui->sldrAnimSeq->blockSignals(false);
//    UpdateTickCounter();

    SetCurrentFrame(curfrm, false);
}

void TabAnimSequence::OnPreviewTick(int /*curtick*/)
{
    UpdateTickCounter();
}

void TabAnimSequence::UpdateTickCounter()
{
    ui->lblAnimTime->setText(QString("%05d t").arg(m_previewrender.getTimeElapsed()));
}

void TabAnimSequence::SetCurrentFrame(int frameidx, bool bupdatescene)
{
    if(bupdatescene)
        m_previewrender.setCurrentFrame(frameidx);

    //Update the spinner
    ui->spinCurFrm->blockSignals(true);
    ui->spinCurFrm->setValue(frameidx);
    ui->spinCurFrm->blockSignals(false);

    //Update the slider
    ui->sldrAnimSeq->blockSignals(true);
    ui->sldrAnimSeq->setValue(frameidx);
    ui->sldrAnimSeq->blockSignals(false);

    //Select the frame in the table view
//    AnimSequence * curseq = currentAnimSequence();
//    if(curseq)
//        ui->tblseqfrmlst->setCurrentIndex(curseq->modelIndex());

    //Update the tick counter
    UpdateTickCounter();
}

//-------------------------------------------------------------------------
//  Sequence Editing
//-------------------------------------------------------------------------
void TabAnimSequence::on_btnSeqAddFrm_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QModelIndex     ind       = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = static_cast<AnimSequence*>(m_curAnimSeq.internalPointer());
    Q_ASSERT(curseq);
    int insertpos = 0;

    if(ind.isValid())
        insertpos = ind.row();
    else
        insertpos = (curseq->nodeChildCount() > 0)? (curseq->nodeChildCount() - 1) : 0;

    if(m_curSeqFramesModel->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));

    m_previewrender.reloadAnim();
    RefreshTable();
}

void TabAnimSequence::on_btnSeqRemFrm_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No frame selected!"));
        return;
    }
    AnimFrame       * pafrm = static_cast<AnimFrame*>(ind.internalPointer());
    AnimSequence    * pseq  = static_cast<AnimSequence*>(pafrm->parentNode());
    Q_ASSERT(pseq && pafrm);

    QItemSelectionModel* sel = ui->tblseqfrmlst->selectionModel();
    QModelIndexList selectedindices = sel->selectedRows();

    //ui->tblseqfrmlst->setCurrentIndex(QModelIndex());
    if(m_curSeqFramesModel->removeRows(selectedindices))// pseq->removeChildrenNodes(selectedindex))
        ShowStatusMessage(tr("Removed animation frame!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    sel->clearSelection();
    m_previewrender.reloadAnim();
    RefreshTable();
}

void TabAnimSequence::on_btnSeqMvUp_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = static_cast<AnimSequence*>(m_curAnimSeq.internalPointer());
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() > 0)? ind.row() - 1 : ind.row();
    if(destrow != ind.row())
    {
        if(m_curSeqFramesModel->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    RefreshTable();
}

void TabAnimSequence::on_btnSeqMvDown_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = static_cast<AnimSequence*>(m_curAnimSeq.internalPointer());
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() < curseq->nodeChildCount()-1 )? ind.row() + 1 : ind.row();
    if(destrow != ind.row())
    {
        if(m_curSeqFramesModel->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    RefreshTable();
}

void TabAnimSequence::on_btnSeqDup_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QModelIndex     ind     = ui->tblseqfrmlst->currentIndex();
    AnimSequence*   curseq  = static_cast<AnimSequence*>(m_curAnimSeq.internalPointer());
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    AnimFrame * tmpfrm = static_cast<AnimFrame*>(ind.internalPointer());

    int insertpos = ind.row();
    if(m_curSeqFramesModel->insertRow(insertpos))
    {
        AnimFrame * pnewfrm = static_cast<AnimFrame *>(m_curSeqFramesModel->getItem( m_curSeqFramesModel->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = *tmpfrm;
        ShowStatusMessage(tr("Duplicated animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    m_previewrender.reloadAnim();
    RefreshTable();
}

//-------------------------------------------------------------------------
//  Anim Playback
//-------------------------------------------------------------------------
void TabAnimSequence::on_btnSeqPlay_clicked()
{
    m_previewrender.beginAnimationPlayback();
}

void TabAnimSequence::on_btnSeqStop_clicked()
{
    m_previewrender.endAnimationPlayback();

    //AnimSequence * curseq = currentAnimSequence();
    //(*curseq)[m_previewrender.getCurrentFrame()];
    //ui->tblseqfrmlst->scrollTo();
}

void TabAnimSequence::on_chkAnimSeqLoop_toggled(bool checked)
{
    m_previewrender.setShouldLoop(checked);
}

void TabAnimSequence::on_sldrAnimSeq_sliderMoved(int position)
{
//    m_previewrender.setCurrentFrame(position);
//    ui->spinCurFrm->blockSignals(true);
//    ui->spinCurFrm->setValue(position);
//    ui->spinCurFrm->blockSignals(false);

    SetCurrentFrame(position);
}

void TabAnimSequence::on_spinCurFrm_editingFinished()
{
//    m_previewrender.setCurrentFrame(ui->spinCurFrm->value());
//    ui->sldrAnimSeq->blockSignals(true);
//    ui->sldrAnimSeq->setValue(ui->spinCurFrm->value());
//    ui->sldrAnimSeq->blockSignals(false);

    SetCurrentFrame(ui->spinCurFrm->value());
}

void TabAnimSequence::on_tblseqfrmlst_activated(const QModelIndex &index)
{
    //When clicking on a frame in the frame list, stop the anim preview
    m_previewrender.endAnimationPlayback();
    SetCurrentFrame(index.row(), true);
}

void TabAnimSequence::on_tblseqfrmlst_clicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    SetCurrentFrame(index.row());
}

void TabAnimSequence::on_chkDisplayImgBorder_toggled(bool checked)
{
    if(checked)
        ui->gvAnimSeqViewport->setBackgroundBrush(QBrush(QColor("black")));
    else
        ui->gvAnimSeqViewport->setBackgroundBrush(QBrush(m_previewrender.getSpriteBGColor()));
}

void TabAnimSequence::on_chkAutoplay_toggled(bool checked)
{
    if(checked)
    {
        m_previewrender.beginAnimationPlayback();
    }
    else
    {
        m_previewrender.endAnimationPlayback();
    }
}

//-------------------------------------------------------------------------
//  Import/Export
//-------------------------------------------------------------------------
void TabAnimSequence::on_btnSeqImport_clicked()
{
    //#TODO
    ShowStatusErrorMessage(tr("TODO"));
}

void TabAnimSequence::on_btnSeqExport_clicked()
{
    //#TODO: Separate this from ui code if possible!
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Images Sequence : Pick name+path first image!"),
                        QString(),
                        AllSupportedImagesFilesFilter());

    if(filename.isNull())
        return;

    int rmpast = filename.size() - filename.lastIndexOf('.');
    if( rmpast > 0 && rmpast < filename.size() )
        filename.chop(rmpast);
    QVector<QImage> sequence = m_previewrender.DumpSequence();

    if(sequence.isEmpty())
        ShowStatusErrorMessage(tr("Error: No sequence was loaded for export!"));

    int cntimg = 0;
    for(; cntimg < sequence.size(); ++cntimg )
        sequence[cntimg].save( QString("%1_%2.png").arg(filename).arg(cntimg) );

    ShowStatusMessage(QString(tr("Exported %1 images!")).arg(cntimg));
}

//-------------------------------------------------------------------------
//  Settings
//-------------------------------------------------------------------------
const QString TabAnimSequence_GroupName = "TabAnimSequence";
const QString TabAnimSequence_Autoplay = "Autoplay";

void TabAnimSequence::writeSettings()
{
}

void TabAnimSequence::readSettings()
{
}
