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

    int16_t getX()const;
    int16_t getY()const;
    void setX(int16_t x);
    void setY(int16_t y);

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;
    Qt::ItemFlags nodeFlags(int column) const override;
private:
    int16_t m_x{0};
    int16_t m_y{0};
};

//Represents a set of offset for displaying sprite effect sprites (exclamation marks, sweat drops, etc..)
class EffectOffsetSet : public TreeNode
{
    typedef TreeNode parent_t;
public:
    //Fixed indices of the offsets for the corresponding part
    enum struct eOffsetsPart : int
    {
        Head = 0,
        RHand,
        LHand,
        Center,

        NbParts,
    };

    EffectOffsetSet(TreeNode * parent);
    EffectOffsetSet(const EffectOffsetSet & cp);
    EffectOffsetSet(EffectOffsetSet && mv);
    EffectOffsetSet operator=(const EffectOffsetSet & cp);
    EffectOffsetSet operator=(EffectOffsetSet && mv);
    ~EffectOffsetSet();

    void ImportOffsetSets(fmt::frameoffsets_t offsets);
    fmt::frameoffsets_t ExportOffsetSets()const;

    fmt::offset_t getHead()const;
    fmt::offset_t getRHand()const;
    fmt::offset_t getLHand()const;
    fmt::offset_t getCenter()const;
    void setHead(fmt::offset_t offs);
    void setRHand(fmt::offset_t offs);
    void setLHand(fmt::offset_t offs);
    void setCenter(fmt::offset_t offs);

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;

    TreeNode *nodeChild(int row) override;
    int nodeChildCount() const override;
    int indexOfChild(const TreeNode *ptr) const override;
    bool _insertChildrenNode(TreeNode *node, int destrow) override;
    bool _insertChildrenNodes(int row, int count) override;
    bool _insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow) override;
    bool _removeChildrenNode(TreeNode *node) override;
    bool _removeChildrenNodes(int row, int count) override;
    bool _removeChildrenNodes(const QList<TreeNode *> &nodes) override;
    bool _deleteChildrenNode(TreeNode *node) override;
    bool _deleteChildrenNodes(int row, int count) override;
    bool _deleteChildrenNodes(const QList<TreeNode *> &nodes) override;
    bool _moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode) override;
    bool _moveChildrenNodes(QModelIndexList &indices, int destrow, QModelIndex destparent) override;
    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent) override;

private:
    QList<EffectOffset*> m_offsets;
};

#endif // EFFECTOFFSETSET_HPP
