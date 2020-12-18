#ifndef SPRITECONTAINER_H
#define SPRITECONTAINER_H
#include <QVector>
#include <QMap>
#include <QFile>
#include <QDataStream>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QList>
#include <QDebug>
#include <QMenu>
#include <cstdint>
#include <QtConcurrent/QtConcurrent>
#include <src/data/sprite/sprite.hpp>
#include <src/data/treenode.hpp>
#include <src/data/base_container.hpp>
/*
*/
extern const QString ElemName_SpriteContainer;
extern const QString ContentName_Sprite;

/*
 * SpriteContainerMenu
 *  Menu for options specific to the sprite container
*/
class SpriteContainer;
class SpriteContainerMenu : public QMenu
{
public:
    SpriteContainerMenu(SpriteContainer * container, QWidget * parent = nullptr);
    virtual ~SpriteContainerMenu();

    void InitContent();

public slots:

private:
    SpriteContainer * m_container {nullptr};
};

/*
 *  SpriteContainer
 *
 * The TreeView's invisible root node, and also an abstraction layer over
 * a single or several sprites loaded. Also, abstract access to PACK files' content.
*/
class SpriteContainer : public BaseContainer
{
    Q_OBJECT
public:
    typedef QList<Sprite*>          list_t;
    typedef list_t::iterator        iterator;
    typedef list_t::const_iterator  const_iterator;
    typedef uint32_t sprid_t;
    enum struct eContainerType
    {
        NONE,
        WAN,
        WAT,
        PACK,
    };
    static const QMap<eContainerType, QString> ContainerTypeNames;
    static const QList<QString> SpriteContentCategories;

    //
    SpriteContainer(QObject * parent = nullptr);
    SpriteContainer(const QString & str, QObject * parent = nullptr);
    SpriteContainer(const SpriteContainer & cp);
    SpriteContainer(SpriteContainer && mv);
    ~SpriteContainer();

    SpriteContainer& operator=(const SpriteContainer & cp);
    SpriteContainer& operator=(SpriteContainer && mv);

    //
    void Initialize()override;

    //
    bool isContainerLoaded()const override;
    bool isContainerPackFile()const;
    bool isContainerSingleSprite()const;
    bool isMultiItemContainer()const override;

    //void           SetSpriteContainerType(eContainerType ty);
    //eContainerType GetSpriteContainerType()const;

    const QString & GetContainerType()const override;
    eContainerType GetContainerTypeEnum()const;
    void SetContainerType(const QString & newtype)override;
    void SetContainerType(eContainerType newtype);

    QString         GetContainerSrcFnameOnly()const override;
    const QString & GetContainerSrcPath()const override;
    void            SetContainerSrcPath(const QString & path)override;

    int GetNbDataColumns()const override;
    QVariant GetContentData(const QModelIndex & index, int role)const override;
    QVariant GetContentHeaderData(int section, Qt::Orientation orientation, int role)const override;

    //
    const QString& GetTopNodeName()const override;

    //
    bool canFetchMore(const QModelIndex & index)const override;
    void fetchMore(const QModelIndex & index)override;

    //
    void LoadContainer()override;
    int WriteContainer()const override;

    //
    void ImportContainer(const QString & path)override;
    void ExportContainer(const QString & path, const QString & exportype)const override;

    //
    Sprite * GetSprite( sprid_t idx );
    sprid_t  AddSprite();

    //
    eCompressionFmtOptions GetExpectedCompression()const;
    void SetExpectedCompression(eCompressionFmtOptions compression);

    fmt::eSpriteType getExpectedSpriteType()const;
    void setExpectedSpriteType(fmt::eSpriteType sprty);

    //Returns a QMenu that offers actions to be done on this container.
    QMenu *MakeActionMenu(QWidget * parent)override;

    //Container standard access
    iterator         begin();
    const_iterator   begin()const;
    iterator         end();
    const_iterator   end()const;
    bool             empty()const;

    // TreeNode interface
public:
    TreeNode *clone() const override;
    int indexOfChild(const TreeNode *ptr) const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

