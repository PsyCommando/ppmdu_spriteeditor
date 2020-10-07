#ifndef TREENODEWITHCHILDS_HPP
#define TREENODEWITHCHILDS_HPP
#include <src/data/treenode.hpp>
#include <src/data/treenodemodel.hpp>

//**************************************************************************************
// TreeNodeWithChilds
//**************************************************************************************
//Implementation of TreeNode that have child items
template<class _CHILD_TY>
    class TreeNodeWithChilds : public TreeNode
{
public:
    typedef _CHILD_TY                       child_t;
    typedef QList<child_t*>                 container_t;
    typedef TreeNodeWithChilds<_CHILD_TY>   my_t;

    TreeNodeWithChilds(TreeNode * parent = nullptr) :TreeNode(parent)   {}
    TreeNodeWithChilds(const my_t & cp)             :TreeNode(cp)       {operator=(cp);}
    TreeNodeWithChilds(my_t && mv)                  :TreeNode(mv)       {operator=(mv);}

    virtual ~TreeNodeWithChilds()
    {
        qDeleteAll(m_container);
    }

    my_t & operator=(const my_t & cp)
    {
        //Copy all child items
        for(child_t * p : cp.m_container)
            m_container.push_back(new child_t(*p));
        TreeNode::operator=(cp);
        return *this;
    }

    my_t & operator=(my_t && mv)
    {
        m_container = qMove(mv.m_container);
        TreeNode::operator=(mv);
        return *this;
    }

    // TreeNode interface
public:

    TreeNode* nodeChild(int row) override
    {
        if(row < nodeChildCount() && row >= 0)
            return m_container[row];
        else
            return nullptr;
    }

    int     nodeChildCount()const override  {return m_container.size();}
    bool    nodeHasChildren()const override {return !m_container.empty();}

    int indexOfChild(const TreeNode * ptr)const override
    {
        QMutexLocker lk(&getMutex());
        using child_ptr_t = child_t *;
        child_ptr_t ptras = static_cast<child_ptr_t>(const_cast<TreeNode*>(ptr));
        //Search a matching child in the list!
        if( ptras )
            return m_container.indexOf(ptras);
        return 0;
    }

protected:
    //Universal internal handling for growing and shrinking the child container


    bool _insertChildrenNodes(int row, int count) override
    {
        if(row < -1 || row > m_container.size())
            return false;

        //If row is -1, insert at the end!
        if(row == -1)
            row = m_container.size();

        QMutexLocker lk(&getMutex());
        TreeNode * pthisnode = const_cast<TreeNode*>(static_cast<const TreeNode*>(this));
        for(int i = 0; i < count; ++i)
            m_container.insert(row, new child_t(pthisnode));
        return true;
    }

    bool _insertChildrenNodes(const QList<TreeNode*> & nodes, int row = -1) override
    {
        if(row < -1 || row > m_container.size())
            return false;

        //If row is -1, insert at the end!
        if(row == -1)
            row = m_container.size();

        QMutexLocker lk(&getMutex());
        TreeNode * pthisnode = const_cast<TreeNode*>(static_cast<const TreeNode*>(this));
        for(int i = 0; i < nodes.size(); ++i)
        {
            m_container.insert(row+i, static_cast<child_t*>(nodes[i]));
            m_container.last()->setParentNode(pthisnode);
        }
        return true;
    }

    bool _removeChildrenNodes(int row, int count) override
    {
        if( (row + (count-1)) >= m_container.size() )
            return false;
        QMutexLocker lk(&getMutex());
        for(int i = 0; i < count; ++i)
            m_container.removeAt(row);
        return true;
    }

    bool _removeChildrenNodes(const QList<TreeNode*> & nodes)override
    {
        QMutexLocker lk(&getMutex());
        for(TreeNode* p : nodes)
            m_container.removeOne(static_cast<child_t*>(p));
        return true;
    }

    bool _deleteChildrenNodes(int row, int count)
    {
        if( (row + (count-1)) >= m_container.size() )
            return false;
        QMutexLocker lk(&getMutex());
        for(int i = 0; i < count; ++i)
        {
            delete (m_container[row]);
            m_container.removeAt(row);
        }
        return true;
    }

    bool _deleteChildrenNodes(const QList<TreeNode*> & nodes)
    {
        QMutexLocker lk(&getMutex());
        for(int i = 0; i < m_container.size(); ++i)
        {
            child_t * p = static_cast<child_t*>(nodes[i]);
            m_container.removeOne(p);
            delete p;
        }
        return true;
    }

    bool _moveChildrenNodes(int srcrow, int count, int destrow, TreeNode* desparent)override
    {
        if( srcrow + (count-1) > m_container.size() || destrow > desparent->nodeChildCount() )
        {
            Q_ASSERT(false);
            return false;
        }

        QList<TreeNode*> movebuffer;
        for(int i = 0; i < count; ++i)
            movebuffer.push_back(m_container.at(i));
        return _removeChildrenNodes(srcrow, count) && _insertChildrenNodes(movebuffer, destrow);
    }

    //Get the mutex to ensure we're not modifying the structure as we're accessing it!
    QMutex & getMutex()const{return const_cast<TreeNodeWithChilds*>(this)->m_mutex;}

protected:
    QMutex      m_mutex; //Mutex for modifying the node structure
    container_t m_container;
};

#endif // TREENODEWITHCHILDS_HPP
