#ifndef ANIMFRAMESMODEL_HPP
#define ANIMFRAMESMODEL_HPP
#include <src/data/sprite/animsequence.hpp>
#include <src/data/treenodemodel.hpp>

//NOTE: Be sure to update ColumnNames when changing this!
enum struct eAnimFrameColumnsType : int
{
    Frame = 0,
    Duration,
    Offset,
    ShadowOffset,
    Flags,
    NBColumns [[maybe_unused]],
    //Everything below this is not displayed as header column

    //To acces some of the merged data individually via model data! Since we merged both x/y param entry into a single one for each categories
    Direct_XOffset [[maybe_unused]],
    Direct_YOffset [[maybe_unused]],
    Direct_ShadowXOffset [[maybe_unused]],
    Direct_ShadowYOffset [[maybe_unused]],
};
extern const QStringList AnimFrameColumnNames;

//*******************************************************************
//  AnimFramesModel
//*******************************************************************
//Model for accessing the animation frames within an animation sequence
class AnimFramesModel : public TreeNodeModel
{
    Q_OBJECT
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
    static QSize calcTextSize(const QString & text);
private:
    AnimSequence*   m_root      {nullptr};
    Sprite*         m_sprite    {nullptr};
};

#endif // ANIMFRAMESMODEL_HPP
