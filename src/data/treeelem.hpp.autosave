#ifndef TREEELEM_HPP
#define TREEELEM_HPP


#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <QMutex>
#include <QMutexLocker>
#include <src/data/treeelemtypes.hpp>

//enum struct eTreeElemType
//{
//    Editable,
//    Fixed,
//};

class Sprite; //forward declaration



//**************************************************************************************
//  TreeElement
//      Helper base class for displaying the data content of a sprite!
//**************************************************************************************
class TreeElement
{
    friend class BaseTreeNodeModel;
public:
    static constexpr const Qt::ItemFlags DEFFlags()
    {
        return Qt::ItemFlags{Qt::ItemFlag::ItemIsSelectable, Qt::ItemFlag::ItemIsEnabled};
    }

    explicit TreeElement(TreeElement *parentItem = nullptr,
                         Qt::ItemFlags flags      = DEFFlags() )
    {
        m_flags      = flags;
        m_dataty     = eTreeElemDataType::None;
        m_parentItem = parentItem;
    }

    explicit TreeElement(const TreeElement & cp)
    {
        operator=(cp);
    }

    TreeElement & operator=(const TreeElement & cp)
    {
        m_flags      = cp.m_flags;
        m_dataty     = cp.m_dataty;
        m_parentItem = cp.m_parentItem;
        return *this;
    }

    explicit TreeElement(TreeElement && mv)
    {
        operator=(mv);
    }

    TreeElement & operator=(TreeElement && mv)
    {
        m_flags      = std::move(mv.m_flags);
        m_dataty     = std::move(mv.m_dataty);
        m_parentItem = std::move(mv.m_parentItem);
        return *this;
    }

    virtual ~TreeElement()
    {
    }

    virtual TreeElement       * nodeChild(int row)=0;
    virtual const TreeElement * nodeChild(int row)const
    {
        return const_cast<TreeElement*>(this)->nodeChild(row);
    }

    virtual bool            nodeHasChildren()const  {return nodeChildCount() > 0;}
    virtual int             nodeChildCount() const=0;
    virtual int             nodeIndex() const=0;
    virtual int             nodeColumnCount() const=0;
    virtual QVariant        nodeData(int column, int role) const = 0;
//    virtual QVariant        headerData(int section, bool bhorizontal, int role) const = 0;
    virtual bool            insertChildrenNodes(int position, int count)=0;
    virtual bool            removeChildrenNodes(int position, int count)=0;
    virtual bool            removeChildrenNodes(QModelIndexList indices)=0;
    virtual bool            moveChildrenNodes(int srcrow, int count, int destrow)=0;
    virtual int             indexOfNode(const TreeElement * ptr )const = 0;

    //Model stuff
    virtual QModelIndex     modelIndex() const = 0;
    virtual QModelIndex     modelChildIndex(int row, int column) const = 0;
    virtual QModelIndex     modelParentIndex() const = 0;


    virtual bool            canFetchMore(const QModelIndex & parent)const{Q_UNUSED(parent); return false;}
    virtual void            fetchMore(const QModelIndex & parent){Q_UNUSED(parent);}

//    eTreeElemType           getElemTy()const { return m_elemty; }
//    void                    setElemTy( eTreeElemType ty ) { m_elemty = ty; }
    inline eTreeElemDataType       getNodeDataTy()const { return m_dataty; }
    inline void                    setNodeDataTy( eTreeElemDataType ty ) { m_dataty = ty; }

    //virtual Qt::ItemFlags     nodeFlags()const                  {return m_flags;}
    virtual Qt::ItemFlags     nodeFlags(int column = 0)const    {Q_UNUSED(column); return m_flags;}
    virtual void              setNodeFlags(Qt::ItemFlags val)   {m_flags=val;}

    virtual TreeElement       * parentNode()                        {return m_parentItem;}
    virtual const TreeElement * parentNode()const                   {return const_cast<TreeElement*>(this)->parentNode();}
    virtual void                setParentNode(TreeElement * parent) {m_parentItem = parent;}

    virtual Sprite             *parentSprite() = 0;
    virtual const Sprite       *parentSprite()const {return const_cast<TreeElement*>(this)->parentSprite();}

