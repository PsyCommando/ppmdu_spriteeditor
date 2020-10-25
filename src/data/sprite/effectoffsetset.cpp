#include "effectoffsetset.hpp"

const QString ElemName_EffectOffset = "Effect Offset";
const QString ElemName_EffectOffsetSet = "Effect Offset Set";

//===========================================================
//  EffectOffsetSet
//===========================================================
EffectOffset::EffectOffset(TreeNode * parent)
    :parent_t(parent)
{}

EffectOffset::EffectOffset(const EffectOffset & cp)
    :parent_t(cp)
{}

EffectOffset::EffectOffset(EffectOffset && mv)
    :parent_t(mv)
{}

EffectOffset EffectOffset::operator=(const EffectOffset & cp)
{
    parent_t::operator=(cp);
    return *this;
}

EffectOffset EffectOffset::operator=(EffectOffset && mv)
{
    parent_t::operator=(mv);
    return *this;
}

EffectOffset::~EffectOffset()
{}

void EffectOffset::setX(int16_t x)
{
    m_x = x;
}

void EffectOffset::setY(int16_t y)
{
    m_y = y;
}

int16_t EffectOffset::getX() const
{
    return m_x;
}

int16_t EffectOffset::getY() const
{
    return m_y;
}

TreeNode *EffectOffset::clone() const
{
    return new EffectOffset(*this);
}

eTreeElemDataType EffectOffset::nodeDataTy() const
{
    return eTreeElemDataType::effectOffset;
}

const QString &EffectOffset::nodeDataTypeName() const
{
    return ElemName_EffectOffset;
}

//===========================================================
//  EffectOffsetSet
//===========================================================
EffectOffsetSet::EffectOffsetSet(TreeNode * parent)
    :parent_t(parent)
{}

EffectOffsetSet::EffectOffsetSet(const EffectOffsetSet & cp)
    :parent_t(cp)
{}

EffectOffsetSet::EffectOffsetSet(EffectOffsetSet && mv)
    :parent_t(mv)
{}

EffectOffsetSet EffectOffsetSet::operator=(const EffectOffsetSet & cp)
{
    parent_t::operator=(cp);
    return *this;
}

EffectOffsetSet EffectOffsetSet::operator=(EffectOffsetSet && mv)
{
    parent_t::operator=(mv);
    return *this;
}

EffectOffsetSet::~EffectOffsetSet()
{}

void EffectOffsetSet::ImportOffsetSets(std::vector<fmt::effectoffset> offsets)
{
    _insertChildrenNodes(0, offsets.size());
    for(size_t i = 0; i < offsets.size(); ++i)
    {
        m_container[i]->setX(offsets[i].xoff);
        m_container[i]->setY(offsets[i].yoff);
    }
}

std::vector<fmt::effectoffset> EffectOffsetSet::ExportOffsetSets() const
{
    std::vector<fmt::effectoffset> offsets;
    for(auto * entry : m_container)
    {
        fmt::effectoffset of;
        of.xoff = entry->getX();
        of.yoff = entry->getY();
        offsets.push_back(of);
    }
    return offsets;
}

TreeNode *EffectOffsetSet::clone() const
{
    return new EffectOffsetSet(*this);
}

eTreeElemDataType EffectOffsetSet::nodeDataTy() const
{
    return eTreeElemDataType::effectOffsetSet;
}

const QString &EffectOffsetSet::nodeDataTypeName() const
{
    return ElemName_EffectOffsetSet;
}

QString EffectOffsetSet::nodeDisplayName() const
{
    return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());
}
