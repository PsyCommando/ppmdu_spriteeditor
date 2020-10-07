#include "animgroup.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequences.hpp>

const QString ElemName_AnimGroup = "Anim Group";

//**********************************************************************************
//  AnimGroup
//**********************************************************************************
const QStringList AnimGroup::ColumnNames
{
    "Group ID",
    "Group Name",
    "Nb Slots",
};

AnimGroup::AnimGroup(TreeNode *parent)
    :TreeNodeTerminal(parent)
{
}

AnimGroup::AnimGroup(AnimGroup &&mv)
    :TreeNodeTerminal(mv)
{
    operator=(mv);
}

AnimGroup::AnimGroup(const AnimGroup &cp)
    :TreeNodeTerminal(cp)
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

//QVariant AnimGroup::nodeData(int column, int role) const
//{
//    if( role != Qt::DisplayRole  &&
//        role != Qt::EditRole     &&
//        role != Qt::SizeHintRole &&
//        role != Qt::DecorationRole)
//        return QVariant();

//    if(role == Qt::SizeHintRole)
//    {
//        QFontMetrics fm(QFont("Sergoe UI", 9));
//        QString str(nodeData(column,Qt::DisplayRole).toString());
//        return QSize(fm.horizontalAdvance(str), fm.height());
//    }

//    switch(static_cast<eColumns>(column))
//    {
//    case eColumns::GroupID:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return getGroupUID();
//            break;
//        }
//    case eColumns::GroupName:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//            {
//                const AnimTable * tbl = static_cast<const AnimTable*>(parentNode());
//                Q_ASSERT(tbl);
//                return tbl->data( tbl->getModel()->index(nodeIndex(), column, QModelIndex()), role);
//            }
//            break;
//        }
//    case eColumns::NbSlots:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return m_seqlist.size();
//            break;
//        }
//    default:
//        {
//            break;
//        }
//    };
//    return QVariant();
//}

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
    return dest;
}

bool AnimGroup::operator==(const AnimGroup &other) const
{
    return this == &other;
}

bool AnimGroup::operator!=(const AnimGroup &other) const
{
    return !operator==(other);
}

void AnimGroup::removeSequenceReferences(fmt::AnimDB::animseqid_t id)
{
    for( auto & seq : m_seqlist )
    {
        if(seq == id)
            seq = -1;
    }
}

uint16_t AnimGroup::getUnk16() const
{
    return m_unk16;
}

void AnimGroup::setUnk16(uint16_t val)
{
    m_unk16 = val;
}

int AnimGroup::getGroupUID() const
{
    return nodeIndex();
}

const AnimGroup::slots_t &AnimGroup::seqSlots() const
{
    return m_seqlist;
}

AnimGroup::slots_t &AnimGroup::seqSlots()
{
    return m_seqlist;
}

void AnimGroup::InsertRow(int row, fmt::AnimDB::animseqid_t val)
{
    m_seqlist.insert(row, val);
}

void AnimGroup::RemoveRow(int row)
{
    m_seqlist.removeAt(row);
}

//Sprite *AnimGroup::parentSprite()
//{
//    return static_cast<AnimTable*>(parentNode())->parentSprite();
//}

//AnimGroupDelegate *AnimGroup::getDelegate()
//{
//    return m_delegate.data();
//}

//const AnimGroupDelegate *AnimGroup::getDelegate() const
//{
//    return m_delegate.data();
//}

TreeNode *AnimGroup::clone() const
{
    return new AnimGroup(*this);
}

eTreeElemDataType AnimGroup::nodeDataTy() const
{
    return eTreeElemDataType::animGroup;
}

const QString &AnimGroup::nodeDataTypeName() const
{
    return ElemName_AnimGroup;
}
