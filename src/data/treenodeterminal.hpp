#ifndef TREENODETERMINAL_HPP
#define TREENODETERMINAL_HPP
#include <src/data/treenode.hpp>

//**************************************************************************************
//  TreeNodeTerminal
//      Base class for implementing terminal tree nodes elements!
//**************************************************************************************
class TreeNodeTerminal : public TreeNode
{
public:
    TreeNodeTerminal(TreeNode * parent = nullptr)
        :TreeNode(parent)
    {
        m_flags |= Qt::ItemFlag::ItemNeverHasChildren;
    }
    TreeNodeTerminal(const TreeNodeTerminal & cp):TreeNode(cp)
    {
    }
    TreeNodeTerminal(TreeNodeTerminal && mv):TreeNode(mv)
    {
    }
    virtual ~TreeNodeTerminal() {}
    TreeNodeTerminal& operator=(const TreeNodeTerminal & cp){TreeNode::operator=(cp); return *this;}
    TreeNodeTerminal& operator=(TreeNodeTerminal && mv)     {TreeNode::operator=(mv); return *this;}

    //No childs so no need for that
    TreeNode *nodeChild(int) final                  {return nullptr;}
    int nodeChildCount() const final                {return 0;}
    int indexOfChild(const TreeNode* )const final   {return 0;}
    bool nodeShowChildrenOnTreeView()const final    {return false;}

    //No childs so no need for that
protected:
    bool _insertChildrenNode(TreeNode*, int)final                           {return false;}
    bool _insertChildrenNodes(int, int) final                               {return false;}
    bool _insertChildrenNodes(const QList<TreeNode*> &, int)final           {return false;}
    bool _removeChildrenNode(TreeNode*)final                                {return false;}
    bool _removeChildrenNodes(int, int) final                               {return false;}
    bool _removeChildrenNodes(const QList<TreeNode*> &)final                {return false;}
    bool _deleteChildrenNode(TreeNode*)final                                {return false;}
    bool _deleteChildrenNodes(int, int)final                                {return false;}
    bool _deleteChildrenNodes(const QList<TreeNode*> &)final                {return false;}
    bool _moveChildrenNodes(int, int, int, TreeNode*)final                  {return false;}
    bool _moveChildrenNodes(const QModelIndexList&,int,QModelIndex) final   {return false;}
    bool _moveChildrenNodes(const QList<TreeNode *>&,int,QModelIndex) final {return false;}
};


#endif // TREENODETERMINAL_HPP
