#ifndef TREEELEM_HPP
#define TREEELEM_HPP


#include <QVariant>
#include <QList>
#include <QAbstractItemModel>


enum struct eTreeElemType
{
    Editable,
    Fixed,
};

enum struct eTreeElemDataType
{
    None,
    sprite,
    effectOffsets,
    palette,
    images,
    image,
    frames,
    frame,
    animSequences,
    animSequence,
    animTable,
    animGroup,
    animFrame,
};

/*
 * TreeElement
 * Helper base class for displaying the data content of a sprite!
*/
class TreeElement
{
public:
    explicit TreeElement(TreeElement *parentItem = 0)
    {
        m_elemty     = eTreeElemType::Fixed;
        m_dataty     = eTreeElemDataType::None;
        m_parentItem = parentItem;
    }

    explicit TreeElement(const TreeElement & cp)
    {
        operator=(cp);
    }

    TreeElement & operator=(const TreeElement & cp)
    {
        m_elemty     = cp.m_elemty;
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
        m_elemty     = std::move(mv.m_elemty);
        m_dataty     = std::move(mv.m_dataty);
        m_parentItem = std::move(mv.m_parentItem);
        return *this;
    }

    virtual ~TreeElement()
    {
    }

    virtual TreeElement *   child(int row)=0;
    virtual const TreeElement *   child(int row)const { return const_cast<TreeElement*>(this)->child(row); }
    virtual int             childCount() const=0;
    virtual int             childNumber() const=0;
    virtual int             columnCount() const=0;
    virtual QVariant        data(int column, int role) const = 0;
//    virtual QVariant        headerData(int section, bool bhorizontal, int role) const = 0;
    virtual bool            insertChildren(int position, int count)=0;
    virtual bool            removeChildren(int position, int count)=0;
    virtual int             indexOf( TreeElement * ptr )const = 0;

    eTreeElemType           getElemTy()const { return m_elemty; }
    void                    setElemTy( eTreeElemType ty ) { m_elemty = ty; }
    eTreeElemDataType       getDataTy()const { return m_dataty; }
    void                    setDataTy( eTreeElemDataType ty ) { m_dataty = ty; }

    virtual TreeElement       * parent()                        {return m_parentItem;}
    virtual const TreeElement * parent()const                   {return m_parentItem;}
    virtual void                setParent(TreeElement * parent) {m_parentItem = parent;}

    virtual void OnClicked(){}
    virtual void OnExpanded(){}

    TreeElement         *m_parentItem;
    eTreeElemType        m_elemty;
    eTreeElemDataType    m_dataty;
};


/*******************************************************************
 * BaseTreeTerminalChild
 *          Base class for implementing terminal tree nodes elements!
*******************************************************************/
template<const char** _STRELEMNAME>
    class BaseTreeTerminalChild : public TreeElement
{
protected:
    constexpr QString ElemName()const
    {
        return QString(*_STRELEMNAME);
    }

public:
    BaseTreeTerminalChild(TreeElement * parent)
        :TreeElement(parent)
    {}

    virtual ~BaseTreeTerminalChild() {}


    TreeElement *child(int) override    {return nullptr;}
    int childCount() const override         {return 0;}
    int childNumber() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOf(const_cast<BaseTreeTerminalChild*>(this));

        return 0;
    }

    int indexOf( TreeElement* )const override  {return 0;}

    //Thos can be re-implemented!
    virtual int columnCount() const                 {return 1;}

    virtual QVariant data(int column, int role) const override
    {
        if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
            return QVariant();

        if( column != 0 )
            return QVariant();
        QString sprname = QString("%1#%2").arg(ElemName()).arg(childNumber());
        return QVariant(sprname);
    }

//    virtual QVariant headerData(int section, bool bhorizontal, int role) const override
//    {
////        if( role != Qt::DisplayRole &&
////            role != Qt::DecorationRole &&
////            role != Qt::SizeHintRole &&
////            role != Qt::EditRole )
////            return QVariant();

////        if( section != 0 )
////            return QVariant();

//        return QVariant();
//    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

};


