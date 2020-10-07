#include "effectoffsetcontainer.hpp"
#include <src/data/sprite/sprite.hpp>

const QString ElemName_EffectOffset  = "Effect Offsets";

EffectOffsetContainer::EffectOffsetContainer(TreeNode *parent)
    :TreeNodeTerminal(parent)
{
}

EffectOffsetContainer::~EffectOffsetContainer()
{
    //qDebug("EffectOffsetContainer::~EffectOffsetContainer()\n");
}

TreeNode *EffectOffsetContainer::clone() const
{
    return new EffectOffsetContainer(*this);
}

eTreeElemDataType EffectOffsetContainer::nodeDataTy() const
{
    return eTreeElemDataType::effectOffsets;
}

const QString &EffectOffsetContainer::nodeDataTypeName() const
{
    return ElemName_EffectOffset;
}

//QVariant EffectOffsetContainer::nodeData(int column, int role) const
//{
//    if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(nodeDataTypeName());
//    return QVariant();
//}

std::vector<fmt::effectoffset> EffectOffsetContainer::exportEffects() const
{
    return m_efx;
}

void EffectOffsetContainer::importEffects(const std::vector<fmt::effectoffset> &efx)
{
    m_efx = efx;
}

bool EffectOffsetContainer::nodeIsMutable() const
{
    return false;
}
