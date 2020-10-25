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
    using vec_t = std::vector<fmt::effectoffset>;
    vec_t result;
    std::back_insert_iterator<vec_t> itin(result);
    for(auto * set : m_container)
    {
        std::vector<fmt::effectoffset> exported = set->ExportOffsetSets();
        std::copy(exported.begin(), exported.end(), itin);
    }
    return result;
}

void EffectOffsetContainer::importEffects(const fmt::WA_SpriteHandler::OffsetsDB &efx, unsigned int nboffsetSet)
{
    unsigned int nbsets = efx.size()/nboffsetSet;
    _insertChildrenNodes(0, nbsets);
    int cntset = 0;
    std::vector<fmt::effectoffset> accumulated;
    accumulated.reserve(nboffsetSet);
    for(auto off : efx)
    {
        accumulated.push_back(off);
        if(accumulated.size() >= nboffsetSet) //chop the offsets in sets of "nboffsetSet"
        {
            EffectOffsetSet * cnt = m_container[cntset];
            cnt->ImportOffsetSets(std::move(accumulated));
            accumulated = std::vector<fmt::effectoffset>();
            accumulated.reserve(nboffsetSet);
            ++cntset;
        }
    }
}

bool EffectOffsetContainer::nodeIsMutable() const
{
    return false;
}