    //clone methode for childs to be copied properly
    virtual void                clone(const TreeElement * other) = 0;

    //Whether we can move, remove, insert this kind of node
    virtual bool nodeIsMutable()const {return true;}

    virtual void OnClicked(){}
    virtual void OnExpanded(){}

    QMutex & getMutex() { return m_mtxhierarchy; }

protected:
    TreeElement        *m_parentItem;
    //eTreeElemType       m_elemty;
    eTreeElemDataType   m_dataty;
    Qt::ItemFlags       m_flags;
    QMutex              m_mtxhierarchy;
};

//**************************************************************************************
//  BaseTreeNodeModelParent
//      Base class used for parent of the BaseTreeNodeModel class!
//**************************************************************************************
class BaseTreeNodeModelParent : public TreeElement
{
public:
    using TreeElement::TreeElement;
    virtual ~BaseTreeNodeModelParent(){}

    virtual int         rowCount    (const QModelIndex &parent)const=0;
    virtual int         columnCount (const QModelIndex &parent)=0;
    virtual QVariant    data        (const QModelIndex &index, int role)const = 0;
    virtual QVariant    headerData  (int section, Qt::Orientation orientation, int role) const = 0;
    virtual bool        setData     (const QModelIndex &/*index*/, const QVariant &/*value*/, int /*role*/ = Qt::EditRole)
    {
        return false;
    }
};