    //Returns the sprite that owns the content at the given index
    TreeNode *getOwnerNode(const QModelIndex & index)override;
    const TreeNode *getOwnerNode(const QModelIndex & index)const override;

//        template<class _MANAGERTY>
//            QModelIndex index(int row, int column, const QModelIndex &parent, const _MANAGERTY * manager )
//        {
//            if ( column != 0)
//                return QModelIndex();

//            TreeNode *parentItem = getItem(parent);
//            TreeNode *childItem  = parentItem->nodeChild(row);
//            if (childItem)
//                return manager->createIndex(row, column, childItem);
//            else
//                return QModelIndex();
//        }

//        template<class _MANAGERTY>
//            QModelIndex parent(const QModelIndex &index, const _MANAGERTY * manager)
//        {
//            if (!index.isValid() )
//                return QModelIndex();

//            if( index.internalPointer() == this || index.internalPointer() == manager )
//                return QModelIndex();

//            if(m_spr.empty())
//                return QModelIndex();

//            TreeNode *childItem = getItem(index);
//            TreeNode *parentItem = childItem->parentNode();
//            Q_ASSERT(parentItem != nullptr);

//            if (parentItem == this)
//                return QModelIndex();

//            return manager->createIndex(parentItem->nodeIndex(), 0, parentItem);
//        }

//        template<class _MANAGERTY>
//            bool removeRows(int position, int rows, const QModelIndex &parent, _MANAGERTY * manager)
//        {
//            TreeNode *parentItem = getItem(parent);
//            bool success = true;

//            manager->beginRemoveRows(parent, position, position + rows - 1);
//            success = parentItem->removeChildrenNodes(position, rows);
//            manager->endRemoveRows();

//            return success;
//        }

//        template<class _MANAGERTY>
//            bool insertRows(int position, int rows, const QModelIndex &parent, _MANAGERTY * manager)
//        {
//            TreeNode *parentItem = getItem(parent);
//            bool success;

//            manager->beginInsertRows(parent, position, position + rows - 1);
//            success = parentItem->insertChildrenNodes(position, rows);
//            manager->endInsertRows();

//            return success;
//        }

    bool _insertChildrenNode(TreeNode *node, int destrow) override;
    bool _insertChildrenNodes(int row, int count)override;
    bool _insertChildrenNodes(const QList<TreeNode*> & nodes, int destrow = -1)override;

    bool _removeChildrenNode(TreeNode *node) override;
    bool _removeChildrenNodes(int row, int count)override;
    bool _removeChildrenNodes(const QList<TreeNode*> & nodes)override;

    //Remove and delete children nodes
    bool _deleteChildrenNode(TreeNode *node) override;
    bool _deleteChildrenNodes(int row, int count)override;
    bool _deleteChildrenNodes(const QList<TreeNode*> & nodes)override;

    //Move children nodes between postions
    bool _moveChildrenNodes(int row, int count, int destrow, TreeNode* destnode)override;
    bool _moveChildrenNodes(QModelIndexList &indices, int destrow, QModelIndex destparent) override;
    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent) override;
private:
    bool _removeChildrenNode(TreeNode *node, bool bdeleteptr = false); //Helper override, to avoid repeating code
    bool _removeChildrenNodes(const QList<TreeNode *> &nodes, bool bdeleteptr = false); //Helper override, to avoid repeating code

public:
    void        appendChild(TreeNode *item);
    TreeNode *  nodeChild(int row)override;
    int         nodeChildCount() const override;
    int         nodeIndex() const override;
    TreeNode *  parentNode() override;
    bool        nodeIsMutable()const override;
    QString     nodeDisplayName()const override;

private:
    void FetchToC(QDataStream & fdat);
    void LoadEntry(sprid_t idx);

private:
    QString         m_srcpath;      //Original path of the container if applicable!
    list_t          m_spr;          //List of all the contained sprites
    eContainerType  m_cntTy     {eContainerType::NONE};
    QThread         m_workthread;
    eCompressionFmtOptions  m_cntCompression {eCompressionFmtOptions::NONE};
    fmt::eSpriteType        m_cntsprty {fmt::eSpriteType::Character}; //By default the only sprite containers with several sprites are character sprites

signals:
    void startThread();
    void showProgress(QFuture<void>&)const;
    void showProgress(QFuture<void>&,QFuture<void>&)const;
};

#endif // SPRITECONTAINER_H
