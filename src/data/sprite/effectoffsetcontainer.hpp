#ifndef EFFECTOFFSETCONTAINER_HPP
#define EFFECTOFFSETCONTAINER_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/effectoffsetset.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

class Sprite;
extern const QString ElemName_EffectOffsetSets;
//====================================================================
//  EffectOffsetContainer
//====================================================================
class EffectOffsetContainer : public TreeNodeWithChilds<EffectOffsetSet>
{
public:
    using parent_t = TreeNodeWithChilds<EffectOffsetSet>;
    EffectOffsetContainer(TreeNode * parent);
    EffectOffsetContainer(EffectOffsetContainer && mv);
    EffectOffsetContainer(const EffectOffsetContainer & cp);
    EffectOffsetContainer & operator=(EffectOffsetContainer && mv);
    EffectOffsetContainer & operator=(const EffectOffsetContainer & cp);
    ~EffectOffsetContainer();

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    bool nodeIsMutable()const override;
    QString nodeDisplayName() const override;

    //Import/export
    fmt::WA_SpriteHandler::OffsetsDB exportEffects()const;
    void importEffects(const fmt::WA_SpriteHandler::OffsetsDB & efx);
};

#endif // EFFECTOFFSETCONTAINER_HPP
