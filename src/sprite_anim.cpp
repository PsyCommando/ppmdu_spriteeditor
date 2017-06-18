#include "sprite_anim.hpp"
#include <src/sprite.h>






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

Sprite *AnimSequence::parentSprite()
{
    return static_cast<AnimSequences*>(parentNode())->parentSprite();
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
        const_cast<QAbstractItemModel*>(index.model())->dataChanged(index, index, QVector<int>{role});
    }
    return bok;
}

//********************************************************************************************
//  AnimSequences
//********************************************************************************************
const QList<QVariant> AnimSequences::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Nb frames"),
};

AnimSequences::AnimSequences(TreeElement *parent)
    :BaseTreeContainerChild(parent), m_pmodel(new model_t(this))
{
    setNodeDataTy(eTreeElemDataType::animSequences);
}

AnimSequences::AnimSequences(const AnimSequences &cp)
    :BaseTreeContainerChild(cp), m_pmodel(new model_t(this))
{

}

AnimSequences::AnimSequences(AnimSequences &&mv)
    :BaseTreeContainerChild(mv), m_pmodel(new model_t(this))
{

}

AnimSequences &AnimSequences::operator=(const AnimSequences &cp)
{
    BaseTreeContainerChild::operator=(cp);
    return *this;
}

AnimSequences &AnimSequences::operator=(AnimSequences && mv)
{
    BaseTreeContainerChild::operator=(mv);
    return *this;
}

QVariant AnimSequences::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

QVariant AnimSequences::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
        return qMove(QVariant( QString("%1").arg(section) ));
    else if( orientation == Qt::Orientation::Horizontal && section < HEADER_COLUMNS.size() )
        return HEADER_COLUMNS.at(section);
    return QVariant();
}

AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id)
{
    return static_cast<AnimSequence*>(nodeChild(id));
}

void AnimSequences::removeSequence(fmt::AnimDB::animseqid_t id)
{
    getModel()->removeRow(id);
}

void AnimSequences::importSequences(const fmt::AnimDB::animseqtbl_t &src)
{
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, src.size());

    for( fmt::AnimDB::animseqid_t cntid = 0; cntid < src.size(); ++cntid )
        m_container[cntid].importSeq(src.at(cntid));
}

fmt::AnimDB::animseqtbl_t AnimSequences::exportSequences()
{
    fmt::AnimDB::animseqtbl_t seqs;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seqs[cntid] = std::move(m_container[cntid].exportSeq());

    return qMove(seqs);
}

AnimSequences::model_t *AnimSequences::getModel() {return m_pmodel.data();}

Sprite *AnimSequences::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}


//***********************************************************************************
//  AnimFrame
//***********************************************************************************
const char * AnimFrame::UProp_AnimFrameID = "AnimFrameID";

const QStringList AnimFrame::ColumnNames
{
    "Frame",
    "Duration",
    "Offset",
    "Shadow Offset",
    "Flags",
};

QSize AnimFrame::calcTextSize(const QString &str)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.width(str), fm.height());
}

Sprite *AnimFrame::parentSprite()
{
    return static_cast<AnimSequence*>(parentNode())->parentSprite();
}

QVariant AnimFrame::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    switch(static_cast<eColumnsType>(column))
    {
    case eColumnsType::Frame: //Frame
        {
            if( role == Qt::DisplayRole )
                return QVariant(QString("FrameID: %1").arg( static_cast<int>(frmidx()) ));
            else if(role == Qt::EditRole)
                return static_cast<int>(frmidx());
            else if( role == Qt::DecorationRole)
            {
                const MFrame * pframe = parentSprite()->getFrame(frmidx());
                if(pframe)
                    return QVariant(QPixmap::fromImage(pframe->AssembleFrame(0,0, QRect())));
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( sz.width() + 64 ); //Compensate for thumbnail
                return sz;
            }
            break;
        }
    case eColumnsType::Duration: //duration
        {
            if( role == Qt::DisplayRole )
                return QString("%1t").arg( static_cast<int>(duration()) );
            else if(role == Qt::EditRole)
                return QVariant(static_cast<int>(duration()));
            else if( role == Qt::SizeHintRole )
                return calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
            break;
        }
    case eColumnsType::Offset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(xoffset()).arg(yoffset());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(xoffset(), yoffset()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eColumnsType::ShadowOffset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(shadowx()).arg(shadowy());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(shadowx(), shadowy()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eColumnsType::Flags: //flag
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant( flags() );
            else if( role == Qt::SizeHintRole )
                return calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
            break;
        }

    // -- Direct acces via nodedate! --
    case eColumnsType::Direct_ShadowXOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(shadowx()));
            break;
        }
    case eColumnsType::Direct_ShadowYOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(shadowy()));
            break;
        }
    case eColumnsType::Direct_XOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(xoffset()));
            break;
        }
    case eColumnsType::Direct_YOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(yoffset()));
            break;
        }
    default:
        break;
    };

    return QVariant();
}



//**********************************************************************************
//  AnimTable
//**********************************************************************************
Sprite *AnimTable::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

//**********************************************************************************
//  AnimGroup
//**********************************************************************************
Sprite *AnimGroup::parentSprite()
{
    return static_cast<AnimTable*>(parentNode())->parentSprite();
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
    AnimFrame * pfrm = static_cast<AnimFrame*>(m_pOwner->getModel().getItem(index));
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
    AnimFrame * pfrm = static_cast<AnimFrame*>(m_pOwner->getModel().getItem(index));
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
            QComboBox * pcmb = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcmb && pfrm->frmidx() >= 0);
            pcmb->setCurrentIndex(pfrm->frmidx());
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

QWidget *AnimSequenceDelegate::makeFlagSelect(QWidget *parent) const
{

}
