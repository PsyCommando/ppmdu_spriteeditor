#ifndef EFFECTCONTAINERMODEL_HPP
#define EFFECTCONTAINERMODEL_HPP
#include <src/data/treenodemodel.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

class EffectOffset;
class EffectOffsetSet;
class EffectSetModel : public TreeNodeModel
{
    Q_OBJECT
    typedef TreeNodeModel parent_t;
public:
    enum struct eColumns : int
    {
        Name,
        XOffset,
        YOffset,
    };
    static const QList<QString> ColumnNames;

    EffectSetModel(EffectOffsetSet* poffsets, Sprite * powner);
    ~EffectSetModel(){}

    const EffectOffset *getHead()const;
    const EffectOffset *getRHand()const;
    const EffectOffset *getLHand()const;
    const EffectOffset *getCenter()const;

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
    const Sprite *getOwnerSprite()const override;

private:
    EffectOffsetSet *   m_root  {nullptr};
    Sprite *            m_sprite{nullptr};
};

#endif // EFFECTCONTAINERMODEL_HPP
