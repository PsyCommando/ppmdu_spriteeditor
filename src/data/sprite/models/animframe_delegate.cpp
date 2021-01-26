#include "animframe_delegate.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequence.hpp>
#include <src/data/sprite/framescontainer.hpp>
#include <src/data/sprite/models/animframe_model.hpp>
#include <src/data/sprite/models/frames_list_model.hpp>

//**********************************************************************************
//  AnimSequenceDelegate
//**********************************************************************************
const char * AnimFrameDelegate::UProp_AnimFrameID = "AnimFrameID";

AnimFrameDelegate::AnimFrameDelegate(AnimSequence *seq, Sprite * spr, QObject *parent)
    :QStyledItemDelegate(parent), m_animseq(seq)
{
    m_selectModel = new FramesListModel(&spr->getFrames(), spr);
}

AnimFrameDelegate::~AnimFrameDelegate()
{
}

QSize AnimFrameDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if(!index.isValid())
        return QSize();
    const AnimFramesModel* pmod = static_cast<const AnimFramesModel*>(index.model());
    Q_ASSERT(pmod);
    return pmod->data(index, Qt::SizeHintRole).toSize();
}

QWidget *AnimFrameDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *pedit = nullptr;
    const AnimFramesModel * pmod = static_cast<const AnimFramesModel*>(index.model());
    Sprite * owner = const_cast<AnimFramesModel*>(pmod)->getOwnerSprite();

    switch(static_cast<AnimFramesModel::eColumns>(index.column()))
    {
    case AnimFramesModel::eColumns::Frame:
        {
            pedit = makeFrameSelect(parent, owner, m_selectModel.data());
            break;
        }
    default:
        {
            return QStyledItemDelegate::createEditor(parent,option,index);
            break;
        }
    };

    //Common properties
    if(pedit != nullptr)
    {
        pedit->setAutoFillBackground(true);
        pedit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        pedit->setProperty(UProp_AnimFrameID, index.row());
    }
    return pedit;
}

void AnimFrameDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const AnimFramesModel * pmod = static_cast<const AnimFramesModel*>(index.model());
    Q_ASSERT(pmod);
    AnimFrame * pfrm = static_cast<AnimFrame*>(const_cast<AnimFramesModel*>(pmod)->getItem(index));
    if(!pfrm)
    {
        Q_ASSERT(false);
        throw std::runtime_error("AnimSequenceDelegate::setEditorData(): Index is inavalid!\n");
    }

    switch(static_cast<AnimFramesModel::eColumns>(index.column()))
    {
    case AnimFramesModel::eColumns::Frame:
        {
            QComboBox * pcmb = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcmb && pfrm->frmidx() >= 0);
            pcmb->setCurrentIndex(pfrm->frmidx());
            break;
        }
    default:
        {
            QStyledItemDelegate::setEditorData(editor,index);
            break;
        }
    };
}

void AnimFrameDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    const AnimFramesModel * pmod = static_cast<const AnimFramesModel*>(index.model());
    Q_ASSERT(pmod);
    AnimFrame * pfrm = static_cast<AnimFrame*>(const_cast<AnimFramesModel*>(pmod)->getItem(index));
    if(!pfrm)
    {
        Q_ASSERT(false);
        throw std::runtime_error("AnimSequenceDelegate::setModelData(): Index is inavalid!\n");
    }

    switch(static_cast<AnimFramesModel::eColumns>(index.column()))
    {
    case AnimFramesModel::eColumns::Frame:
        {
            bool        bok  = false;
            QComboBox * pcmb = static_cast<QComboBox*>(editor);
            fmt::frmid_t frmid = static_cast<fmt::frmid_t>(pcmb->currentData().toInt(&bok));
            Q_ASSERT(pcmb && bok);
            model->setData(index, frmid, Qt::EditRole);
            break;
        }
    default:
        {
            QStyledItemDelegate::setModelData(editor,model,index);
            break;
        }
    };
    emit SlotChanged(index.row());
}

void AnimFrameDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

const QString &AnimFrameDelegate::ComboBoxStyleSheet()
{
    static const QString StyleSheet(("QComboBox QAbstractItemView::item {margin-top: 2px;}"));
    return StyleSheet;
}

QWidget *AnimFrameDelegate::makeFrameSelect(QWidget *parent, Sprite* spr, TreeNodeModel * pmodel) const
{
    QComboBox             *imglstb = new QComboBox(parent);
    const FramesContainer &pcnt    = spr->getFrames();
    imglstb->setIconSize( QSize(32,32) );
    imglstb->setStyleSheet(ComboBoxStyleSheet());

    //Fill the combobox!
    for( int cntfrm = 0; cntfrm < pcnt.nodeChildCount(); ++cntfrm )
    {
        const MFrame * pfrm = pcnt.getFrame(cntfrm);
        QPixmap pmap = QPixmap::fromImage(pmodel->data(pmodel->indexOfChildNode(pfrm), Qt::DecorationRole).value<QImage>());
        QString text = pmodel->data(pmodel->indexOfChildNode(pfrm), Qt::DisplayRole).toString();
        imglstb->addItem( QIcon(pmap), text, QVariant(pfrm->getFrameUID()) ); //Set user data to frame's UID
    }
    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    return imglstb;
}

//QWidget *AnimFrameDelegate::makeOffsetWidget(QWidget *parent) const
//{
//    QFrame      *pselect = new QFrame(parent);
//    QBoxLayout  *play    = new QBoxLayout(QBoxLayout::Direction::LeftToRight, pselect);
//    QSpinBox    *pxoff   = new QSpinBox(pselect);
//    QSpinBox    *pyoff   = new QSpinBox(pselect);

//    pselect->setMinimumWidth(100);
//    pselect->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding);
//    pxoff->setObjectName(XOffsSpinBoxName);
//    pyoff->setObjectName(YOffsSpinBoxName);
//    pxoff->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
//    pyoff->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());

//    pselect->setLayout(play);
//    play->addWidget(pxoff);
//    play->addWidget(pyoff);
//    pselect->setContentsMargins(1, 1, 1, 1);
//    play->setContentsMargins(0,0,0,0);
//    pselect->adjustSize();
//    parent->adjustSize();
//    pselect->setFocusProxy(pxoff);
//    return pselect;
//}

//QWidget *AnimFrameDelegate::makeShadowOffsetWidget(QWidget *parent) const
//{
//    return makeOffsetWidget(parent);
//}
