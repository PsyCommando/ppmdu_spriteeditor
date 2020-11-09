#include "tabeffects.hpp"
#include "ui_tabeffects.h"

TabEffects::TabEffects(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabEffects)
{
    ui->setupUi(this);
    ui->numXOffset->setRange(-255, 255); //10bits
    ui->numYOffset->setRange(-127, 127); //9bits
}

TabEffects::~TabEffects()
{
    delete ui;
}

void TabEffects::PrepareForNewContainer()
{
    OnHideTab();
}

void TabEffects::ConnectSignals()
{
    //Connect mapper
    //connect(ui->tvOffsets, &QTableView::activated, m_offsetdatamapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}

void TabEffects::DisconnectSignals()
{
    //Disconnect mapper
    //disconnect(ui->tvOffsets, &QTableView::activated, m_offsetdatamapper.data(), &QDataWidgetMapper::setCurrentModelIndex);
}

void TabEffects::OnShowTab(QPersistentModelIndex element)
{
    if(!element.isValid())
        return;
    Sprite* spr = currentSprite();
    EffectOffsetSet * set = static_cast<EffectOffsetSet *>(element.internalPointer());
    m_currentSet = element;

    m_effectModel.reset(new EffectSetModel(set, spr));
    ui->tvOffsets->setModel(m_effectModel.data());
    ui->tvOffsets->adjustSize();

    m_offsetdatamapper.reset(new QDataWidgetMapper);
    m_offsetdatamapper->setModel(m_effectModel.data());
    m_offsetdatamapper->addMapping(ui->numXOffset,  static_cast<int>(EffectSetModel::eColumns::XOffset) );
    m_offsetdatamapper->addMapping(ui->numYOffset,  static_cast<int>(EffectSetModel::eColumns::YOffset) );
    m_offsetdatamapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    MFrame * pfrm = spr->getFrame(set->nodeIndex());
    if(!pfrm)
    {
        //No matching frame.. Display empty
        ui->lblFrameID->setText(tr("<html><head/><body><p><span style=\" font-weight:600; color:#aa0000;\">No matching frame!!</span></p></body></html>"));
    }
    else
    {
        ui->lblFrameID->setText(QString(tr("Frame: %1")).arg(pfrm->nodeIndex()));
        m_preview.InstallPreview(ui->gvEffectsPreview, m_effectModel.data(), spr, pfrm->nodeIndex());
    }

    m_offsetMarker.reset(new OffsetMarkerItem("+"));
    m_offsetMarker->setZValue(999);
    m_offsetMarker->setEnabled(false);
    ConnectSignals();
    BaseSpriteTab::OnShowTab(element);
}

void TabEffects::OnHideTab()
{
    m_offsetMarker.reset();
    m_preview.UninstallPreview(ui->gvEffectsPreview);
    DisconnectSignals();
    m_offsetdatamapper.reset();
    ui->tvOffsets->clearSelection();

    m_effectModel.reset();
    ui->tvOffsets->setModel(nullptr);
    m_currentSet = QModelIndex();
    BaseSpriteTab::OnHideTab();
}

void TabEffects::OnDestruction()
{
    OnHideTab();
    BaseSpriteTab::OnDestruction();
}

void TabEffects::OnItemRemoval(const QModelIndex &item)
{
    BaseSpriteTab::OnItemRemoval(item);
}

void TabEffects::SelectOffset(const QModelIndex & element)
{
//    Sprite * spr = currentSprite();
//    Q_ASSERT(spr);
//    AnimSequence * pseq = spr->getAnimSequence(m_currentSet.row());
//    if(!pseq)
//        return;

//    EffectOffset * off = static_cast<EffectOffset*>(element.internalPointer());
//    Q_ASSERT(off);
    m_offsetdatamapper->setCurrentModelIndex(element);
//    m_offsetMarker->setPos(off->getX(), off->getY());
//    m_offsetMarker->setEnabled(true);
//    m_previewrender.addGraphicsItem(m_offsetMarker.data());
    m_preview.setSelected(element);

}

void TabEffects::UnselectOffset()
{
    m_offsetdatamapper->setCurrentModelIndex(QModelIndex());
//    m_offsetMarker->setEnabled(false);
//    m_previewrender.removeGraphicsItem(m_offsetMarker.data());
    m_preview.Clear();
}

void TabEffects::on_chkPlayAnim_toggled(bool checked)
{
//    if(checked)
//        m_previewrender.beginAnimationPlayback();
//    else
//        m_previewrender.endAnimationPlayback();
}

void TabEffects::on_tvOffsets_activated(const QModelIndex &index)
{
    SelectOffset(index);
}

void TabEffects::on_tvOffsets_clicked(const QModelIndex &index)
{
    if(index.isValid())
        SelectOffset(index);
    else
        UnselectOffset();
}


void OffsetMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setCompositionMode(QPainter::CompositionMode::CompositionMode_Multiply);
    QGraphicsTextItem::paint(painter, option, widget);
}