//**************************************************************************************
//  BaseTreeNodeModel
//      A separate "proxy" model class to add to a treenode via
//      composition and use as a model for that node's children,
//      ignoring completely the treenode's parents.
//**************************************************************************************
class BaseTreeNodeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    typedef BaseTreeNodeModelParent modelparent_t;
    typedef TreeElement             mpchild_t;

    BaseTreeNodeModel(modelparent_t * par, QObject * parentobj = nullptr)
        :QAbstractItemModel(parentobj), m_modelparent(par)
    {}

    BaseTreeNodeModel(const BaseTreeNodeModel & cp)
        :QAbstractItemModel(), m_modelparent(cp.m_modelparent)
    {}

    BaseTreeNodeModel(BaseTreeNodeModel && mv)
        :QAbstractItemModel(), m_modelparent(mv.m_modelparent)
    {}

    BaseTreeNodeModel & operator=(const BaseTreeNodeModel && cp)
    {
        m_modelparent = cp.m_modelparent;
        return *this;
    }

    BaseTreeNodeModel & operator=(BaseTreeNodeModel && mv)
    {
        m_modelparent = mv.m_modelparent;
        return *this;
    }

    virtual ~BaseTreeNodeModel()
    {}

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        const TreeElement *item = getItem(index);
        if(item)
            return item->nodeFlags(index.column());
        else
            return Qt::ItemFlag::NoItemFlags;
    }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override
    {
        const TreeElement *parentItem = getItem(parent);
        const TreeElement *childItem  = parentItem->nodeChild(row);
        if (childItem)
            return createIndex(row, column, const_cast<TreeElement*>(childItem));
        else
            return QModelIndex();
    }
    virtual QModelIndex parent(const QModelIndex &child) const override
    {
        if(!child.isValid())
            return QModelIndex();
        const TreeElement *childItem = getItem(child);
        Q_ASSERT(childItem);
        const TreeElement *parentItem = childItem->parentNode();
        Q_ASSERT(parentItem != nullptr);

        if (parentItem == m_modelparent)
            return QModelIndex();

        return createIndex(parentItem->nodeIndex(), 0, const_cast<TreeElement*>(parentItem));
    }

    virtual int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid())
            return static_cast<mpchild_t*>(parent.internalPointer())->nodeColumnCount();
        else
            return m_modelparent->rowCount(parent);
    }

    virtual int columnCount(const QModelIndex &parent) const override
    {
        if (parent.isValid())
            return static_cast<mpchild_t*>(parent.internalPointer())->nodeColumnCount();
        else
            return m_modelparent->columnCount(parent);
    }

    virtual bool hasChildren(const QModelIndex &parent) const override
    {
        if(!parent.isValid())
            return false;
        const TreeElement * parentItem = getItem(parent);
        if(parentItem)
            return parentItem->nodeHasChildren();
        else
            return false;
    }

    virtual QVariant data(const QModelIndex &index, int role) const override
    {
//        if (!index.isValid())
//            return QVariant("root");

//        if (role != Qt::DisplayRole &&
//            role != Qt::DecorationRole &&
//            role != Qt::SizeHintRole &&
//            role != Qt::EditRole)
//            return QVariant();

//        const child_t *img = static_cast<child_t*>(getItem(index));
//        return img->imgData(index.column(), role);
        return m_modelparent->data(index, role);
    }

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)override
    {
        return m_modelparent->setData(index, value, role);
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
//        if( role != Qt::DisplayRole )
//            return QVariant();

//        if( orientation == Qt::Orientation::Vertical )
//        {
//            return std::move(QVariant( QString("%1").arg(section) ));
//        }
//        else if( orientation == Qt::Orientation::Horizontal )
//        {
//            switch(section)
//            {
//            case 0:
//                return std::move(QVariant( QString("Preview") ));
//            case 1:
//                return std::move(QVariant( QString("Bit Depth") ));
//            case 2:
//                return std::move(QVariant( QString("Resolution") ));
//            };
//        }
//        return QVariant();
        return m_modelparent->headerData(section, orientation, role);
    }

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
    {
        TreeElement *parentItem = getItem(parent);
        bool success = false;

        if( count > 0 )
        {
            beginInsertRows(parent, row, row + count - 1);
            success = parentItem->insertChildrenNodes(row, count);
            endInsertRows();
        }

        return success;
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
    {
        TreeElement *parentItem = getItem(parent);
        bool success = true;

        if( count > 0 )
        {
            beginRemoveRows(parent, row, row + count - 1);
            success = parentItem->removeChildrenNodes(row, count);
            endRemoveRows();
        }
        return success;
    }

    virtual bool moveRows(const QModelIndex &sourceParent,
                          int sourceRow,
                          int count,
                          const QModelIndex &destinationParent,
                          int destinationChild) override
    {
        TreeElement *srcparentItem = getItem(sourceParent);
        //TreeElement *destparentItem = getItem(destinationParent);

        if( destinationParent == sourceParent && sourceRow == destinationChild )
            return true;

        if(destinationParent == sourceParent)
        {
            int newdest = (sourceRow < destinationChild)? destinationChild + 1 : destinationChild;
            if(beginMoveRows(sourceParent, sourceRow, sourceRow + (count - 1), destinationParent, newdest))
            {
                bool result = srcparentItem->moveChildrenNodes(sourceRow, count, destinationChild);
                endMoveRows();
                return result;
            }
            else
                return false;
        }
        else
        {
            Q_ASSERT(false); //Not implemented
            return false;
        }
        return true;
    }

    virtual TreeElement *getItem(const QModelIndex &index)
    {
        if (index.isValid())
        {
            TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
            if (item)
                return item;
        }
        return m_modelparent;
    }

    virtual const TreeElement *getItem(const QModelIndex &index)const
    {
        return const_cast<BaseTreeNodeModel*>(this)->getItem(index);
    }

    //Convenience function for emiting the datachanged signal
    inline void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ())
    {
        emit dataChanged(topLeft, bottomRight, roles);
    }

protected:
    modelparent_t * m_modelparent;
};

//**************************************************************************************
//  BaseTreeTerminalChild
//      Base class for implementing terminal tree nodes elements!
//**************************************************************************************
template<const char** _STRELEMNAME>
    class BaseTreeTerminalChild : public TreeElement
{
protected:
    constexpr QString ElemName()const
    {
        return QString(*_STRELEMNAME);
    }

public:
    BaseTreeTerminalChild(TreeElement * parent, Qt::ItemFlags flags = DEFFlags())
        :TreeElement(parent, flags |= Qt::ItemFlag::ItemNeverHasChildren)
    {}
    virtual ~BaseTreeTerminalChild() {}


    //Can't be reinplemented:
    TreeElement *nodeChild(int) final            {return nullptr;}
    int nodeChildCount() const final             {return 0;}
    int indexOfNode(const TreeElement* )const final   {return 0;}
    int nodeIndex() const final
    {
        if (m_parentItem)
            return m_parentItem->indexOfNode(this);
        return 0;
    }
    //You don't!!
    bool insertChildrenNodes(int, int) final {return false;}
    bool removeChildrenNodes(int, int) final {return false;}
    bool removeChildrenNodes(QModelIndexList ) final {return false;}
    bool moveChildrenNodes(int, int, int)final {return false;}


