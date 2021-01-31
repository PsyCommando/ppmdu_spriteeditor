#include "unknown_item.hpp"

const QString ElemName_UnknownItem = "Unknown Item";

UnknownItemNode::UnknownItemNode(TreeNode * parent, const std::vector<uint8_t> & rawdata)
    :parent_t(parent)
{
    m_raw = rawdata;
}

UnknownItemNode::UnknownItemNode(TreeNode * parent, std::vector<uint8_t> && rawdata)
    :parent_t(parent)
{
    m_raw = rawdata;
}

UnknownItemNode::UnknownItemNode(const UnknownItemNode & cp)
    :parent_t(cp)
{
    operator=(cp);
}

UnknownItemNode::UnknownItemNode(UnknownItemNode && mv)
    :parent_t(mv)
{
    operator=(mv);
}

UnknownItemNode::~UnknownItemNode()
{

}

UnknownItemNode & UnknownItemNode::operator=(const UnknownItemNode & cp)
{
    m_raw = cp.m_raw;
    parent_t::operator=(cp);
    return *this;
}

UnknownItemNode & UnknownItemNode::operator=(UnknownItemNode && mv)
{
    m_raw = std::move(mv.m_raw);
    parent_t::operator=(mv);
    return *this;
}

TreeNode *UnknownItemNode::clone() const
{
    return new UnknownItemNode(*this);
}

bool UnknownItemNode::nodeCanFetchMore() const
{
    return false;
}

bool UnknownItemNode::nodeIsMutable() const
{
    return false;
}

eTreeElemDataType UnknownItemNode::nodeDataTy() const
{
    return eTreeElemDataType::unknown_item;
}

const QString &UnknownItemNode::nodeDataTypeName() const
{
    return ElemName_UnknownItem;
}

QString UnknownItemNode::nodeDisplayName() const
{
    return QString("%1#%2").arg(ElemName_UnknownItem).arg(nodeIndex());
}

QVariant UnknownItemNode::nodeDecoration() const
{
    return QVariant();
}

Qt::ItemFlags UnknownItemNode::nodeFlags(int column) const
{
    return parent_t::nodeFlags(column);
}

bool UnknownItemNode::nodeShouldAutoExpand() const
{
    return false;
}

bool UnknownItemNode::nodeAllowFetchMore() const
{
    return false;
}

std::vector<uint8_t> & UnknownItemNode::raw()
{
    return m_raw;
}


const std::vector<uint8_t> & UnknownItemNode::raw() const
{
    return m_raw;
}

void UnknownItemNode::setRaw(const std::vector<uint8_t> &raw)
{
    m_raw = raw;
}
