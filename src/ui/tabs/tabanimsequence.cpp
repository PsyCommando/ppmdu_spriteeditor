#include "tabanimsequence.hpp"
#include "ui_tabanimsequence.h"
#include <QFileDialog>

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

void TabAnimSequence::OnShowTab(Sprite * pspr, QPersistentModelIndex element)
{
    Q_ASSERT(pspr && element.isValid());
    AnimSequence * aniseq = static_cast<AnimSequence*>(element.internalPointer());
    ui->tblseqfrmlst->setModel(aniseq->getModel());
    ui->tblseqfrmlst->setItemDelegate(aniseq->getDelegate());
    ui->tblseqfrmlst->resizeRowsToContents();
    ui->tblseqfrmlst->resizeColumnsToContents();

    m_previewrender.InstallAnimPreview(ui->gvAnimSeqViewport, pspr, aniseq);
    ConnectSceneRenderer();
    OnPreviewRangeChanged(0, m_previewrender.getAnimationLength()); //Force set the preview range
    qDebug() << "TabAnimSequence::ShowTab(): Scene set!\n";
    BaseSpriteTab::OnShowTab(pspr, element);
}

void TabAnimSequence::OnHideTab()
{
    DisconnectSceneRenderer();
    m_previewrender.UninstallAnimPreview(ui->gvAnimSeqViewport);

    OnPreviewFrameChanged(0, QRectF()); //Reset the slider and spinbox to 0!
    ui->tblseqfrmlst->clearSelection();
    ui->tblseqfrmlst->setModel(nullptr);
    ui->tblseqfrmlst->setItemDelegate(nullptr);
    ui->tblseqfrmlst->reset();
    qDebug() << "TabAnimSequence::HideTab(): Unset scene!\n";
    BaseSpriteTab::OnHideTab();
}

//Called before shared data is destroyed, so we get a chance to cleanup
void TabAnimSequence::OnDestruction()
{
    DisconnectSceneRenderer();
    m_previewrender.UninstallAnimPreview(ui->gvAnimSeqViewport);
}

void TabAnimSequence::PrepareForNewContainer()
{
    OnHideTab();
    ui->tblseqfrmlst->clearSelection();
    ui->tblseqfrmlst->setModel(nullptr);
    ui->tblseqfrmlst->setItemDelegate(nullptr);
    ui->tblseqfrmlst->reset();
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
}

void TabAnimSequence::DisconnectSceneRenderer()
{
    //Connect UI to scene
    //disconnect( ui->chkAnimSeqLoop,&QCheckBox::toggled,    &m_previewrender, &SpriteScene::setShouldLoop );
    //disconnect( ui->btnSeqPlay,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::beginAnimationPlayback );
    //disconnect( ui->btnSeqStop,    &QPushButton::clicked,  &m_previewrender, &SpriteScene::endAnimationPlayback );
    //disconnect( ui->sldrAnimSeq,   &QSlider::sliderMoved,  &m_previewrender, &SpriteScene::setCurrentFrame);

    //Connect scene signals to UI
    disconnect( &m_previewrender, &SpriteScene::rangechanged, this, &TabAnimSequence::OnPreviewRangeChanged);
    disconnect( &m_previewrender, &SpriteScene::framechanged, this, &TabAnimSequence::OnPreviewFrameChanged);
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
}

void TabAnimSequence::OnPreviewFrameChanged(int curfrm, QRectF /*area*/)
{
    //Block signals so we don't end up with infinite recursion and stuff!
    ui->spinCurFrm->blockSignals(true);
    ui->spinCurFrm->setValue(curfrm);
    ui->spinCurFrm->blockSignals(false);

    ui->sldrAnimSeq->blockSignals(true);
    ui->sldrAnimSeq->setValue(curfrm);
    ui->sldrAnimSeq->blockSignals(false);
}

//-------------------------------------------------------------------------
//  Sequence Editing
//-------------------------------------------------------------------------
void TabAnimSequence::on_btnSeqAddFrm_clicked()
{
    //#TODO: Separate this from ui code if possible!
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

    ui->tblseqfrmlst->setCurrentIndex(QModelIndex());
    if(pseq->removeChildrenNodes(ind.row(), 1))
        ShowStatusMessage(tr("Removed animation frame!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void TabAnimSequence::on_btnSeqMvUp_clicked()
{
    //#TODO: Separate this from ui code if possible!
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

void TabAnimSequence::on_btnSeqMvDown_clicked()
{
    //#TODO: Separate this from ui code if possible!
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

void TabAnimSequence::on_btnSeqDup_clicked()
{
    //#TODO: Separate this from ui code if possible!
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
    m_previewrender.setCurrentFrame(position);
    ui->spinCurFrm->blockSignals(true);
    ui->spinCurFrm->setValue(position);
    ui->spinCurFrm->blockSignals(false);
}

void TabAnimSequence::on_spinCurFrm_editingFinished()
{
    m_previewrender.setCurrentFrame(ui->spinCurFrm->value());
    ui->sldrAnimSeq->blockSignals(true);
    ui->sldrAnimSeq->setValue(ui->spinCurFrm->value());
    ui->sldrAnimSeq->blockSignals(false);
}

void TabAnimSequence::on_tblseqfrmlst_activated(const QModelIndex &/*index*/)
{
    //When clicking on a frame in the frame list, stop the anim preview
    m_previewrender.endAnimationPlayback();
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
                        "PNG image (*.png)");

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


