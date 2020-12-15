#ifndef TREENODEMODEL_HPP
#define TREENODEMODEL_HPP
#include <QAbstractItemModel>
#include <src/data/treenode.hpp>

//**************************************************************************************
//  TreeNodeModel
//**************************************************************************************
//Model for representing a structure of TreeNodes
class TreeNodeModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TreeNodeModel); //Base class won't allow it
public:
    using node_t = TreeNode; //Type of the base node we're using

    TreeNodeModel(QObject* parent = nullptr) : QAbstractItemModel(parent) {}
    virtual ~TreeNodeModel(){}

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        const node_t *item = getItem(index);
        if(item)
            return item->nodeFlags(index.column());
        else
            return Qt::ItemFlag::NoItemFlags;
    }

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override
    {
        const node_t *parentItem = getItem(parent);
        const node_t *childItem  = parentItem->nodeChild(row);
        if (childItem)
            return createIndex(row, column, const_cast<node_t*>(childItem));
        else
            return QModelIndex();
    }

    virtual QModelIndex indexOfChildNode(const TreeNode * childItem, int hdrcolumn = 0)const
    {
        const node_t *parentnode = childItem->parentNode();
        if (!childItem || childItem == getRootNode() || parentnode != getRootNode())
            return QModelIndex();
        return createIndex(parentnode->indexOfChild(childItem), hdrcolumn, const_cast<node_t*>(childItem));
    }

    virtual QModelIndex parent(const QModelIndex &child) const override
    {
        if(!child.isValid())
            return QModelIndex();
        const node_t *childItem = getItem(child);
        Q_ASSERT(childItem);
        const node_t *parentItem = childItem->parentNode();
        Q_ASSERT(parentItem != nullptr);

        if (parentItem == getRootNode())
            return QModelIndex();

        return createIndex(parentItem->nodeIndex(), 0, const_cast<node_t*>(parentItem));
    }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return static_cast<node_t*>(parent.internalPointer())->nodeChildCount();
        else
            return getRootNode()->nodeChildCount();
    }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return 0;
        else
            return 1;
    }

    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override
    {
        if(!parent.isValid())
            return false;
        const node_t * parentItem = getItem(parent);
        if(parentItem)
            return parentItem->nodeHasChildren();
        else
            return false;
    }

    //Default data implementation returns the data type name and index
    virtual QVariant data(const QModelIndex &index, int role) const override
    {
        if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
            return QVariant();

        if(index.column() != 0)
            return QVariant();
        TreeNode * node = static_cast<TreeNode*>(index.internalPointer());
        QString sprname = QString("%1#%2").arg(node->nodeDataTypeName()).arg(index.row());
        return QVariant(sprname);
    }

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
    {
        node_t *parentItem = getItem(parent);
        bool success = false;

        if( count > 0 )
        {
            beginInsertRows(parent, row, row + count - 1);
            success = parentItem->_insertChildrenNodes(row, count);
            endInsertRows();
        }

        return success;
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
    {
        node_t *parentItem = getItem(parent);
        bool success = true;

        if( count > 0 )
        {
            beginRemoveRows(parent, row, row + count - 1);
            success = parentItem->_removeChildrenNodes(row, count);
            endRemoveRows();
        }
        return success;
    }

    virtual bool removeRows(QModelIndexList & indices, const QModelIndex &parent = QModelIndex())
    {
        node_t *parentItem = getItem(parent);
        bool success = false;

        if(indices.size() > 0)
        {
            success = true;
            QModelIndexList origIndices = persistentIndexList(); /////#FIXME!!!!: not the entire list of indices!!!
            QModelIndexList changedIndices = origIndices;
            QList<TreeNode*> toremove;

            layoutAboutToBeChanged();

            //Make a list of things to remove, and clear the index from the
            for(int i = 0; i < changedIndices.size(); ++i)
            {
                QModelIndex & idx = changedIndices[i];
                auto itfound = std::find_if(indices.begin(), indices.end(), [idx](QModelIndex &idc)->bool{return idc == idx;});
                if(itfound != indices.end())
                {
                    origIndices.removeAt(i);    //Remove entries that we removed from the indices to update
                    changedIndices.removeAt(i); //Remove entries that we removed from the indices to update
                    //toremove.push_back(static_cast<TreeNode*>(idx.internalPointer()));
                    if(i > 0)
                        --i; //Make sure we don't skip the entry after the one we deleted from the changed list!
                }
            }

            //Add the indices to be removed
            for(const QModelIndex & idx : indices)
            {
                TreeNode* premove = static_cast<TreeNode*>(idx.internalPointer());
                toremove.push_back(premove);
            }

            //Actually let the node remove its childs
            success = success & parentItem->_removeChildrenNodes(toremove);

            //Update persistent indices
            for(QModelIndex & idx : changedIndices)
            {
                if(!idx.isValid())
                    continue;
                TreeNode * curelem = static_cast<TreeNode*>(idx.internalPointer());
                int newindice = parentItem->indexOfChild(curelem);
                if(newindice < 0)
                    idx = QModelIndex();
                else
                    idx = createIndex(newindice, 0, curelem);
            }

            //Update the persistent indices from the lists of changed indices
            changePersistentIndexList(origIndices, changedIndices);
            layoutChanged();
        }
        return success;
    }

    virtual bool moveRows(const QModelIndex &sourceParent,
                          int sourceRow,
                          int count,
                          const QModelIndex &destinationParent,
                          int destinationChild) override
    {
        node_t *srcparentItem = getItem(sourceParent);
        node_t *destparentItem = getItem(destinationParent);

        if( destinationParent == sourceParent && sourceRow == destinationChild )
            return true;

        if(destinationParent == sourceParent)
        {
            int newdest = (sourceRow < destinationChild)? destinationChild + 1 : destinationChild;
            if(beginMoveRows(sourceParent, sourceRow, sourceRow + (count - 1), destinationParent, newdest))
            {
                bool result = srcparentItem->_moveChildrenNodes(sourceRow, count, destinationChild, destparentItem);
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

    virtual bool moveRows(QModelIndexList & indices, int destrow, QModelIndex destparent = QModelIndex() )
    {
        if(indices.empty())
            return false;
        QModelIndex srcparentidx = indices.first().parent();
        node_t *srcparentItem = getItem(srcparentidx);
        node_t *destparentItem = getItem(destparent);
        if(srcparentItem == destparentItem)
        {
            if(beginMoveRows(srcparentidx, indices.first().row(), indices.last().row(), destparent, destrow))
            {
                bool result = srcparentItem->_moveChildrenNodes(indices, destrow, destparent);
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

    virtual node_t *getItem(const QModelIndex &index)
    {
        if (!index.isValid())
            return getRootNode();
        return reinterpret_cast<node_t*>(index.internalPointer());
    }
    const node_t *getItem(const QModelIndex &index)const {return const_cast<TreeNodeModel*>(this)->getItem(index);}

    //Convenience function for emiting the datachanged signal
    inline void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ())
    {
        emit dataChanged(topLeft, bottomRight, roles);
    }

    //Implement those to get the model's root node
    virtual node_t * getRootNode()=0;
    virtual const node_t * getRootNode()const {return const_cast<TreeNodeModel*>(this)->getRootNode();}

    //Implement those to get the sprite that owns
    virtual Sprite* getOwnerSprite() = 0;
    virtual const Sprite* getOwnerSprite()const {return const_cast<TreeNodeModel*>(this)->getOwnerSprite();}
};

#endif // TREENODEMODEL_HPP
