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
    return qMove(QImage());
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
    :BaseTreeContainerChild(parent),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{
    setNodeDataTy(eTreeElemDataType::animSequences);
}

AnimSequences::AnimSequences(const AnimSequences &cp)
    :BaseTreeContainerChild(cp),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{

}

AnimSequences::AnimSequences(AnimSequences &&mv)
    :BaseTreeContainerChild(mv),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{

}

AnimSequences::~AnimSequences()
{
    qDebug("AnimSequences::~AnimSequences()\n");
}

void AnimSequences::clone(const TreeElement *other)
{
    const AnimSequences * ptr = static_cast<const AnimSequences*>(other);
    if(!ptr)
        throw std::runtime_error("AnimSequences::clone(): other is not a AnimSequences!");
    (*this) = *ptr;
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

QVariant AnimSequences::data(const QModelIndex &index, int role) const
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

int AnimSequences::columnCount(const QModelIndex &parent)
{
    if (parent.isValid())
        return static_cast<TreeElement*>(parent.internalPointer())->nodeColumnCount();
    else
        return HEADER_COLUMNS.size();
}

AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id)
{
    return static_cast<AnimSequence*>(nodeChild(id));
}

AnimSequencesPickerModel *AnimSequences::getPickerModel()
{
    return m_pickermodel.data();
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

QImage AnimFrame::makePreview() const
{
    const MFrame * pframe = parentSprite()->getFrame(frmidx());
    if(pframe)
        return pframe->AssembleFrame(0,0, QRect());
    return qMove(QImage());
}

//**********************************************************************************
//  AnimTableDelegate
//**********************************************************************************
class AnimTableDelegate : public QStyledItemDelegate
{
public:
    AnimTableDelegate(AnimTable * parent, QObject * pobj = nullptr)
        :QStyledItemDelegate(pobj), m_pOwner(parent)
    {}

    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
        if(pnode)
            return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
        return QStyledItemDelegate::sizeHint(option,index);
    }

    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget * pedit = nullptr;
        Q_ASSERT(false); //TODO
        return pedit;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        Q_ASSERT(false); //TODO
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        if(!index.isValid())
        {
            Q_ASSERT(false);
            qCritical("AnimTableDelegate::setModelData(): Index is inavalid!\n");
            return;
        }

        Sprite          * spr   = m_pOwner->parentSprite();
        const AnimGroup * pfrm  = static_cast<const AnimGroup*>(spr->getAnimTable().nodeChild(index.row()));
        Q_ASSERT(spr && pfrm);

        switch(static_cast<AnimGroup::eColumns>(index.column()))
        {
        case AnimGroup::eColumns::GroupName:
            {
                Q_ASSERT(false); //TODO
                break;
            }
        case AnimGroup::eColumns::GroupID:
            {
                Q_ASSERT(false); //TODO
                break;
            }
        case AnimGroup::eColumns::NbSlots:
        default:
            {
                QStyledItemDelegate::setModelData(editor,model,index);
                break;
            }
        };
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override
    {
        editor->setGeometry(option.rect);
    }

private:
    AnimTable * m_pOwner;
};

//**********************************************************************************
//  AnimTable
//**********************************************************************************
AnimTable::AnimTable(TreeElement *parent)
    :BaseTreeContainerChild(parent),
      m_delegate(new AnimTableDelegate(this))
{
    setNodeDataTy(eTreeElemDataType::animTable);
}

AnimTable::AnimTable(const AnimTable &cp)
    :BaseTreeContainerChild(cp),
      m_delegate(new AnimTableDelegate(this))
{
    operator=(cp);
}

AnimTable::AnimTable(AnimTable &&mv)
    :BaseTreeContainerChild(qMove(mv)),
      m_delegate(new AnimTableDelegate(this))
{
    operator=(qMove(mv));
}

AnimTable &AnimTable::operator=(const AnimTable &cp)
{
    m_slotNames = cp.m_slotNames;
    return *this;
}

AnimTable &AnimTable::operator=(AnimTable &&mv)
{
    m_slotNames = qMove(mv.m_slotNames);
    return *this;
}

