#include "animtable.hpp"
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/animtable_delegate.hpp>
#include <src/ppmdu/fmts/sprite/sprite_content.hpp>
#include <src/data/sprite/animgroups_container.hpp>

const QString ElemName_AnimGroupRef = "AnimGroup Ref";
const QString ElemName_AnimTable    = "Animation Table";

//**********************************************************************************
//  AnimGroupRef
//**********************************************************************************

const QMap<AnimTableSlot::eColumns,QString> AnimTableSlot::ColumnNames
{
    {eColumns::GroupId,     QString("Group ID")},
    {eColumns::GroupName,   QString("Slot Name")},
    {eColumns::NbSlots,     QString("Nb Group Slots")},
};

AnimTableSlot::AnimTableSlot(TreeNode * parent)
    :parent_t(parent)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTableSlot::AnimTableSlot(const AnimTableSlot & cp)
    :parent_t(cp)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTableSlot::AnimTableSlot(AnimTableSlot && mv)
    :parent_t(mv)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTableSlot & AnimTableSlot::operator=(const AnimTableSlot & cp)
{
    parent_t::operator=(cp);
    m_grpId = cp.m_grpId;
    return *this;
}

AnimTableSlot & AnimTableSlot::operator=(AnimTableSlot && mv)
{
    parent_t::operator=(mv);
    m_grpId = mv.m_grpId;
    return *this;
}

AnimTableSlot::~AnimTableSlot()
{
}

void AnimTableSlot::setGroupRef(const QModelIndex & ref)
{
    if(ref.isValid())
    {
        const AnimGroup* pgrp = static_cast<const AnimGroup*>(ref.internalPointer());
        Q_ASSERT(pgrp);
        m_grpId = pgrp->getGroupUID();
    }
    else
        m_grpId = fmt::NullGrpIndex;
}

void AnimTableSlot::setGroupRef(fmt::animgrpid_t ref)
{
    m_grpId = ref;
}

fmt::animgrpid_t AnimTableSlot::getGroupRef() const
{
    return m_grpId;
}

void AnimTableSlot::setSlotName(const QString &name)
{
    m_slotName = name;
}

const QString &AnimTableSlot::getSlotName() const
{
    return m_slotName;
}

bool AnimTableSlot::isNull() const
{
    return m_grpId == fmt::NullGrpIndex;
}

TreeNode *AnimTableSlot::clone() const
{
    return new AnimTableSlot(*this);
}

eTreeElemDataType AnimTableSlot::nodeDataTy() const
{
    return eTreeElemDataType::animGroupRef;
}

const QString &AnimTableSlot::nodeDataTypeName() const
{
    return ElemName_AnimGroupRef;
}

QString AnimTableSlot::nodeDisplayName() const
{
    QString dname = QString("Slot %1").arg(nodeIndex());
    if(!isNull())
        return QString("%1: Group %2").arg(dname).arg(m_grpId);
    else
        return QString("%1: None").arg(dname);
}

//**********************************************************************************
//  AnimTable
//**********************************************************************************
AnimTable::AnimTable(TreeNode *parentsprite)
    :TreeNodeWithChilds(parentsprite)
{
    //m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTable::AnimTable(const AnimTable &cp)
    :TreeNodeWithChilds(cp)
{
    //m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTable::AnimTable(AnimTable &&mv)
    :TreeNodeWithChilds(qMove(mv))
{
    //m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimTable &AnimTable::operator=(AnimTable &&mv)
{
    TreeNodeWithChilds::operator=(mv);
    //m_slotNames = qMove(mv.m_slotNames);
    return *this;
}

AnimTable &AnimTable::operator=(const AnimTable &cp)
{
    TreeNodeWithChilds::operator=(cp);
    //m_slotNames = cp.m_slotNames;
    return *this;
}

AnimTable::~AnimTable()
{
}

void AnimTable::importAnimationTable(const fmt::AnimDB::animtbl_t &atbl, const AnimGroups &groups)
{
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, atbl.size());

    for(size_t i = 0; i < atbl.size(); ++i)
    {
        AnimTableSlot * curGrpRef = m_container[i]; //The current reference entry
        fmt::animgrpid_t curgrpid = atbl[i]; //The grp id in the original animation tbl
        if(curgrpid != fmt::NullGrpIndex)
        {
            //Fetch the corresponding group, and make a ref to it
            const AnimGroup * pgrp = static_cast<const AnimGroup*>(groups.nodeChild(curgrpid));
            Q_ASSERT(pgrp);
            curGrpRef->setGroupRef(pgrp->getGroupUID());
        }
        else
            curGrpRef->setGroupRef(fmt::NullGrpIndex);
    }
}

fmt::AnimDB::animtbl_t AnimTable::exportAnimationTable(const AnimGroups & groups) const
{
    fmt::AnimDB::animtbl_t atbl;
    for(const AnimTableSlot * g : *this)
    {
        if(g->isNull())
            atbl.push_back(fmt::NullGrpIndex); //Empty references are treated as null values in the table
        else
        {
            const AnimGroup * pgrp = groups.getGroup(g->getGroupRef());
            Q_ASSERT(pgrp);
            atbl.push_back(pgrp->getGroupUID());
        }
    }
    return atbl;
}

void AnimTable::DeleteGroupRefs(fmt::animgrpid_t id)
{
    Q_ASSERT(false);
    Q_UNUSED(id);
    //Invalidate all references to this group!
//    for(int i = 0; i < m_slotNames.size(); ++i)
//    {
//        if(m_slotNames[i].first == id)
//            m_slotNames[i].first = -1;
//    }
}

void AnimTable::DeleteGroupChild(fmt::animgrpid_t id)
{
    DeleteGroupRefs(id);
    _removeChildrenNodes(id, 1);
    //getModel()->removeRow(id);
}

int AnimTable::getNbNamedSlots() const
{
    return nodeChildCount();
}

QString AnimTable::getSlotName(animtblidx_t entry) const
{
    if(entry >= 0 && entry < nodeChildCount() )
    {
        return m_container[entry]->getSlotName();
    }
    return QString();
}

void AnimTable::setSlotName(animtblidx_t entry, const QString &name)
{
    if(entry >= 0 && entry < nodeChildCount())
        m_container[entry]->setSlotName(name);
}

AnimTable::animtblidx_t AnimTable::findFirstGroupRef(const AnimGroup *grp) const
{
    animtblidx_t found = -1;
    const fmt::animgrpid_t idtofind = grp->getGroupUID();
    for(animtblidx_t i = 0; i < nodeChildCount(); ++i)
    {
        if(m_container[i]->getGroupRef() == idtofind)
        {
            found = i;
            break;
        }
    }
    return found;
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

const QString &AnimTable::ComboBoxStyleSheet()
{
    static const QString SSheet("QComboBox QAbstractItemView::item {margin-top: 2px;}");
    return SSheet;
}

