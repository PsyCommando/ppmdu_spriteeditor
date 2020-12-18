#include "content_manager.hpp"
#include <src/data/content_factory.hpp>
#include <QDebug>

const QString ContainerTypeNone = "None";

ContentManager &ContentManager::Instance()
{
    static ContentManager s_manager;
    return s_manager;
}

ContentManager::ContentManager()
{
}

ContentManager::~ContentManager()
{
    qDebug("ContentManager::~ContentManager(): Deleting sprite manager!\n");
}

void ContentManager::Reset()
{
    m_container.reset();
}

bool ContentManager::isContainerLoaded() const
{
    return m_container;
}

const QString &ContentManager::getContainerType() const
{
    if(!m_container)
        return ContainerTypeNone;
    return m_container->GetContainerType();
}

QString ContentManager::getContentShortName() const
{
    return getContainerType();
}

QString ContentManager::getContainerSrcFile() const
{
    if(m_container)
        return m_container->GetContainerSrcPath();
    else
        return QString();
}

BaseContainer *ContentManager::getContainer()
{
    return m_container.data();
}

void ContentManager::NewContainer(const QString &type)
{
    m_container.reset(ContentContainerFactory::instance().MakeContainerFromType(type));
    if(!m_container)
    {
        qWarning() << "ContentManager::NewContainer(): Failed to find a container type named \'" <<type << "\'!";
        throw std::runtime_error("ContentManager::NewContainer(): Unknown container type!");
    }
    m_container->Initialize();
}

void ContentManager::OpenContainer(const QString &fname)
{
    m_container.reset(ContentContainerFactory::instance().MakeContainer(fname));
    if(!m_container)
    {
        qWarning() << "ContentManager::OpenContainer(): Failed to find a container type to open the file \'" <<fname << "\'!";
        throw std::runtime_error("ContentManager::OpenContainer(): Unknown file format!");
    }
    m_container->LoadContainer(fname);
    m_container->Initialize();
}

void ContentManager::CloseContainer()
{
    qInfo("ContentManager::CloseContainer(): Close container called!");
    Reset();
}

int ContentManager::SaveContainer()
{
    if(!m_container)
        throw ExNoContainer("ContentManager::SaveContainer(): No container loaded!");
    return SaveContainer(m_container->GetContainerSrcPath());
}

int ContentManager::SaveContainer(const QString &fname)
{
    if(!m_container)
        throw ExNoContainer("ContentManager::SaveContainer(): No container loaded!");
    m_container->SetContainerSrcPath(fname);
    return m_container->WriteContainer();
}

void ContentManager::ExportContainer(const QString &fname, const QString & exportType)
{
    if(!m_container)
        throw ExNoContainer("ContentManager::ExportContainer(): No container loaded!");
    m_container->ExportContainer(fname, exportType);
}

void ContentManager::ImportContainer(const QString &fname)
{
    //#TODO: need more details to work out how to get the possible import options
    Q_ASSERT(false);
}

QModelIndex ContentManager::modelIndexOf(TreeNode *ptr) const
{
    if (!ptr || !isContainerLoaded())
        return QModelIndex();
    TreeNode * parent = ptr->parentNode();
    return createIndex(parent->indexOfChild(ptr), 0, ptr);
}

TreeNode *ContentManager::getOwnerNode(const QModelIndex &index)
{
    BaseContainer * cnt = ContentManager::Instance().getContainer();
    if(!cnt)
        return nullptr;
    return cnt->getOwnerNode(index);
}

bool ContentManager::isMultiItemContainer() const
{
    return m_container? m_container->isMultiItemContainer() : false;
}

TreeNode *ContentManager::getItem(const QModelIndex &index)
{
    if (index.isValid() || !isContainerLoaded())
    {
        TreeNode *item = reinterpret_cast<TreeNode*>(index.internalPointer());
        return item;
    }
    return m_container.data();
}

Qt::ItemFlags ContentManager::flags(const QModelIndex &index) const
{
    if (!isContainerLoaded())
        return QAbstractItemModel::flags(index);

    TreeNode * pnode = nullptr;
    if(!index.isValid())
        pnode = m_container.data();
    else
        pnode = static_cast<TreeNode*>(index.internalPointer());

    if(!pnode)
        return QAbstractItemModel::flags(index);
    return pnode->nodeFlags();
}

QVariant ContentManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !isContainerLoaded())
        return QVariant();
    return m_container->GetContentData(index, role);
}

QVariant ContentManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(!isContainerLoaded())
        return QVariant();
    return m_container->GetContentHeaderData(section, orientation, role);
}

int ContentManager::columnCount(const QModelIndex &/*parent*/) const
{
    if(!isContainerLoaded())
        return 0;
    return m_container->GetNbDataColumns();
}

int ContentManager::rowCount(const QModelIndex &parent) const
{
    if (!isContainerLoaded())
        return 0;
    if(!parent.isValid()) //We return the parent on invalid nodes
        return m_container->nodeChildCount();
    const TreeNode *parentItem = static_cast<const TreeNode*>(parent.internalPointer());
    Q_ASSERT(parentItem);
    if(!parentItem->nodeShowChildrenOnTreeView())
        return 0;
    return parentItem->nodeChildCount();
}

bool ContentManager::hasChildren(const QModelIndex &parent) const
{
    if(!isContainerLoaded())
        return false;
    if(!parent.isValid()) //We return the parent on invalid nodes
        return m_container->nodeHasChildren();
    const TreeNode *parentItem = static_cast<const TreeNode*>(parent.internalPointer());
    Q_ASSERT(parentItem);
    return parentItem->nodeShowChildrenOnTreeView() && parentItem->nodeHasChildren();
}

QModelIndex ContentManager::index(int row, int column, const QModelIndex &parent) const
{
    if (!isContainerLoaded())
        return QModelIndex();
    Q_ASSERT(column == 0);

    TreeNode *parentItem =  parent.isValid()? static_cast<TreeNode*>(parent.internalPointer()) : m_container.data();
    Q_ASSERT(parentItem);
    TreeNode *childItem  = parentItem->nodeChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex ContentManager::parent(const QModelIndex &index) const
{
    if (!isContainerLoaded() || !m_container->nodeHasChildren() || !index.isValid())
        return QModelIndex();
    if(index.internalPointer() == this || index.internalPointer() == m_container.get())
        return QModelIndex();
    TreeNode *childItem = static_cast<TreeNode*>(index.internalPointer());
    TreeNode *parentItem = childItem->parentNode();
    Q_ASSERT(parentItem != nullptr);
    if (parentItem == m_container.get())
        return QModelIndex();
    return createIndex(parentItem->nodeIndex(), 0, parentItem);
}

bool ContentManager::removeRows(int position, int rows, const QModelIndex &/*parent*/)
{
    if (!isContainerLoaded())
        return false;
    //changing the container handles updating the model in this case
    return m_container->_removeChildrenNodes(position, rows);
}

bool ContentManager::insertRows(int position, int rows, const QModelIndex &/*parent*/)
{
    if (!isContainerLoaded())
        return false;
    //changing the container handles updating the model in this case
    return m_container->_insertChildrenNodes(position, rows);
}

void ContentManager::fetchMore(const QModelIndex &parent)
{
    if (!isContainerLoaded())
        return;
    if(canFetchMore(parent))
        m_container->fetchMore(parent);
    else
    {
        qWarning() << "Couldn't fetchMore sprite #" << parent.row() <<"\n";
    }
}

bool ContentManager::canFetchMore(const QModelIndex &parent) const
{
    if(!isContainerLoaded())
        return false;
    return m_container->canFetchMore(parent);
}