void AnimTable::clone(const TreeElement *other)
{
    const AnimTable * ptr = static_cast<const AnimTable*>(other);
    if(!ptr)
        throw std::runtime_error("AnimTable::clone(): other is not a AnimTable!");
    (*this) = *ptr;
}

AnimTable::~AnimTable()
{

}

QVariant AnimTable::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

void AnimTable::importAnimationTable(const fmt::AnimDB::animtbl_t &orig)
{
    //#TODO: See if we can't load strings from a template at the same time for naming slots!
    for(auto id : orig)
        m_slotNames.insert(id,QString());
}

fmt::AnimDB::animtbl_t AnimTable::exportAnimationTable()
{
    //#FIXME:
    //For now just dummy fill it will the ids of all our groups
    //But eventually rewrite this and the import/export code, so we don't care about group ids
    //considering groups are tied to animations, and groups never are null or shared.
    fmt::AnimDB::animtbl_t dest;
    dest.reserve(nodeChildCount());
    for( int i = 0; i < nodeChildCount(); ++i )
    {
        dest.push_back(i);
    }
    return std::move(dest);
}

void AnimTable::importAnimationGroups(fmt::AnimDB::animgrptbl_t &animgrps)
{
    m_container.reserve(animgrps.size());
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, animgrps.size());

    for( fmt::AnimDB::animgrpid_t cntgrp = 0; cntgrp < static_cast<fmt::AnimDB::animgrpid_t>(animgrps.size());
         ++cntgrp )
        m_container[cntgrp].importGroup(animgrps[cntgrp]);
}

fmt::AnimDB::animgrptbl_t AnimTable::exportAnimationGroups()
{
    fmt::AnimDB::animgrptbl_t grps;
    for( int cntgrp = 0; cntgrp < nodeChildCount(); ++cntgrp )
    {
        grps[cntgrp] = std::move(m_container[cntgrp].exportGroup());
    }
    return std::move(grps);
}

void AnimTable::DeleteGroupRefs(fmt::AnimDB::animgrpid_t id)
{
    //Invalidate all references to this group!
//    for(int i = 0; i < m_slotNames.size(); ++i)
//    {
//        if(m_slotNames[i].first == id)
//            m_slotNames[i].first = -1;
//    }
}

void AnimTable::DeleteGroupChild(fmt::AnimDB::animgrpid_t id)
{
    DeleteGroupRefs(id);
    getModel()->removeRow(id);
}

Sprite *AnimTable::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

QVariant AnimTable::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

//    if (role != Qt::DisplayRole &&
//        role != Qt::DecorationRole &&
//        role != Qt::SizeHintRole &&
//        role != Qt::EditRole)
//        return QVariant();

//    const AnimGroup *grp = static_cast<const AnimGroup*>(getItem(index));
//    return grp->nodeData(index.column(), role);

    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole)
        return QVariant();

    const AnimGroup * grp = static_cast<const AnimGroup*>(getItem(index));
    Q_ASSERT(grp);

//    if( Qt::SizeHintRole )
//    {
//        QFontMetrics fm(QFont("Sergoe UI", 9));
//        QString str = data(index, Qt::DisplayRole).toString();
//        return QSize(fm.width(str), fm.height());
//    }

    switch(static_cast<AnimGroup::eColumns>(index.column()))
    {
    case AnimGroup::eColumns::GroupID:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return grp->getGroupUID();
            break;
        }
    case AnimGroup::eColumns::GroupName:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                auto itf = m_slotNames.find(index.row());
                if(itf != m_slotNames.end())
                    return (*itf);
                return QString("--");
            }
            break;
        }
    case AnimGroup::eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return grp->seqSlots().size();
            break;
        }
    default:
        {
            break;
        }
    };
    return QVariant();
}

QVariant AnimTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             (section >= 0) && (section < AnimGroup::ColumnNames.size()) )
    {
        return AnimGroup::ColumnNames[section];
    }
    return QVariant();
}

