#ifndef ANIMGROUPMODEL_HPP
#define ANIMGROUPMODEL_HPP
#include <QAbstractItemModel>
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  AnimGroupModel
//*******************************************************************
//Used to display the animation sequences referred to inside an animation group
class AnimGroup;
class AnimGroupModel : public TreeNodeModel
{
    Q_OBJECT
public:
    explicit AnimGroupModel(AnimGroup * pgrp, Sprite * owner);
    ~AnimGroupModel();

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;
    const TreeNodeModel::node_t *getRootNode() const override
    {
        return const_cast<AnimGroupModel*>(this)->getRootNode();
    }

    const Sprite *getOwnerSprite() const override
    {
        return const_cast<AnimGroupModel*>(this)->getOwnerSprite();
    }

    // QAbstractItemModel interface
public:
    QVariant    data(const QModelIndex &index, int role) const override;
    QVariant    headerData(int section, Qt::Orientation orientation, int role) const override;
    bool        setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    AnimGroup * m_root  {nullptr};
    Sprite *    m_sprite{nullptr};
};

#endif // ANIMGROUPMODEL_HPP
