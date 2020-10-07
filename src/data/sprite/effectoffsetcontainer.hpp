#ifndef EFFECTOFFSETCONTAINER_HPP
#define EFFECTOFFSETCONTAINER_HPP
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

class Sprite;
extern const QString ElemName_EffectOffset;
//====================================================================
//  EffectOffsetContainer
//====================================================================
class EffectOffsetContainer : public TreeNodeTerminal
{
public:
    EffectOffsetContainer( TreeNode * parent );
    ~EffectOffsetContainer();

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    bool nodeIsMutable()const override;

    //Import/export
    std::vector<fmt::effectoffset> exportEffects()const;
    void importEffects( const std::vector<fmt::effectoffset> & efx );

private:
    std::vector<fmt::effectoffset> m_efx;
};

#endif // EFFECTOFFSETCONTAINER_HPP