//*******************************************************************
//  AnimGroupDelegate
//*******************************************************************
class AnimGroupDelegate : public QStyledItemDelegate
{
public:
    AnimGroupDelegate(AnimGroup * owner)
        :QStyledItemDelegate(nullptr), m_pOwner(owner)
    {
    }


    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
        if(pnode)
            return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
        return QStyledItemDelegate::sizeHint(option,index);
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        editor->setGeometry(option.rect);
    }

private:
    AnimGroup * m_pOwner;
};

//**********************************************************************************
//  AnimGroup
//**********************************************************************************
const QStringList AnimGroup::ColumnNames
{
    "Group ID",
    "Group Name",
    "Nb Slots",
};


AnimGroup::AnimGroup(TreeElement *parent)
    :BaseTreeTerminalChild(parent),
      m_delegate(new AnimGroupDelegate(this)),
      m_model(this)
{
    setNodeDataTy(eTreeElemDataType::animGroup);
}

AnimGroup::AnimGroup(AnimGroup &&mv)
    :BaseTreeTerminalChild(qMove(mv)),
      m_delegate(new AnimGroupDelegate(this)),
      m_model(this)
{
    operator=(qMove(mv));
}

AnimGroup::AnimGroup(const AnimGroup &cp)
    :BaseTreeTerminalChild(cp),
      m_delegate(new AnimGroupDelegate(this)),
      m_model(this)
{
    operator=(cp);
}

AnimGroup & AnimGroup::operator=(AnimGroup &&mv)
{
    m_unk16     = mv.m_unk16;
    m_seqlist   = qMove(mv.m_seqlist);
    //We don't touch the delegate
    return *this;
}

AnimGroup & AnimGroup::operator=(const AnimGroup &cp)
{
    m_unk16     = cp.m_unk16;
    m_seqlist   = cp.m_seqlist;
    //We don't touch the delegate
    return *this;
}

AnimGroup::~AnimGroup()
{
    //Need non-default destructor for defining the delegate in the cpp
}

void AnimGroup::clone(const TreeElement *other)
{
    const AnimGroup * ptr = static_cast<const AnimGroup*>(other);
    if(!ptr)
        throw std::runtime_error("AnimGroup::clone(): other is not a AnimGroup!");
    (*this) = *ptr;
}

QVariant AnimGroup::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole)
        return QVariant();

    if(role == Qt::SizeHintRole)
    {
        QFontMetrics fm(QFont("Sergoe UI", 9));
        QString str(nodeData(column,Qt::DisplayRole).toString());
        return QSize(fm.width(str), fm.height());
    }

    switch(static_cast<eColumns>(column))
    {
    case eColumns::GroupID:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return getGroupUID();
            break;
        }
    case eColumns::GroupName:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                const AnimTable * tbl = static_cast<const AnimTable*>(parentNode());
                Q_ASSERT(tbl);
                return tbl->data( tbl->getModel()->index(nodeIndex(), column, QModelIndex()), role);
            }
            break;
        }
    case eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_seqlist.size();
            break;
        }
    default:
        {
            break;
        }
    };
    return QVariant();
}

void AnimGroup::importGroup(const fmt::AnimDB::animgrp_t &grp)
{
    m_seqlist.reserve(grp.seqs.size());
    for( const auto & seq : grp.seqs )
        m_seqlist.push_back(seq);

    m_unk16 = grp.unk16;
}

fmt::AnimDB::animgrp_t AnimGroup::exportGroup()
{
    fmt::AnimDB::animgrp_t dest;
    dest.seqs.resize(m_seqlist.size());
    std::copy(m_seqlist.begin(), m_seqlist.end(), dest.seqs.begin());
    dest.unk16 = m_unk16;
    return std::move(dest);
}

void AnimGroup::removeSequenceReferences(fmt::AnimDB::animseqid_t id)
{
    for( auto & seq : m_seqlist )
    {
        if(seq == id)
            seq = -1;
    }
}

Sprite *AnimGroup::parentSprite()
{
    return static_cast<AnimTable*>(parentNode())->parentSprite();
}

//AnimGroupDelegate *AnimGroup::getDelegate()
//{
//    return m_delegate.data();
//}

//const AnimGroupDelegate *AnimGroup::getDelegate() const
//{
//    return m_delegate.data();
//}

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