    //Thos can be re-implemented!
    virtual int nodeColumnCount() const override  {return 1;}
    virtual QVariant nodeData(int column, int role) const override
    {
        if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
            return QVariant();

        if( column != 0 )
            return QVariant();
        QString sprname = QString("%1#%2").arg(ElemName()).arg(nodeIndex());
        return QVariant(sprname);
    }

    QModelIndex modelIndex() const final
    {
        return QModelIndex();
    }

    QModelIndex modelChildIndex(int row, int column) const final
    {
        Q_UNUSED(row);
        Q_UNUSED(column);
        return QModelIndex();
    }

    virtual QModelIndex modelParentIndex() const override
    {
        if (m_parentItem)
            return m_parentItem->modelIndex();
        return QModelIndex();
    }
};



//**************************************************************************************
// BaseTreeContainerChild
//      Base class for implementing tree nodes with childrens!
//**************************************************************************************
template<const char** _STRELEMNAME, class _CHILD_TY, class _MODEL_TY = BaseTreeNodeModel>
    class BaseTreeContainerChild : public BaseTreeNodeModelParent
{
protected:
    constexpr QString ElemName()const
    {
        return QString(*_STRELEMNAME);
    }
    typedef _CHILD_TY       child_t;
    typedef QList<child_t>  container_t;
    typedef _MODEL_TY       model_t;

    typedef BaseTreeContainerChild<_STRELEMNAME, _CHILD_TY> my_t;

public:
    BaseTreeContainerChild(TreeElement * parent, Qt::ItemFlags flags = DEFFlags())
        :BaseTreeNodeModelParent(parent, flags), m_pmodel(new model_t(this))
    {}

    BaseTreeContainerChild(const my_t & cp)
        :BaseTreeNodeModelParent(cp),m_container(cp.m_container), m_pmodel(new model_t(this))
    {}

    BaseTreeContainerChild(my_t && mv)
        : BaseTreeNodeModelParent(mv), m_container(qMove(mv.m_container)), m_pmodel(new model_t(this))
    {}

    my_t & operator=(const my_t & cp)
    {
        m_pmodel.reset(new model_t(this));
        m_container = cp.m_container;
        return *this;
    }

    my_t & operator=(my_t && mv)
    {
        m_pmodel.reset(new model_t(this));
        m_container = qMove(mv.m_container);
        return *this;
    }

    virtual ~BaseTreeContainerChild() {}


    TreeElement *nodeChild(int row) override
    {
        if(row < nodeChildCount() && row >= 0)
            return &m_container[row];
        else
            return nullptr;
    }

    virtual int rowCount(const QModelIndex &parent)const override
    {
        TreeElement * p = const_cast<my_t*>(this)->getItem(parent);
        if(p)
            return p->nodeChildCount();
        else
            return 0;
    }


    virtual int nodeChildCount() const override         {return m_container.size();}

    inline bool hasChildren()const {return nodeChildCount() > 0;}

    int nodeIndex() const override
    {
        //QMutexLocker lk(&const_cast<typename std::add_pointer<my_t>::type>(this)->getMutex());
        if (m_parentItem)
            return m_parentItem->indexOfNode(this);

        return 0;
    }

    int indexOfNode(const TreeElement * ptr)const override
    {
        QMutexLocker lk(&const_cast<typename std::add_pointer<my_t>::type>(this)->getMutex());
        using const_ptr_t = const child_t *;
        const_ptr_t ptras = static_cast<const_ptr_t>(ptr);
        //Search a matching child in the list!
        if( ptras )
            return m_container.indexOf(*ptras);
        return 0;
    }

