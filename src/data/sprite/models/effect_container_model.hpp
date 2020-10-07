#ifndef EFFECTCONTAINERMODEL_HPP
#define EFFECTCONTAINERMODEL_HPP
#include <src/data/treenodemodel.hpp>

class EffectOffsetContainer;
class EffectContainerModel : public TreeNodeModel
{
    Q_OBJECT
public:
    EffectContainerModel(EffectOffsetContainer* poffsets, Sprite * powner);
    ~EffectContainerModel(){}

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;

private:
    EffectOffsetContainer * m_root  {nullptr};
    Sprite *                m_sprite{nullptr};
};

#endif // EFFECTCONTAINERMODEL_HPP
