#include "effectoffsetcontainer.hpp"
#include <src/data/sprite/sprite.hpp>

const QString ElemName_EffectOffsetSets  = "Effect Offsets Sets";

EffectOffsetContainer::EffectOffsetContainer(TreeNode *parent)
    :parent_t(parent)
{}

EffectOffsetContainer::EffectOffsetContainer(EffectOffsetContainer && mv)
    :parent_t(mv)
{}

EffectOffsetContainer::EffectOffsetContainer(const EffectOffsetContainer & cp)
    :parent_t(cp)
{}

EffectOffsetContainer & EffectOffsetContainer::operator=(EffectOffsetContainer && mv)
{
    parent_t::operator=(mv);
    return *this;
}

EffectOffsetContainer & EffectOffsetContainer::operator=(const EffectOffsetContainer & cp)
{
    parent_t::operator=(cp);
    return *this;
}

EffectOffsetContainer::~EffectOffsetContainer()
{}

TreeNode *EffectOffsetContainer::clone() const
{
    return new EffectOffsetContainer(*this);
}

eTreeElemDataType EffectOffsetContainer::nodeDataTy() const
{
    return eTreeElemDataType::effectOffsetSets;
}

const QString &EffectOffsetContainer::nodeDataTypeName() const
{
    return ElemName_EffectOffsetSets;
}

QString EffectOffsetContainer::nodeDisplayName() const
{
    return nodeDataTypeName();
}

//QVariant EffectOffsetContainer::nodeData(int column, int role) const
//{
//    if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(nodeDataTypeName());
//    return QVariant();
//}

fmt::WA_SpriteHandler::OffsetsDB EffectOffsetContainer::exportEffects() const
{
    fmt::WA_SpriteHandler::OffsetsDB result;
    for(auto * set : m_container)
        result.push_back(set->ExportOffsetSets());
    return result;
}

void EffectOffsetContainer::importEffects(const fmt::WA_SpriteHandler::OffsetsDB &efx)
{
    _insertChildrenNodes(0, efx.size());
    for(size_t i = 0; i < efx.size(); ++i)
        m_container[i]->ImportOffsetSets(efx[i]);
}

bool EffectOffsetContainer::nodeIsMutable() const
{
    return false;
}
