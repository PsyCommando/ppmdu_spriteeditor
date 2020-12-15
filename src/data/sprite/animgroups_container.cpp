#include "animgroups_container.hpp"

const QString ElemName_AnimGroups = "AnimGroups";

AnimGroups::AnimGroups(TreeNode *parent)
    :parent_t(parent)
{
}

AnimGroups::AnimGroups(const AnimGroups &cp)
    :parent_t(cp)
{
}

AnimGroups::AnimGroups(AnimGroups &&mv)
    :parent_t(mv)
{
}

AnimGroups &AnimGroups::operator=(const AnimGroups &cp)
{
    parent_t::operator=(cp);
    return *this;
}

AnimGroups &AnimGroups::operator=(AnimGroups &&mv)
{
    parent_t::operator=(mv);
    return *this;
}

AnimGroups::~AnimGroups()
{
}

AnimGroup *AnimGroups::getGroup(fmt::animgrpid_t id)
{
    return (id >= 0 && id < m_container.size())? m_container[id] : nullptr;
}

const AnimGroup *AnimGroups::getGroup(fmt::animgrpid_t id) const
{
    return const_cast<AnimGroups*>(this)->getGroup(id);
}

void AnimGroups::importAnimationGroups(const fmt::AnimDB::animgrptbl_t &grps)
{
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, grps.size());

    for(size_t i = 0; i < grps.size(); ++i)
    {
        AnimGroup * g = m_container.at(i);
        g->importGroup(grps.at(i));
    }
}

fmt::AnimDB::animgrptbl_t AnimGroups::exportAnimationGroups() const
{
    fmt::AnimDB::animgrptbl_t tbl;
    for(const AnimGroup * g : *this)
        tbl.emplace(g->getGroupUID(), g->exportGroup());
    return tbl;
}

TreeNode *AnimGroups::clone() const
{
    return new AnimGroups(*this);
}

eTreeElemDataType AnimGroups::nodeDataTy() const
{
    return eTreeElemDataType::animGroups;
}

const QString &AnimGroups::nodeDataTypeName() const
{
    return ElemName_AnimGroups;
}

QString AnimGroups::nodeDisplayName() const
{
    return nodeDataTypeName();
}
