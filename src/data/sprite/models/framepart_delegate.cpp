#include "framepart_delegate.hpp"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QApplication>
#include <QStyle>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/imagescontainer.hpp>
#include <src/utility/graphics_util.hpp>

//=====================================================================================
//  MFrameDelegate
//=====================================================================================
MFramePartDelegate::MFramePartDelegate(MFrame *frm, QObject *parent)
    :QStyledItemDelegate(parent), m_minusone(MinusOneImgRes)
{
    m_pfrm = frm;
}

MFramePartDelegate::MFramePartDelegate(const MFramePartDelegate & cp)
    :QStyledItemDelegate(cp.parent()), m_minusone(MinusOneImgRes)
{
    operator=(cp);
}

MFramePartDelegate::MFramePartDelegate(MFramePartDelegate && mv)
    :QStyledItemDelegate(mv.parent()), m_minusone(MinusOneImgRes)
{
    operator=(mv);
}

MFramePartDelegate & MFramePartDelegate::operator=(const MFramePartDelegate & cp)
{
    setParent(cp.parent());
    m_pfrm    = cp.m_pfrm;
    return *this;
}

MFramePartDelegate & MFramePartDelegate::operator=(MFramePartDelegate && mv)
{
    setParent(mv.parent());
    m_pfrm    = mv.m_pfrm;
    mv.m_pfrm = nullptr;
    return *this;
}

QSize MFramePartDelegate::sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
{
    if(!index.isValid())
    {
        Q_ASSERT(false);
        return QSize();
    }

    const QAbstractItemModel * model = index.model();
    if(model)
        return model->data(index, Qt::SizeHintRole).toSize();
    else
        return QSize();
}


MFramePartDelegate::~MFramePartDelegate()
{
}

QWidget *MFramePartDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QWidget *pedit = nullptr;
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::ImgID:
        pedit = makeImgSelect(parent, index);
        break;
    case eFramePartColumnsType::BlockNum:
        pedit = makeBlockNumSelect(parent, index);
        break;
    case eFramePartColumnsType::PaletteID:
        pedit = makePaletteIDSelect(parent, index);
        break;
    case eFramePartColumnsType::XOffset:
        pedit = makeXOffsetSelect(parent, index.row());
        break;
    case eFramePartColumnsType::YOffset:
        pedit = makeYOffsetSelect(parent, index.row());
        break;
    case eFramePartColumnsType::Mode:
        pedit = makeModeSelect(parent, index.row());
        break;
    case eFramePartColumnsType::Priority:
        pedit = makePrioritySelect(parent, index.row());
        break;
    case eFramePartColumnsType::RnSParam:
        pedit = makeRotnScalingParamSelect(parent);
        break;
    default:
        //nothing here!
        return QStyledItemDelegate::createEditor(parent,option,index);
        break;
    };

    //Common properties
    if(pedit != nullptr)
    {
        pedit->setAutoFillBackground(true);
        pedit->setProperty(MFrame::PropPartID, index.row());
    }
    return pedit;
}

void MFramePartDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
        case eFramePartColumnsType::ImgID:
        {
            QComboBox * pimglist = static_cast<QComboBox*>(editor);
            Q_ASSERT(pimglist);

            if(part->getFrameIndex() >= 0)
                pimglist->setCurrentIndex(part->getFrameIndex() + 1); //add one, because 0 is reserved!!
            else
                pimglist->setCurrentIndex(0); //index 0 is -1 frame!
            break;
        }
        case eFramePartColumnsType::XOffset:
        {
            QSpinBox *px = static_cast<QSpinBox*>(editor);
            Q_ASSERT(px);
            px->setValue(part->getXOffset());
            break;
        }
        case eFramePartColumnsType::YOffset:
        {
            QSpinBox *py = static_cast<QSpinBox*>(editor);
            Q_ASSERT(py);
            py->setValue(part->getYOffset());
            break;
        }
//        case eFramePartColumnsType::VFlip:
//        {
//            QCheckBox *pvf = static_cast<QCheckBox*>(editor);
//            Q_ASSERT(pvf);
//            pvf->setChecked(part->isVFlip());
//            break;
//        }
//        case eFramePartColumnsType::HFlip:
//        {
//            QCheckBox *phf = static_cast<QCheckBox*>(editor);
//            Q_ASSERT(phf);
//            phf->setChecked(part->isHFlip());
//            break;
//        }
        case eFramePartColumnsType::PaletteID:
        {
            QComboBox *ppalid = static_cast<QComboBox*>(editor);
            Q_ASSERT(ppalid);
            ppalid->setCurrentIndex(part->getPalNb());
            break;
        }
        case eFramePartColumnsType::Mode:
        {
            QComboBox *pobjmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pobjmode);
            pobjmode->setCurrentIndex(static_cast<int>(part->getObjMode()));
            break;
        }
        case eFramePartColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            pprio->setCurrentIndex(part->getPriority());
            break;
        }
        case eFramePartColumnsType::BlockNum:
        {
            //Keep in mind that, blocknums can go to really high numbers on specific sprite types. Numbers that don't make sense for block nums


            Q_ASSERT(false); //Need to fix this since we wanna use a combobox for looking up blocks instead of just numbers
            QSpinBox *ptid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ptid);
            ptid->setValue(part->getBlockNum());
            break;
        }
        case eFramePartColumnsType::RnSParam:
        {
            QSpinBox *prnsparam = static_cast<QSpinBox*>(editor);
            Q_ASSERT(prnsparam);
            prnsparam->setValue(part->getRnSParam());
            break;
        }
        default:
            QStyledItemDelegate::setEditorData(editor,index);
        break;
    };

}

void MFramePartDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
        case eFramePartColumnsType::ImgID:
        {
            QComboBox   *pimglist = static_cast<QComboBox*>(editor);
            Q_ASSERT(pimglist);
            if(pimglist->currentIndex() == 0)
                model->setData(index, -1, Qt::EditRole);
            else
                model->setData(index, (pimglist->currentIndex() - 1), Qt::EditRole);
            break;
        }
        //Spinboxes
        case eFramePartColumnsType::RnSParam:
        case eFramePartColumnsType::XOffset:
        case eFramePartColumnsType::YOffset:
        case eFramePartColumnsType::BlockNum:
        {
            QSpinBox * poff = static_cast<QSpinBox*>(editor);
            Q_ASSERT(poff);
            poff->interpretText();
            model->setData(index, poff->value(), Qt::EditRole);
            break;
        }
        //Comboboxes
        case eFramePartColumnsType::PaletteID:
        case eFramePartColumnsType::Mode:
        case eFramePartColumnsType::Priority:
        case eFramePartColumnsType::BlockRange:
        {
            QComboBox *pcombo = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcombo);
            model->setData(index, pcombo->currentIndex(), Qt::EditRole);
            break;
        }
        default:
            QStyledItemDelegate::setModelData(editor, model, index);
            break;
    };
}

void MFramePartDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

QWidget *MFramePartDelegate::makeImgSelect(QWidget *parent, const QModelIndex & index) const
{
    const TreeNodeModel *   model = static_cast<const TreeNodeModel*>(index.model());
    QComboBox *             imglstb = new QComboBox(parent);
    FillComboBoxWithSpriteImages(model->getOwnerSprite(), *imglstb);
    return imglstb;
}

QWidget *MFramePartDelegate::makeXOffsetSelect(QWidget *parent, int /*row*/) const
{
    QSpinBox * numxoff = new QSpinBox(parent);
    numxoff->setRange(0, fmt::step_t::XOFFSET_MAX);
    return numxoff;
}

QWidget *MFramePartDelegate::makeYOffsetSelect(QWidget *parent, int /*row*/) const
{
    QSpinBox * numyoff = new QSpinBox(parent);
    numyoff->setRange(0, fmt::step_t::YOFFSET_MAX);
    return numyoff;
}

QWidget *MFramePartDelegate::makePaletteIDSelect(QWidget *parent, const QModelIndex & index) const
{
    const TreeNodeModel * model = static_cast<const TreeNodeModel*>(index.model());
    const Sprite        * pspr = model->getOwnerSprite();
    QComboBox           * pcombopal = new QComboBox(parent);
    const MFramePart    * ppart = static_cast<MFramePart *>(index.internalPointer());
    FillComboBoxWithSpritePalettes(pspr, *pcombopal, ppart->isColorPal256());
    return pcombopal;
}

QWidget *MFramePartDelegate::makePrioritySelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    FillComboBoxWithFramePartPriorities(*pselect);
    return pselect;
}

//QWidget *MFramePartDelegate::makeBlockNumSelect(QWidget *parent, const QModelIndex & index) const
//{
//    const TreeNodeModel * model = static_cast<const TreeNodeModel*>(index.model());
//    const Sprite * pspr = model->getOwnerSprite();
//    QComboBox    *pselect = new QComboBox(parent);
//    if(pspr->type() == fmt::eSpriteType::Character)
//        FillComboBoxWithFrameBlocks(pspr, static_cast<const MFrame*>(index.parent().internalPointer()), *pselect);
//    else
//        FillComboBoxWithSpriteBlocks(pspr, *pselect);
//    return pselect;
//}

QWidget *MFramePartDelegate::makeBlockRangeSelect(QWidget *parent, const QModelIndex & index) const
{
    const TreeNodeModel * model = static_cast<const TreeNodeModel*>(index.model());
    const Sprite * pspr = model->getOwnerSprite();
    QComboBox    *pselect = new QComboBox(parent);
    if(pspr->type() == fmt::eSpriteType::Character)
        FillComboBoxWithFrameBlocks(pspr, static_cast<const MFrame*>(index.parent().internalPointer()), *pselect);
    else
        FillComboBoxWithSpriteBlocks(pspr, *pselect);
    return pselect;
}

QWidget *MFramePartDelegate::makeModeSelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(FRAME_PART_MODE_NAMES);
    pselect->setMinimumWidth(100);
    return pselect;
}

QWidget *MFramePartDelegate::makeRotnScalingParamSelect(QWidget *parent) const
{
    QSpinBox * param = new QSpinBox(parent);
    param->setRange(0, fmt::step_t::NB_RNS_PARAM - 1);
    return param;
}


//void MFramePartDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    QVariant data(index.data());
//    painter->save();
//    switch(static_cast<eFramePartColumnsType>(index.column()))
//    {
//        case eFramePartColumnsType::HFlip:
//        case eFramePartColumnsType::VFlip:
//        {
//            QStyleOptionButton itemoptions;
//            bool state = data.toBool();
//            itemoptions.state = state? QStyle::State_On : QStyle::State_Off;
//            QApplication::style()->drawControl(QStyle::CE_CheckBox, &itemoptions, painter);
//            break;
//        }
//        case eFramePartColumnsType::XOffset:
//        case eFramePartColumnsType::YOffset:
//        case eFramePartColumnsType::TileNum:
//        {
//            QStyleOptionSpinBox itemoptions;

//            QApplication::style()->drawComplexControl(QStyle::QStyle::CC_SpinBox, &itemoptions, painter);
//            break;
//        }
//        default:
//            break;
//    }
//    painter->restore();
//    return QStyledItemDelegate::paint(painter, option, index);
//}
