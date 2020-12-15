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

    const TreeNode* nodeChild(int row)const override
    {
        return const_cast<my_t*>(this)->nodeChild(row);
    }

    int     nodeChildCount()const override  {return m_container.size();}
    bool    nodeHasChildren()const override {return !m_container.empty();}
    virtual bool nodeShowChildrenOnTreeView()const override {return true;}

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


    bool _insertChildrenNode(TreeNode *node, int destrow) override
    {
        if(destrow < 0 || destrow > m_container.size())
            return false;
        QMutexLocker lk(&getMutex());
        TreeNode * pthisnode = const_cast<TreeNode*>(static_cast<const TreeNode*>(this));
        m_container.insert(destrow, static_cast<child_t*>(node));
        m_container.last()->setParentNode(pthisnode);
        return true;
    }

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

    bool _insertChildrenNodes(const QList<TreeNode*> & nodes, int row = 0) override
    {
        if(row < 0 || row > m_container.size())
            return false;
        bool result = true;
        for(int i = 0; i < nodes.size() && (result = _insertChildrenNode(nodes[i], row + i)); ++i);
        return result;
    }

    bool _removeChildrenNode(TreeNode *node) override
    {
        QMutexLocker lk(&getMutex());
        return m_container.removeOne(static_cast<child_t*>(node));
    }

    bool _removeChildrenNodes(int row, int count) override
    {
        if( (row + (count-1)) >= m_container.size() )
            return false;
        QMutexLocker lk(&getMutex());
        for(int i = 0; i < count; ++i)
        {
            m_container.removeAt(row);
        }
        return true;
    }

    bool _removeChildrenNodes(const QList<TreeNode*> & nodes)override
    {
        for(TreeNode* p : nodes)
        {
            if(!_removeChildrenNode(p))
                return false;
        }
        return true;
    }

    bool _deleteChildrenNode(TreeNode *node) override
    {
        QMutexLocker lk(&getMutex());
        child_t * p = dynamic_cast<child_t*>(node);
        Q_ASSERT(p);
        bool succ = m_container.removeOne(p);
        if(succ)
        {
            delete p;
            return true;
        }
        else
            return false;
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
        for(int i = 0; i < m_container.size(); ++i)
        {
            if(!_deleteChildrenNode(nodes[i]))
                return false;
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

    bool _moveChildrenNodes(QModelIndexList &indices, int destrow, QModelIndex destparent) override
    {
        QList<TreeNode *> tomove;
        for(QModelIndex & idx : indices)
            tomove.push_back(static_cast<TreeNode*>(idx.internalPointer()));
        return _moveChildrenNodes(tomove, destrow, destparent);
    }

    bool _moveChildrenNodes(const QList<TreeNode *> &tomove, int destrow, QModelIndex destparent) override
    {
        TreeNode* pdestparent = nullptr;
        if(destparent.isValid())
            pdestparent = static_cast<TreeNode*>(destparent.internalPointer());
        else
            pdestparent = this;
        if(destrow >= pdestparent->nodeChildCount()+1) //Since inserting is done before the specified offset, we gotta add one more child to the count
            return false;

        for(int i = tomove.size()-1; i >= 0; --i)
        {
            int newdest = destrow > tomove[i]->nodeIndex()? destrow - 1 : destrow;
            if( !_removeChildrenNodes(tomove[i]->nodeIndex(), 1) ||
                !_insertChildrenNodes(QList<TreeNode*>{tomove[i]}, newdest) )
            {
                return false;
            }
        }

//        QList<int> predictedDes; //destination rows for each subsequent moves
//        int newdest = destrow;
//        for(TreeNode * t : tomove)
//        {
//            int oldidx = t->nodeIndex();
//            if(newdest > oldidx)
//            {
//                //Destination is after the item we're moving
//                newdest -= 1;
//            }
//            predictedDes.push_back(newdest);
//        }

//        for(int i = 0; i < tomove.size(); ++i)
//        {
//            if( !_removeChildrenNodes(tomove[i]->nodeIndex(), 1) ||
//                !_insertChildrenNodes(QList<TreeNode*>{tomove[i]}, predictedDes[i]) )
//            {
//                return false;
//            }
//        }

//        for(TreeNode * p : tomove)
//        {
//            int newdest = destrow;
//            //Compensate for removal of the source node if we're operating on the same parent!!!
//            //If we're operating on the same parent and placing after the source, we don't need to offset the insertion!
//            if(destparent.internalPointer() != this || (destparent.internalPointer() == this && destrow < p->nodeIndex()))
//                newdest = destrow + cntinsert;

//            if( !_removeChildrenNodes(p->nodeIndex(), 1) ||
//                !_insertChildrenNodes(QList<TreeNode*>{p}, newdest) )
//            {
//                return false;
//            }
//            if(destparent.internalPointer() != this || (destparent.internalPointer() == this && destrow < p->nodeIndex()))
//                ++cntinsert;
//        }
        return true;
    }

    //Get the mutex to ensure we're not modifying the structure as we're accessing it!
    QMutex & getMutex()const{return const_cast<TreeNodeWithChilds*>(this)->m_mutex;}

    //Content Accessor Stuff to allow range based iteration
public:
    typedef typename container_t::iterator       iterator;
    typedef typename container_t::const_iterator const_iterator;

    iterator begin()
    {
        return m_container.begin();
    }

    const_iterator begin() const
    {
        return m_container.begin();
    }

    iterator end()
    {
        return m_container.end();
    }

    const_iterator end() const
    {
        return m_container.end();
    }

    size_t size() const
    {
        return m_container.size();
    }

    bool empty() const
    {
        return m_container.empty();
    }

protected:
    QMutex      m_mutex; //Mutex for modifying the node structure
    container_t m_container;
};

#endif // TREENODEWITHCHILDS_HPP
