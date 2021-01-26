#ifndef ANIMFRAMESMODEL_HPP
#define ANIMFRAMESMODEL_HPP
#include <src/data/sprite/animsequence.hpp>
#include <src/data/treenodemodel.hpp>
#include <map>

//*******************************************************************
//  AnimFramesModel
//*******************************************************************
//Model for accessing the animation frames within an animation sequence
class AnimFramesModel : public TreeNodeModel
{
    Q_OBJECT
public:
    enum struct eColumns : int
    {
        Frame = 0,
        Duration,
        OffsetX,
        OffsetY,
        ShadowX,
        ShadowY,
        Flags,
        //Everything below this is not displayed as header column
        NBColumns [[maybe_unused]],
    };
    static const std::map<eColumns, QString> ColumnNames;

public:
    explicit AnimFramesModel(AnimSequence* pseq, Sprite* pspr);
    ~AnimFramesModel();

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // TreeNodeModel interface
public:
    node_t *getRootNode()override;
    Sprite *getOwnerSprite() override;
    const node_t *getRootNode()const override;
    const Sprite *getOwnerSprite()const override;

private:
    AnimSequence*   m_root      {nullptr};
    Sprite*         m_sprite    {nullptr};
};

#endif // ANIMFRAMESMODEL_HPP
