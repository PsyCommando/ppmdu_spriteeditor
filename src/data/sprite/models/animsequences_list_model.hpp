#ifndef ANIMSEQUENCES_LIST_MODEL_HPP
#define ANIMSEQUENCES_LIST_MODEL_HPP
#include <QAbstractItemModel>
#include <src/data/treenodemodel.hpp>
#include <map>

//*******************************************************************
//  AnimSequencesModel
//*******************************************************************
//Model to display all animation sequences and their data
class AnimSequences;
class AnimSequencesListModel : public TreeNodeModel
{
    Q_OBJECT
public:
    enum struct eColumns : int
    {
        Preview = 0,
        NbFrames,
        NbColumns,
    };
    static const std::map<eColumns, QString> ColumnNames;

public:
    explicit AnimSequencesListModel(AnimSequences * pseqs, Sprite * owner);
    virtual ~AnimSequencesListModel(){}

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;
    const TreeNodeModel::node_t *getRootNode() const override
    {
        return const_cast<AnimSequencesListModel*>(this)->getRootNode();
    }

    const Sprite *getOwnerSprite() const override
    {
        return const_cast<AnimSequencesListModel*>(this)->getOwnerSprite();
    }

    // QAbstractItemModel interface
public:
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual QVariant data(const QModelIndex &index, int role)const override;

protected:
    AnimSequences * m_root  {nullptr};
    Sprite *        m_sprite{nullptr};
};

#endif // ANIMSEQUENCES_LIST_MODEL_HPP
