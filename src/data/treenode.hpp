#ifndef TREEELEM_HPP
#define TREEELEM_HPP
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <QMutex>
#include <QMutexLocker>
#include <src/data/treeenodetypes.hpp>

class Sprite; //forward declaration

//**************************************************************************************
//  TreeNode
//      Helper base class for displaying the data content of a sprite!
//**************************************************************************************
class TreeNode
{
public:
    TreeNode() = default;
    explicit TreeNode(TreeNode *parentItem)                       : m_parentItem(parentItem){}
    explicit TreeNode(TreeNode *parentItem, Qt::ItemFlags flags)  : m_parentItem(parentItem), m_flags(flags){}
    explicit TreeNode(const TreeNode & cp)                        {operator=(cp);}
    explicit TreeNode(TreeNode && mv)                             {operator=(mv);}

    TreeNode & operator=(const TreeNode & cp)
    {
        m_flags      = cp.m_flags;
        m_parentItem = cp.m_parentItem;
        return *this;
    }

    TreeNode & operator=(TreeNode && mv)
    {
        m_flags      = std::move(mv.m_flags);
        m_parentItem = mv.m_parentItem;
        mv.m_parentItem = nullptr;
        return *this;
    }

    virtual ~TreeNode()
    {
        m_parentItem = nullptr;
    }

    //clone methode for childs to be copied properly
    virtual TreeNode* clone()const = 0;

    //Child nodes handling
    virtual TreeNode*    nodeChild(int row)=0;
    const TreeNode*      nodeChild(int row)const {return const_cast<TreeNode*>(this)->nodeChild(row);}

    virtual bool    nodeHasChildren()const  {return nodeChildCount() > 0;}
    virtual int     nodeChildCount()const = 0;
    virtual int     nodeIndex() const       {return parentNode()? parentNode()->indexOfChild(this): -1;}
    virtual int     indexOfChild(const TreeNode * ptr )const = 0;

    virtual bool nodeCanFetchMore()const {return false;}
    virtual void nodeFetchMore(){}

    //Parent node handling
    virtual TreeNode*       parentNode()                    {return m_parentItem;}
    virtual const TreeNode* parentNode()const               {return const_cast<TreeNode*>(this)->parentNode();}
    virtual void            setParentNode(TreeNode * parent){m_parentItem = parent;}

    //Appending child nodes at the end, or removing a specific child node
    virtual void            nodeAddChild(TreeNode* child)   {_insertChildrenNodes(QList<TreeNode*>{child});}
    virtual void            nodeRemChild(TreeNode* child)   {_removeChildrenNodes(QList<TreeNode*>{child});}

    //Node properties
    virtual bool nodeIsMutable()const {return true;} //Whether we can move, remove, insert this kind of node
    virtual eTreeElemDataType nodeDataTy()const=0; //Node data type id
    virtual const QString&  nodeDataTypeName()const=0; //The name to give elements of this type

    //Name displayed in the main tree view for this node.
    virtual QString nodeDisplayName()const {return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());}

    //Item specific flags
    virtual Qt::ItemFlags     nodeFlags(int column = 0)const    {Q_UNUSED(column); return m_flags;}
    virtual void              setNodeFlags(Qt::ItemFlags val)   {m_flags=val;}

    //Whether the node should be auto-expanded when selected
    virtual bool nodeShouldAutoExpand()const {return false;}

    //Whether the node should display the children of this node in the main tree view
    virtual bool nodeShowChildrenOnTreeView()const {return true;}

    //Whether fetchMore() can be attempted on this node
    virtual bool nodeAllowFetchMore()const {return false;}

    //Helper methods for models
//protected:
    friend class TreeNodeModel;
    virtual bool _insertChildrenNode(TreeNode* node, int destrow) = 0;
    virtual bool _insertChildrenNodes(int row, int count = 1) = 0;
    virtual bool _insertChildrenNodes(const QList<TreeNode*> & nodes, int destrow = -1) = 0; //-1 means insert at the end!

    //Remove children nodes without deleting them
    virtual bool _removeChildrenNode(TreeNode* node) = 0;
    virtual bool _removeChildrenNodes(int row, int count = 1) = 0;
    virtual bool _removeChildrenNodes(const QList<TreeNode*> & nodes) = 0;

    //Remove and delete children nodes
    virtual bool _deleteChildrenNode(TreeNode* node) = 0;
    virtual bool _deleteChildrenNodes(int row, int count = 1) = 0;
    virtual bool _deleteChildrenNodes(const QList<TreeNode*> & nodes) = 0;

    //Move children nodes between postions
    virtual bool _moveChildrenNodes(int row, int count, int destrow, TreeNode* destnode) = 0;
    virtual bool _moveChildrenNodes(QModelIndexList & indices, int destrow, QModelIndex destparent = QModelIndex()) = 0;
    virtual bool _moveChildrenNodes(const QList<TreeNode*> & nodes, int destrow, QModelIndex destparent = QModelIndex()) = 0;

protected:
    TreeNode*       m_parentItem  {nullptr};
    Qt::ItemFlags   m_flags       {Qt::ItemFlag::ItemIsSelectable, Qt::ItemFlag::ItemIsEnabled};
};

#endif // TREEELEM_HPP
