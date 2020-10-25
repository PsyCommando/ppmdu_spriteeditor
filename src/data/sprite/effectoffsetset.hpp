#ifndef EFFECTOFFSETSET_HPP
#define EFFECTOFFSETSET_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>


extern const QString ElemName_EffectOffset;
extern const QString ElemName_EffectOffsetSet;

class EffectOffset : public TreeNodeTerminal
{
    typedef TreeNodeTerminal parent_t;
public:
    EffectOffset(TreeNode * parent);
    EffectOffset(const EffectOffset & cp);
    EffectOffset(EffectOffset && mv);
    EffectOffset operator=(const EffectOffset & cp);
    EffectOffset operator=(EffectOffset && mv);
    ~EffectOffset();

    void setX(int16_t x);
    void setY(int16_t y);
    int16_t getX()const;
    int16_t getY()const;

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

private:
    int16_t m_x {0};
    int16_t m_y {0};
};

//Represents a set of offset for displaying sprite effect sprites (exclamation marks, sweat drops, etc..)
class EffectOffsetSet : public TreeNodeWithChilds<EffectOffset>
{
    typedef TreeNodeWithChilds<EffectOffset> parent_t;
public:
    EffectOffsetSet(TreeNode * parent);
    EffectOffsetSet(const EffectOffsetSet & cp);
    EffectOffsetSet(EffectOffsetSet && mv);
    EffectOffsetSet operator=(const EffectOffsetSet & cp);
    EffectOffsetSet operator=(EffectOffsetSet && mv);
    ~EffectOffsetSet();

    void ImportOffsetSets(std::vector<fmt::effectoffset> offsets);
    std::vector<fmt::effectoffset> ExportOffsetSets()const;

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;
};

#endif // EFFECTOFFSETSET_HPP