//******************************************************************************
//  AnimGroupModel
//******************************************************************************
AnimGroupModel::AnimGroupModel(AnimGroup *pgrp, QObject *parent)
    :QAbstractItemModel(parent), m_pOwner(pgrp)
{
}

QModelIndex AnimGroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if(row < rowCount(parent) && row >= 0 &&
       column < columnCount(parent) && column >= 0)
        return createIndex(row, column, row);
    return QModelIndex();
}

QModelIndex AnimGroupModel::parent(const QModelIndex &) const
{
    //        if(child.internalId() >= 0 && child.internalId() < m_pOwner->nodeColumnCount())
    return QModelIndex();
}

int AnimGroupModel::rowCount(const QModelIndex &/*parent*/) const
{
    Q_ASSERT(m_pOwner);
    return m_pOwner->seqSlots().size();
}

int AnimGroupModel::columnCount(const QModelIndex &parent) const
{
//    if(!parent.isValid())
//        return AnimSequences::HEADER_COLUMNS.size();
    return 1;
}

bool AnimGroupModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

QVariant AnimGroupModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole )
        return QVariant();

    if(index.row() > m_pOwner->seqSlots().size())
    {
        Q_ASSERT(false);
        return QVariant();
    }

    if(index.column() == 0)
    {
        fmt::AnimDB::animseqid_t id = m_pOwner->seqSlots().at(index.row());
        if(role == Qt::DisplayRole)
        {
            AnimSequence *pseq = m_pOwner->parentSprite()->getAnimSequence(id);

            if(id == -1)
                return QString("INVALID");

            if(!pseq)
                return QString("MissingID:%1").arg(id);
            return QString("SequenceID:%1, %2 frames").arg(id).arg(pseq->nodeChildCount());
        }
        else if(role == Qt::DecorationRole)
        {
            AnimSequence * pseq = m_pOwner->parentSprite()->getAnimSequence(id);
            if(!pseq || id == -1)
                return QVariant();
            return QVariant(pseq->makePreview());
        }
        else if(role == Qt::EditRole)
        {
            return id;
        }
        else if(role == Qt::SizeHintRole)
        {
            QFontMetrics fm(QFont("Sergoe UI", 9));
            QString str = data(index, Qt::DisplayRole).toString();
            QSize   szimg = data(index, Qt::DecorationRole).value<QImage>().size();
            QSize   sztxt(fm.width(str), fm.height());

            QSize outsz = sztxt;
            if( szimg.height() > sztxt.height() )
                outsz.setHeight(szimg.height());
            outsz.setWidth( szimg.width() + sztxt.width());
            return outsz;
        }
    }
    return QVariant();
}

QVariant AnimGroupModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole /*&& role != Qt::SizeHintRole*/)
        return QVariant();

    if(orientation == Qt::Orientation::Vertical)
    {
        return section;
    }
    else if(orientation == Qt::Orientation::Horizontal &&
            section >= 0 && section < AnimSequences::HEADER_COLUMNS.size())
    {
        return AnimSequences::HEADER_COLUMNS[section];
    }
    return QVariant();
}

bool AnimGroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return false;

    if(index.row() > m_pOwner->seqSlots().size())
    {
        Q_ASSERT(false);
        return false;
    }

    bool bok = false;
    if(index.column() == 0)
        m_pOwner->seqSlots()[index.row()] = value.toInt(&bok);

    if(bok)
        emit dataChanged(index, index, QVector<int>{role});
    return bok;
}

bool AnimGroupModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid() || row < 0 || row > m_pOwner->seqSlots().size() )
        return false;

    beginInsertRows(parent, row, row + (row + count));
    for( int cntins = 0; cntins < count; ++cntins )
        m_pOwner->InsertRow(row,-1);
    endInsertRows();
    return true;
}

bool AnimGroupModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid() ||
       row < 0 ||
       row > m_pOwner->seqSlots().size() ||
       count > m_pOwner->seqSlots().size() )
        return false;

    beginRemoveRows(parent, row, row + (count - 1));
    for( int cnt = 0; cnt < count; ++cnt )
        m_pOwner->RemoveRow(row);
    endRemoveRows();
    return true;
}

