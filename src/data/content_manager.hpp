#ifndef CONTENTMANAGER_HPP
#define CONTENTMANAGER_HPP
#include <QAbstractItemModel>
#include <QScopedPointer>
#include <src/utility/baseqtexception.hpp>
#include <src/data/base_container.hpp>

//
class ExNoContainer : public BaseException {public:using BaseException::BaseException;}; //No container loaded

/*
 * SpriteManager
 *
 * Model containing the list of things loaded. Used by the QTreeView in the main window!
 * The kind of content is abstracted by the type of container.
*/
class ContentManager : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ContentManager);
    ContentManager();
public:
    static ContentManager & Instance();
    ~ContentManager();
    void Reset(); //Empty everything and return to default state

    //Info
    bool isContainerLoaded()const;
    const QString &getContainerType()const;
    QString getContentShortName()const; //Returns a short string to indicate in the status bar the kind of container working on!
    QString getContainerSrcFile()const;
    BaseContainer *getContainer();

    QString getContainerParentDir()const;  //Returns the parent directory of the loaded container if it has been saved at least once
    QString getContainerFileFilter()const; //Return the file filter to use for the container in a open/save file dialog

    //IO
    void NewContainer(const QString & type);
    void OpenContainer(const QString & fname);
    void CloseContainer();

    int SaveContainer();
    int SaveContainer(const QString & fname);

    void ExportContainer(const QString & fname, const QString & exportType);
    void ImportContainer(const QString & fname);

    //Helpers
    QModelIndex modelIndexOf(const TreeNode *ptr) const;
    TreeNode *getItem(const QModelIndex &index);
    const TreeNode *getItem(const QModelIndex &index)const {return const_cast<ContentManager*>(this)->getItem(index);}
    TreeNode *getOwnerNode(const QModelIndex &index);
    const TreeNode *getOwnerNode(const QModelIndex &index)const {return const_cast<ContentManager*>(this)->getOwnerNode(index);}

    bool isMultiItemContainer()const;

    //QAbstractItemModel
public:
    Qt::ItemFlags flags(const QModelIndex &index  = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index = QModelIndex()) const override;

    bool removeRows(int position, int rows, const QModelIndex &parent)override;
    bool insertRows(int position, int rows, const QModelIndex &parent)override;

    //Incremental load
    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

    //Made those public because containers needs to trigger updates
    using QAbstractItemModel::beginInsertRows;
    using QAbstractItemModel::endInsertRows;

    using QAbstractItemModel::beginRemoveRows;
    using QAbstractItemModel::endRemoveRows;

    using QAbstractItemModel::beginMoveRows;
    using QAbstractItemModel::endMoveRows;

    using QAbstractItemModel::beginInsertColumns;
    using QAbstractItemModel::endInsertColumns;

    using QAbstractItemModel::beginRemoveColumns;
    using QAbstractItemModel::endRemoveColumns;

    using QAbstractItemModel::beginMoveColumns;
    using QAbstractItemModel::endMoveColumns;

    using QAbstractItemModel::layoutAboutToBeChanged;
    using QAbstractItemModel::layoutChanged;

    using QAbstractItemModel::changePersistentIndex;
    using QAbstractItemModel::changePersistentIndexList;
    using QAbstractItemModel::persistentIndexList;

public slots:

signals:
    void contentChanged(); //Emited when the container and content changed and need the ui to be redrawn

private:
    QScopedPointer<BaseContainer> m_container;
};

#endif // CONTENTMANAGER_HPP