    QModelIndex modelIndex() const override
    {
        //QMutexLocker lk(&const_cast<typename std::add_pointer<my_t>::type>(this)->getMutex());
        if(m_pmodel)
        {
            const TreeElement * parentnode = parentNode();
            if(!parentnode)
                return QModelIndex(); //No parents, we're the parent, so invalid index
            QModelIndex mparent = parentnode->modelIndex();
            const BaseTreeNodeModel * pmod = nullptr;
            if(!mparent.isValid())
                pmod = m_pmodel.data(); //When we get an invalid parent, it means we're at the top!
            else
                pmod = static_cast<const BaseTreeNodeModel*>(mparent.model());

            if(m_parentItem == this)
                return pmod->index(nodeIndex(), 0, QModelIndex());
            else
                return pmod->index(nodeIndex(), 0, m_parentItem->modelIndex());
        }
        return QModelIndex();
    }

    virtual QModelIndex modelChildIndex(int row, int column) const override
    {
        //QMutexLocker lk(&const_cast<typename std::add_pointer<my_t>::type>(this)->getMutex());
        if(m_pmodel)
            return m_pmodel->index(row, column, modelIndex());
        return QModelIndex();
    }

    QModelIndex modelParentIndex() const override
    {
        //QMutexLocker lk(&const_cast<typename std::add_pointer<my_t>::type>(this)->getMutex());
        if (m_parentItem)
            return m_parentItem->modelIndex();
        return QModelIndex();
    }

    //Thos can be re-implemented!
    virtual int nodeColumnCount() const                 {return 1;}


    virtual QVariant nodeData(int column, int role) const override
    {
        if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
            return QVariant();

        if( column != 0 )
            return QVariant();
        QString sprname = QString("%1#%2").arg(ElemName()).arg(nodeIndex());
        return QVariant(sprname);
    }

//        virtual QVariant headerData(int section, bool bhorizontal, int role) const override
//        {
////            if( role != Qt::DisplayRole &&
////                role != Qt::DecorationRole &&
////                role != Qt::SizeHintRole &&
////                role != Qt::EditRole )
////                return QVariant();

////            if( column != 0 )
////                return QVariant();

//            return QVariant();
//        }


    bool insertChildrenNodes(int position, int count) override
    {
        QMutexLocker lk(&getMutex());
        for(int i = 0; i < count; ++i)
            m_container.insert(position, child_t(this) );
        return true;
    }

    bool removeChildrenNodes(int position, int count) override
    {
        QMutexLocker lk(&getMutex());
        if( (position + (count-1)) >= m_container.size() )
            return false;
        for(int i = 0; i < count; ++i)
            m_container.removeAt(position);
        return true;
    }

    bool removeChildrenNodes(QModelIndexList indices)override
    {
        bool success = true;
        for(QModelIndex idx : indices)
        {
            if(!idx.isValid())
                continue;
            TreeElement * node = static_cast<TreeElement*>(idx.internalPointer());
            Q_ASSERT(node);
            success = success && removeChildrenNodes(node->nodeIndex(), 1);
        }
        return success;
    }

    bool moveChildrenNodes(int srcrow, int count, int destrow)override
    {
        QMutexLocker lk(&getMutex());

        if( srcrow + (count-1) > m_container.size() || destrow > m_container.size() )
        {
            Q_ASSERT(false);
            return false;
        }

        if(destrow > srcrow)
        {
            for( int i = 0; i < count; ++i )
                m_container.move(srcrow, destrow);
        }
        else
        {
            for( int i = 0; i < count; ++i )
                m_container.move(srcrow, destrow + i);
        }

        return true;
    }

    virtual int columnCount(const QModelIndex &parent) override
    {
        if (parent.isValid())
            return static_cast<TreeElement*>(parent.internalPointer())->nodeColumnCount();
        else
            return nodeColumnCount();
    }

    //Implement those!!
//        virtual QVariant data(const QModelIndex &index, int role)const override
//        virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override

    inline model_t * getModel() {return m_pmodel.data();}
    inline const model_t * getModel()const {return m_pmodel.data();}

    virtual TreeElement *getItem(const QModelIndex &index)
    {
        if (index.isValid())
        {
            TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
            if (item)
                return item;
        }
        return this;
    }

    virtual const TreeElement *getItem(const QModelIndex &index)const
    {
        return const_cast<my_t*>(this)->getItem(index);
    }

protected:
    container_t                 m_container;
    QScopedPointer<model_t>     m_pmodel;
};

#endif // TREEELEM_HPP