bool AnimGroupModel::moveRows(const QModelIndex &sourceParent,
                              int sourceRow,
                              int count,
                              const QModelIndex &destinationParent,
                              int destinationChild)
{
    if(sourceParent.isValid() ||
       sourceParent != destinationParent ||
       sourceRow < 0 ||
       sourceRow > m_pOwner->seqSlots().size() ||
       destinationChild < 0 ||
       destinationChild > m_pOwner->seqSlots().size() ||
       count < 0 ||
       count > m_pOwner->seqSlots().size())
        return false;

    beginMoveRows(sourceParent, sourceRow, sourceRow + (count-1), destinationParent, destinationChild);
    QVector<fmt::AnimDB::animseqid_t> tomove;
    tomove.reserve(count);

    for( int cnt = 0; cnt < count; ++cnt )
        tomove.push_back(m_pOwner->seqSlots().takeAt(sourceRow));

    //Compute the new destination index after removing the elements to move
    int newdest = (destinationChild < sourceRow)? destinationChild : (destinationChild - count);

    for(int i = 0; i < tomove.size(); ++i)
        m_pOwner->seqSlots().insert(i + newdest, tomove[i]);

    endMoveRows();
    return true;
}

Qt::ItemFlags AnimGroupModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
}

QMap<int, QVariant> AnimGroupModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> res;
    if( index.isValid() )
    {
        res.insert( Qt::DisplayRole,    QVariant(data(index, Qt::DisplayRole)) );
        res.insert( Qt::SizeHintRole,   QVariant(data(index, Qt::SizeHintRole)) );
        res.insert( Qt::EditRole,       QVariant(data(index, Qt::EditRole)) );
        res.insert( Qt::DecorationRole, QVariant(data(index, Qt::DecorationRole)) );
    }
    return qMove(res);
}

bool AnimGroupModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    auto itf = roles.find(Qt::EditRole);
    if( !index.isValid() && itf == roles.end() )
        return false;
    return setData(index, *itf, Qt::EditRole);
}

Qt::DropActions AnimGroupModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions AnimGroupModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

//*******************************************************************
//  AnimSequencesPickerModel
//*******************************************************************
//Model meant to display all animation sequences so they can be dragged to an animation slot in the animation table.
const QStringList AnimSequencesPickerModel::ColumnNames
{
    "Thubmnail",
    "Nb Frames",
};

AnimSequencesPickerModel::AnimSequencesPickerModel(AnimSequences *pseqs, QObject *parent)
    :QAbstractItemModel(parent), m_pOwner(pseqs)
{}

QModelIndex AnimSequencesPickerModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid() && row < m_pOwner->nodeChildCount())
        return createIndex(row, column, m_pOwner->nodeChild(row));
    return QModelIndex();
}

QModelIndex AnimSequencesPickerModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int AnimSequencesPickerModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_pOwner->nodeChildCount();
    else
        return 0;
}

int AnimSequencesPickerModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_pOwner->nodeColumnCount();
    else
        return 0;
}

QVariant AnimSequencesPickerModel::data(const QModelIndex &index, int role) const
{
    //Custom data display for the anim sequence picker!
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole )
        return QVariant();

    switch(static_cast<eColumns>(index.column()))
    {
    case eColumns::Preview:
        {
            if(role == Qt::DecorationRole)
            {
                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QVariant(pseq->makePreview());
            }
            else if(role == Qt::DisplayRole)
            {
                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QString("Sequence#%1 - %2 frames").arg(index.row()).arg(pseq->getSeqLength());
            }
            break;
        }
    case eColumns::NbFrames:
//        {
//            if(role == Qt::DisplayRole)
//            {
//                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
//                Q_ASSERT(pseq);
//                return QString("%1 frames").arg(pseq->nodeChildCount());
//            }
//            break;
//        }
    default: break;
    };
    return QVariant();
}

QVariant AnimSequencesPickerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString("%1").arg(section);
    }
    return QVariant();
}

Qt::ItemFlags AnimSequencesPickerModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}
