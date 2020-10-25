#include "animtable.hpp"
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/animtable_delegate.hpp>

const QString ElemName_AnimTable = "Animation Table";

//**********************************************************************************
//  AnimTable
//**********************************************************************************
AnimTable::~AnimTable()
{
}

//QVariant AnimTable::nodeData(int column, int role) const
//{
//    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(ElemName());
//    return QVariant();
//}

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
    return dest;
}

void AnimTable::importAnimationGroups(fmt::AnimDB::animgrptbl_t &animgrps)
{
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, animgrps.size());
//    m_container.reserve(animgrps.size());
//    getModel()->removeRows(0, nodeChildCount());
//    getModel()->insertRows(0, animgrps.size());
    using grpid_t = fmt::AnimDB::animgrpid_t;
    const grpid_t NBGroups = static_cast<grpid_t>(animgrps.size());

    for(grpid_t cntgrp = 0; cntgrp < NBGroups; ++cntgrp)
        m_container[cntgrp]->importGroup(animgrps[cntgrp]);
}

fmt::AnimDB::animgrptbl_t AnimTable::exportAnimationGroups()
{
    fmt::AnimDB::animgrptbl_t grps;
    for( int cntgrp = 0; cntgrp < nodeChildCount(); ++cntgrp )
    {
        grps[cntgrp] = m_container[cntgrp]->exportGroup();
    }
    return grps;
}

void AnimTable::DeleteGroupRefs(fmt::AnimDB::animgrpid_t id)
{
    Q_UNUSED(id);
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
    _removeChildrenNodes(id, 1);
    //getModel()->removeRow(id);
}

int AnimTable::getNbGroupSequenceSlots() const
{
    const AnimGroup * grp = m_container.first();
    if(grp->nodeHasChildren())
        return grp->nodeChildCount();
    return 0;
}

QString AnimTable::getSlotName(fmt::AnimDB::animgrpid_t entry) const
{
    if(entry >= 0 && entry < m_slotNames.size() )
    {
        auto itf = m_slotNames.find(entry);
        if(itf != m_slotNames.end())
            return (*itf);
    }
    return QString();
}

//QVariant AnimTable::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant("root");

//    //    if (role != Qt::DisplayRole &&
//    //        role != Qt::DecorationRole &&
//    //        role != Qt::SizeHintRole &&
////        role != Qt::EditRole)
////        return QVariant();

////    const AnimGroup *grp = static_cast<const AnimGroup*>(getItem(index));
////    return grp->nodeData(index.column(), role);

//    if( role != Qt::DisplayRole  &&
//        role != Qt::EditRole     &&
//        role != Qt::SizeHintRole &&
//        role != Qt::DecorationRole)
//        return QVariant();

//    const AnimGroup * grp = static_cast<const AnimGroup*>(getItem(index));
//    Q_ASSERT(grp);

////    if( Qt::SizeHintRole )
////    {
////        QFontMetrics fm(QFont("Sergoe UI", 9));
////        QString str = data(index, Qt::DisplayRole).toString();
////        return QSize(fm.width(str), fm.height());
////    }

//    switch(static_cast<AnimGroup::eColumns>(index.column()))
//    {
//    case AnimGroup::eColumns::GroupID:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return grp->getGroupUID();
//            break;
//        }
//    case AnimGroup::eColumns::GroupName:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//            {
//                auto itf = m_slotNames.find(index.row());
//                if(itf != m_slotNames.end())
//                    return (*itf);
//                return QString("--");
//            }
//            break;
//        }
//    case AnimGroup::eColumns::NbSlots:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return grp->seqSlots().size();
//            break;
//        }
//    default:
//        {
//            break;
//        }
//    };
//    return QVariant();
//}

//QVariant AnimTable::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//    {
//        return QVariant(QString("%1").arg(section));
//    }
//    else if( orientation == Qt::Orientation::Horizontal &&
//             (section >= 0) && (section < AnimGroup::ColumnNames.size()) )
//    {
//        return AnimGroup::ColumnNames[section];
//    }
//    return QVariant();
//}

void AnimTable::setSlotName(fmt::AnimDB::animgrpid_t entry, const QString &name)
{
    if(entry >= 0 && entry < m_slotNames.size())
        m_slotNames[entry] = name;
}

TreeNode *AnimTable::clone() const
{
    return new AnimTable(*this);
}

eTreeElemDataType AnimTable::nodeDataTy() const
{
    return eTreeElemDataType::animTable;
}

const QString &AnimTable::nodeDataTypeName() const
{
    return ElemName_AnimTable;
}

QString AnimTable::nodeDisplayName() const
{
    return nodeDataTypeName();
}
