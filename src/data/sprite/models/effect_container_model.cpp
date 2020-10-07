#include "effect_container_model.hpp"
#include <src/data/sprite/effectoffsetcontainer.hpp>
#include <src/data/sprite/sprite.hpp>

EffectContainerModel::EffectContainerModel(EffectOffsetContainer *poffsets, Sprite *powner)
    :TreeNodeModel()
{
    m_root = poffsets;
    m_sprite = powner;
}

int EffectContainerModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return 1;
}

QVariant EffectContainerModel::data(const QModelIndex &index, int role) const
{
    return TreeNodeModel::data(index, role);
}

bool EffectContainerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return TreeNodeModel::setData(index, value, role);
}

QVariant EffectContainerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return TreeNodeModel::headerData(section, orientation, role);
}

TreeNodeModel::node_t *EffectContainerModel::getRootNode()
{
    return m_root;
}

Sprite *EffectContainerModel::getOwnerSprite()
{
    return m_sprite;
}
