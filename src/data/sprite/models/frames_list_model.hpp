#ifndef FRAMESLISTMODEL_HPP
#define FRAMESLISTMODEL_HPP
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  FramesContainerModel
//*******************************************************************
// Used to display all mframes in the sprite and their data on the mframe tab!
class FramesContainer;
class FramesListModel : public TreeNodeModel
{
    Q_OBJECT
public:
    enum struct eColumns:int
    {
        Preview = 0,
        Id = 0,
        NbColumns,
    };
    static const std::map<eColumns, QString> ColumnNames;
public:
    explicit FramesListModel(FramesContainer * parent, Sprite * parentsprite);
     ~FramesListModel();

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
    const node_t * getRootNode()const override  {return const_cast<FramesListModel*>(this)->getRootNode();}
    const Sprite* getOwnerSprite()const override {return const_cast<FramesListModel*>(this)->getOwnerSprite();}
private:
    FramesContainer*    m_root  {nullptr};
    Sprite*             m_sprite{nullptr};
};
#endif // FRAMESLISTMODEL_HPP
