#include "animsequence.hpp"
#include <src/data/sprite/animframe.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/sprite.hpp>
#include <QComboBox>
#include <QSpinBox>
#include <QVBoxLayout>

const char * ElemName_AnimSequence  = "Anim Sequence";
//********************************************************************************************
//  AnimSequence
//********************************************************************************************
//const QList<QVariant> AnimSequence::HEADER_COLUMNS
//{
//    QString("Frame"),
//    QString("Duration"),
//    QString("X"),
//    QString("Y"),
//    QString("Shadow X"),
//    QString("Shadow Y"),
//    QString("Flag"),
//};

void AnimSequence::importSeq(const fmt::AnimDB::animseq_t &seq)
{
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, seq.size());

    auto itseq = seq.begin();
    for( fmt::frmid_t cntid = 0; cntid < static_cast<fmt::frmid_t>(seq.size()); ++cntid, ++itseq )
    {
        m_container[cntid].importFrame(*itseq);
    }
}

fmt::AnimDB::animseq_t AnimSequence::exportSeq() const
{
    fmt::AnimDB::animseq_t seq;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seq.push_back(std::move(m_container[cntid].exportFrame()));

    return qMove(seq);
}

Sprite *AnimSequence::parentSprite()
{
    return static_cast<AnimSequences*>(parentNode())->parentSprite();
}

QVariant AnimSequence::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal && section < AnimFrame::ColumnNames.size() )
    {
        return AnimFrame::ColumnNames[section];
    }
    return QVariant();
}

QVariant AnimSequence::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    return static_cast<TreeElement*>(index.internalPointer())->nodeData(index.column(), role);
}

bool AnimSequence::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || role != Qt::EditRole)
        return false;

    AnimFrame *pfrm = static_cast<AnimFrame*>(nodeChild(index.row()));
    Q_ASSERT(pfrm);
    bool bok = false;

    switch(static_cast<AnimFrame::eColumnsType>(index.column()))
    {
    case AnimFrame::eColumnsType::Frame:
        {
            pfrm->setFrmidx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case AnimFrame::eColumnsType::Duration:
        {
            pfrm->setDuration(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }
    case AnimFrame::eColumnsType::Offset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                pfrm->setXoffset(offs.first);
                pfrm->setYoffset(offs.second);
            }
            else
                qDebug("AnimSequence::setData(): Couldn't convert offset value to a QPair!\n");
            break;
        }
    case AnimFrame::eColumnsType::ShadowOffset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                pfrm->setShadowx(offs.first);
                pfrm->setShadowy(offs.second);
            }
            else
                qDebug("AnimSequence::setData(): Couldn't convert shadow offset value to a QPair!\n");
            break;
        }
    case AnimFrame::eColumnsType::Flags:
        {
            pfrm->setFlags(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }

    //direct access columns
    case AnimFrame::eColumnsType::Direct_XOffset:
        {
            pfrm->setXoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case AnimFrame::eColumnsType::Direct_YOffset:
        {
            pfrm->setYoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case AnimFrame::eColumnsType::Direct_ShadowXOffset:
        {
            pfrm->setShadowx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case AnimFrame::eColumnsType::Direct_ShadowYOffset:
        {
            pfrm->setShadowy(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    //Undefined cases
    default:
        return false;
    };

    if(bok && index.model())
    {
        QVector<int> roles{role};
        QAbstractItemModel* pmod = const_cast<QAbstractItemModel*>(index.model());
        if(!pmod)
            qFatal("Model is null!");

        pmod->dataChanged(index, index, roles);
    }
    return bok;
}

QImage AnimSequence::makePreview() const
{
    const_iterator itbeg = begin();
    if( itbeg != end() )
        return qMove(itbeg->makePreview());
    return QImage();
}

//**********************************************************************************
//  AnimSequenceDelegate
//**********************************************************************************
const QString AnimSequenceDelegate::XOffsSpinBoxName = "XOffs";
const QString AnimSequenceDelegate::YOffsSpinBoxName = "YOffs";


QSize AnimSequenceDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
    if(pnode)
        return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
    else
        return QSize();
}

QWidget *AnimSequenceDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *pedit = nullptr;
    switch(static_cast<AnimFrame::eColumnsType>(index.column()))
    {
    case AnimFrame::eColumnsType::Frame:
        {
            pedit = makeFrameSelect(parent);
            break;
        }
    case AnimFrame::eColumnsType::Offset:
        {
            pedit = makeOffsetWidget(parent);
            break;
        }
    case AnimFrame::eColumnsType::ShadowOffset:
        {
            pedit = makeOffsetWidget(parent);
            break;
        }
    case AnimFrame::eColumnsType::Flags:
    case AnimFrame::eColumnsType::Duration:
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
        pedit->setProperty(AnimFrame::UProp_AnimFrameID, index.row());
    }
    return pedit;
}

void AnimSequenceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    AnimFrame * pfrm = static_cast<AnimFrame*>(m_pOwner->getModel()->getItem(index));
    if(!pfrm)
    {
        Q_ASSERT(false);
        qCritical("AnimSequenceDelegate::setEditorData(): Index is inavalid!\n");
        return;
    }

    switch(static_cast<AnimFrame::eColumnsType>(index.column()))
    {
    case AnimFrame::eColumnsType::Frame:
        {
            QComboBox * pcmb = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcmb && pfrm->frmidx() >= 0);
            pcmb->setCurrentIndex(pfrm->frmidx());
            break;
        }
    case AnimFrame::eColumnsType::Offset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(XOffsSpinBoxName);
            QSpinBox *py = editor->findChild<QSpinBox*>(YOffsSpinBoxName);
            Q_ASSERT(px && py);
            px->setValue(pfrm->xoffset());
            py->setValue(pfrm->yoffset());
            break;
        }
    case AnimFrame::eColumnsType::ShadowOffset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(XOffsSpinBoxName);
            QSpinBox *py = editor->findChild<QSpinBox*>(YOffsSpinBoxName);
            Q_ASSERT(px && py);
            px->setValue(pfrm->shadowx());
            py->setValue(pfrm->shadowy());
            break;
        }
    case AnimFrame::eColumnsType::Flags:
    case AnimFrame::eColumnsType::Duration:
    default:
        {
            QStyledItemDelegate::setEditorData(editor,index);
            break;
        }
    };
}

void AnimSequenceDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    AnimFrame * pfrm = static_cast<AnimFrame*>(m_pOwner->getModel()->getItem(index));
    if(!pfrm)
    {
        Q_ASSERT(false);
        qCritical("AnimSequenceDelegate::setModelData(): Index is inavalid!\n");
        return;
    }

    switch(static_cast<AnimFrame::eColumnsType>(index.column()))
    {
    case AnimFrame::eColumnsType::Frame:
        {
            bool        bok  = false;
            QComboBox * pcmb = static_cast<QComboBox*>(editor);
            fmt::frmid_t frmid = static_cast<fmt::frmid_t>(pcmb->currentData().toInt(&bok));
            Q_ASSERT(pcmb && bok);
            model->setData(index, frmid, Qt::EditRole);
            break;
        }
    case AnimFrame::eColumnsType::Offset:
    case AnimFrame::eColumnsType::ShadowOffset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(XOffsSpinBoxName);
            QSpinBox *py = editor->findChild<QSpinBox*>(YOffsSpinBoxName);
            Q_ASSERT(px && py);
            px->interpretText();
            py->interpretText();
            QVariant val;
            val.setValue(QPair<int,int>(px->value(), py->value()));
            model->setData(index, val, Qt::EditRole);
            break;
        }
    case AnimFrame::eColumnsType::Flags:
    case AnimFrame::eColumnsType::Duration:
    default:
        {
            QStyledItemDelegate::setModelData(editor,model,index);
            break;
        }
    };

}

void AnimSequenceDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

const QString &AnimSequenceDelegate::ComboBoxStyleSheet()
{
    static const QString StyleSheet(("QComboBox QAbstractItemView::item {margin-top: 2px;}"));
    return StyleSheet;
}

QWidget *AnimSequenceDelegate::makeFrameSelect(QWidget *parent) const
{
    QComboBox             *imglstb = new QComboBox(parent);
    const FramesContainer &pcnt    = m_pOwner->parentSprite()->getFrames();
    imglstb->setIconSize( QSize(32,32) );
    imglstb->setStyleSheet(ComboBoxStyleSheet());

    //Fill the combobox!
    for( int cntfrm = 0; cntfrm < pcnt.nodeChildCount(); ++cntfrm )
    {
        const MFrame * pfrm = pcnt.getFrame(cntfrm);
        QPixmap pmap = QPixmap::fromImage(pfrm->frameDataCondensed(Qt::DecorationRole).value<QImage>());
        QString text = pfrm->frameDataCondensed(Qt::DisplayRole).toString();
        imglstb->addItem( QIcon(pmap), text, QVariant(pfrm->getFrameUID()) ); //Set user data to frame's UID
    }
    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    return imglstb;
}

QWidget *AnimSequenceDelegate::makeOffsetWidget(QWidget *parent) const
{
    QFrame      *pselect = new QFrame(parent);
    QBoxLayout  *play    = new QBoxLayout(QBoxLayout::Direction::LeftToRight, pselect);
    QSpinBox    *pxoff   = new QSpinBox(pselect);
    QSpinBox    *pyoff   = new QSpinBox(pselect);

    pselect->setMinimumWidth(100);
    pselect->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding);
    pxoff->setObjectName(XOffsSpinBoxName);
    pyoff->setObjectName(YOffsSpinBoxName);
    pxoff->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
    pyoff->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());

    pselect->setLayout(play);
    play->addWidget(pxoff);
    play->addWidget(pyoff);
    pselect->setContentsMargins(1, 1, 1, 1);
    play->setContentsMargins(0,0,0,0);
    pselect->adjustSize();
    parent->adjustSize();
    pselect->setFocusProxy(pxoff);
    return pselect;
}

QWidget *AnimSequenceDelegate::makeShadowOffsetWidget(QWidget *parent) const
{
    return makeOffsetWidget(parent);
}
