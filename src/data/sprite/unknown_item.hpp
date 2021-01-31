#ifndef UNKNOWN_ITEM_HPP
#define UNKNOWN_ITEM_HPP
#include <src/data/treenodeterminal.hpp>
#include <QVector>

extern const QString ElemName_UnknownItem;

//Represent files that aren't sprites in some pack files
class UnknownItemNode : public TreeNodeTerminal
{
    using parent_t = TreeNodeTerminal;
public:
    UnknownItemNode(TreeNode * parent, const std::vector<uint8_t> & rawdata);
    UnknownItemNode(TreeNode * parent, std::vector<uint8_t> && rawdata);
    UnknownItemNode(const UnknownItemNode & cp);
    UnknownItemNode(UnknownItemNode && mv);
    ~UnknownItemNode();
    UnknownItemNode & operator=(const UnknownItemNode & cp);
    UnknownItemNode & operator=(UnknownItemNode && mv);

    std::vector<uint8_t> & raw();
    const std::vector<uint8_t> & raw() const;
    void setRaw(const std::vector<uint8_t> &raw);

    // TreeNode interface
public:
    TreeNode *clone() const override;
    bool nodeCanFetchMore() const override;
    bool nodeIsMutable() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;
    QVariant nodeDecoration() const override;
    Qt::ItemFlags nodeFlags(int column) const override;
    bool nodeShouldAutoExpand() const override;
    bool nodeAllowFetchMore() const override;

private:
    std::vector<uint8_t> m_raw;
};

#endif // UNKNOWN_ITEM_HPP