/*******************************************************************
 * BaseListContainerChild
 *      Base class for implementing tree nodes with childrens!
*******************************************************************/
    template<const char** _STRELEMNAME, class _CHILD_TY>
        class BaseListContainerChild : public TreeElement
    {
    protected:
        constexpr QString ElemName()const
        {
            return QString(*_STRELEMNAME);
        }
        typedef _CHILD_TY               child_t;
        typedef QList<child_t> container_t;

        typedef BaseListContainerChild<_STRELEMNAME, _CHILD_TY> my_t;

    public:
        BaseListContainerChild(TreeElement * parent)
            :TreeElement(parent)
        {}

        BaseListContainerChild(const my_t & cp)
            :TreeElement(cp),m_container(cp.m_container)
        {}

        BaseListContainerChild(my_t && mv)
            :m_container(qMove(mv.m_container)), TreeElement(mv)
        {}

        my_t & operator=(const my_t & cp)
        {
            m_container = cp.m_container;
            return *this;
        }

        my_t & operator=(my_t && mv)
        {
            m_container = qMove(mv.m_container);
            return *this;
        }

        virtual ~BaseListContainerChild() {}


        TreeElement *child(int row) override    {return &m_container[row];}
        int childCount() const override         {return m_container.size();}
        int childNumber() const override
        {
            if (m_parentItem)
                return m_parentItem->indexOf(const_cast<BaseListContainerChild*>(this));

            return 0;
        }

        int indexOf( TreeElement * ptr )const override
        {
            child_t * ptras = static_cast<child_t *>(ptr);
            //Search a matching child in the list!
            if( ptras )
                return m_container.indexOf(*ptras);
            return 0;
        }

        //Thos can be re-implemented!
        virtual int columnCount() const                 {return 1;}

        virtual QVariant data(int column, int role) const override
        {
            if( role != Qt::DisplayRole &&
                role != Qt::DecorationRole &&
                role != Qt::SizeHintRole &&
                role != Qt::EditRole )
                return QVariant();

            if( column != 0 )
                return QVariant();
            QString sprname = QString("%1#%2").arg(ElemName()).arg(childNumber());
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


        bool insertChildren(int position, int count) override
        {
            int i = 0;
            for( ; i < count; ++i )
                m_container.insert(position, child_t(this) );
            return true;
        }

        bool removeChildren(int position, int count) override
        {
            if( (position + count) >= m_container.size() )
                return false;

            int i = 0;
            for( ; i < count; ++i )
                m_container.removeAt(position);
            return true;
        }

    protected:
        container_t m_container;

    };

/*******************************************************************
 * BaseTreeNodeModel
 *      Base class for adding a model to a child treenode so the
 *      child of that tree node can be used as a separate model!
*******************************************************************/
template<class _TREE_NODE_TY, class _CHILD_NODE_TY>
    class BaseTreeNodeModel : public QAbstractItemModel
{
public:
    typedef _TREE_NODE_TY                                   modelparent_t;
    typedef _CHILD_NODE_TY                                  mpchild_t;
    typedef BaseTreeNodeModel<_TREE_NODE_TY,_CHILD_NODE_TY> my_t;

    BaseTreeNodeModel(modelparent_t * par)
        :QAbstractItemModel(), m_modelparent(par)
    {}

    BaseTreeNodeModel(const my_t & cp)
        :QAbstractItemModel(), m_modelparent(cp.m_modelparent)
    {}

    BaseTreeNodeModel(my_t && mv)
        :QAbstractItemModel(), m_modelparent(mv.m_modelparent)
    {}

    my_t & operator=(const my_t && cp)
    {
        m_modelparent = cp.m_modelparent;
        return *this;
    }

    my_t & operator=(my_t && mv)
    {
        m_modelparent = mv.m_modelparent;
        return *this;
    }

    virtual ~BaseTreeNodeModel()
    {}

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override
    {
        const TreeElement *parentItem = getItem(parent);
        const TreeElement *childItem  = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, const_cast<TreeElement*>(childItem));
        else
            return QModelIndex();
    }
    virtual QModelIndex parent(const QModelIndex &child) const override
    {
        const TreeElement *childItem = getItem(child);
        const TreeElement *parentItem = childItem->parent();
        Q_ASSERT(parentItem != nullptr);

        if (parentItem == m_modelparent)
            return QModelIndex();

        return createIndex(parentItem->childNumber(), 0, const_cast<TreeElement*>(parentItem));
    }

    int rowCount(const QModelIndex &parent) const override
    {
        const TreeElement *parentItem = getItem(parent);
        return parentItem->childCount();
    }

    virtual int columnCount(const QModelIndex &parent) const override
    {
        if (parent.isValid())
            return static_cast<mpchild_t*>(parent.internalPointer())->columnCount();
        else
            return m_modelparent->columnCount(parent);
    }

    virtual bool hasChildren(const QModelIndex &parent) const override
    {
        const TreeElement * parentItem = getItem(parent);
        if(parentItem)
            return parentItem->childCount() > 0;
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

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override
    {
        TreeElement *parentItem = getItem(parent);
        bool success;

        beginInsertRows(parent, row, row + count - 1);
        success = parentItem->insertChildren(row, count);
        endInsertRows();

        return success;
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent) override
    {
        TreeElement *parentItem = getItem(parent);
        bool success = true;

        beginRemoveRows(parent, row, row + count - 1);
        success = parentItem->removeChildren(row, count);
        endRemoveRows();

        return success;
    }

    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override
    {
        Q_ASSERT(false);
        return false;
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
        return const_cast<my_t*>(this)->getItem(index);
    }

protected:
    modelparent_t * m_modelparent;
};


#endif // TREEELEM_HPP
