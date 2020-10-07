#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H
#if 0
/*
*/
#include "sprite_container.hpp"
#include <QScopedPointer>
#include <cstdint>
#include <qstring.h>
#include <QTreeView>
#include <QDebug>
#include <QAbstractItemModel>
#include <src/data/treenode.hpp>

//
class ExNoContainer : public BaseException {public:using BaseException::BaseException;}; //No container loaded

/*
 * SpriteManager
 *
 * Model containing the list of things loaded. Used by the QTreeView in the main window!
 * The kind of content is abstracted by the type of container.
*/
class SpriteManager : public QAbstractItemModel
{
    friend class SpriteContainer;
    Q_OBJECT
public:

    SpriteManager();
    ~SpriteManager();

    static SpriteManager & Instance();

    //Info
    bool IsContainerLoaded()const;
//    bool ContainerIsPackFile()const;
//    bool ContainerIsSingleSprite()const;

    //IO
    SpriteContainer * OpenContainer(const QString & fname);
    void CloseContainer();

    int SaveContainer();
    int SaveContainer(const QString & fname);

    void ExportContainer(const QString & fname);
    SpriteContainer *ImportContainer(const QString & fname);
    SpriteContainer *NewContainer(const QString & type);

    //Empty the sprite list and etc.
    void Reset();

//    void AddSpriteToContainer  ( Sprite && spr );
//    void RemSpriteFromContainer( QModelIndex index );

//    bool DumpSprite(const QModelIndex &index, const QString &path);
//    void DumpPalette(const Sprite * sprite, const QString &path, ePaletteDumpType type);
//    void DumpPalette(const QModelIndex & sprite, const QString &path, ePaletteDumpType type);
//    void ImportPalette(Sprite * sprite, const QString &path, ePaletteDumpType type);

    //Access
    //int     GetNbSpritesInContainer()const;
    QString GetContainerSrcFile()const;

    const QString & GetContainerType()const;

    //Returns a short string to indicate in the status bar the kind of container working on!
    QString getContentShortName()const;

    //Tool for retrieving animation table templates names
//    QStringList       & getAnimSlotNames();
//    const QStringList & getAnimSlotNames()const;

    SpriteContainer * getContainer();

    //Forces the model to update its associated views
    void ForceTreeModelRefrech()const;


    //QAbstractItemModel
//    public:
//        using QAbstractItemModel::createIndex;
//        using QAbstractItemModel::beginRemoveRows;
//        using QAbstractItemModel::beginInsertRows;
//        using QAbstractItemModel::endRemoveRows;
//        using QAbstractItemModel::endInsertRows;
public:
    Qt::ItemFlags flags(const QModelIndex &index  = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    TreeNode *getItem(const QModelIndex &index) const;

    QModelIndex modelIndexOf(TreeNode *ptr) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index = QModelIndex()) const override;


    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    bool removeRows(int position, int rows, const QModelIndex &parent)override;

    bool insertRows(int position, int rows, const QModelIndex &parent)override;

    //Incremental load
    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

private:
    QScopedPointer<SpriteContainer> m_container; //<-- MAKE THIS A QPOINTER And destroy manually or via the item model!!!!!
    QStringList m_animslotnames;
};

#endif
#endif // SPRITEMANAGER_H
