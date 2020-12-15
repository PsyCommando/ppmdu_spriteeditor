#ifndef ANIMTABLEMODEL_HPP
#define ANIMTABLEMODEL_HPP
#include <src/data/treenodemodel.hpp>
#include <src/data/sprite/models/animtable_delegate.hpp>

//====================================================================
//  AnimTableModel
//====================================================================
//Model for displaying all the animation groups contained within the animation table of a sprite.
class AnimTableModel : public TreeNodeModel
{
    Q_OBJECT
    using parent_t = TreeNodeModel;
public:
    explicit AnimTableModel(AnimTable* ptable, Sprite * powner);
    ~AnimTableModel();

    //Slots naming
//    void setSlotNames(QVector<QString> && names);
//    QVector<QString>& getSlotNames();
//    const QVector<QString>& getSlotNames()const;

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    const node_t *getRootNode()const override   {return const_cast<AnimTableModel*>(this)->getRootNode();}
    Sprite *getOwnerSprite() override;
    const Sprite *getOwnerSprite()const override{return const_cast<AnimTableModel*>(this)->getOwnerSprite();}

private:
    //QVector<QString>    m_slotNames;            //Strings for naming the animation slots after. For example running animation would be "Run"
    AnimTable*          m_root      {nullptr};
    Sprite*             m_sprite    {nullptr};
};

#endif // ANIMTABLEMODEL_HPP
