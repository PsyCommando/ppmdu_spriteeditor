#include "sprite_container.hpp"
#include <QMessageBox>
#include <QString>
#include <QSaveFile>
#include <QtConcurrent>
#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/packfile.hpp>
#include <src/data/sprite/spritemanager.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/ui/dialogprogressbar.hpp>
#include <src/utility/threadedwriter.hpp>
#include <src/data/content_factory.hpp>
#include <src/data/content_manager.hpp>

const QString NullString;
const QString PackFileExt = "bin";
const QString WANFileExt  = "wan";
const QString WATFileExt  = "wat";

const QString ElemName_SpriteContainer = "Container";
const QString ContentName_Sprite = "sprite";

bool FileIsSpriteContainer(const QString & filepath);

//Define the container as a handler for sprite containers
DEFINE_CONTAINER(SpriteContainer, ContentName_Sprite, FileIsSpriteContainer);

//=================================================================================================
//  SpriteContainer
//=================================================================================================
const QList<QString> SpriteContainer::SpriteContentCategories=
{
    "Images",
    "Frames",
    "Sequences",
    "Animations",
    "Palette",
    "Effects Offsets",
};

const QMap<SpriteContainer::eContainerType, QString> SpriteContainer::ContainerTypeNames
{
    {SpriteContainer::eContainerType::NONE, ("NONE")},
    {SpriteContainer::eContainerType::PACK, ("PACK")},
    {SpriteContainer::eContainerType::WAN,  ("WAN")},
    {SpriteContainer::eContainerType::WAT,  ("WAT")},
};

SpriteContainer::SpriteContainer(QObject *parent)
    :BaseContainer(parent), m_cntTy(eContainerType::NONE)
{
}

SpriteContainer::SpriteContainer(const QString &str, QObject *parent)
    :BaseContainer(parent), m_srcpath(str), m_cntTy(eContainerType::NONE)
{}

SpriteContainer::SpriteContainer(const SpriteContainer & cp)
    :BaseContainer(cp)
{
    operator=(cp);
}

SpriteContainer::SpriteContainer(SpriteContainer && mv)
    :BaseContainer(mv)
{
    operator=(mv);
}

SpriteContainer& SpriteContainer::operator=(const SpriteContainer & cp)
{
    setParent(cp.parent());
    TreeNode::operator=(cp);
    m_srcpath   = cp.m_srcpath;
    m_cntTy     = cp.m_cntTy;

    //copy all the sprites
    m_spr.reserve(cp.m_spr.size());
    for(Sprite* spr : m_spr)
        m_spr.push_back(new Sprite(*spr));
    return *this;
}
SpriteContainer& SpriteContainer::operator=(SpriteContainer && mv)
{
    setParent(mv.parent());
    TreeNode::operator=(mv);
    m_srcpath   = mv.m_srcpath;
    m_spr       = qMove(mv.m_spr);
    mv.m_spr.clear();
    m_cntTy     = mv.m_cntTy;
    return *this;
}

void SpriteContainer::Initialize()
{
    if(isContainerLoaded())
        return; //Don't do anything when we're already filed up!
    if(ContainerIsSingleSprite())
    {
        //We want to add a single empty sprite for a single sprite!
        AddSprite();
    }
}

bool SpriteContainer::isContainerLoaded() const
{
    return !m_spr.empty() || (m_cntTy != eContainerType::NONE);
}

SpriteContainer::~SpriteContainer()
{
    //m_rootelem = nullptr;
    qDebug("SpriteContainer::~SpriteContainer(): Deleting sprite container!\n");
    if(m_workthread.isRunning())
    {
        m_workthread.terminate();
        qDebug("SpriteContainer::~SpriteContainer(): Waiting on thread..\n");
        m_workthread.wait();
    }
    qDebug("SpriteContainer::~SpriteContainer(): Done!\n");
    qDeleteAll(m_spr);
}

bool SpriteContainer::ContainerIsPackFile() const
{
    return isContainerLoaded() && m_cntTy == eContainerType::PACK;
}

bool SpriteContainer::ContainerIsSingleSprite() const
{
    return !ContainerIsPackFile();
}

const QString &SpriteContainer::GetContainerType() const
{
    switch(m_cntTy)
    {
    case eContainerType::PACK:
        return PackFileExt;
    case eContainerType::WAN:
        return WANFileExt;
    case eContainerType::WAT:
        return WATFileExt;
    default:
        break;
    };
    return NullString;
}

void SpriteContainer::SetContainerType(const QString &newtype)
{
    if(newtype == PackFileExt)
        m_cntTy = eContainerType::PACK;
    else if(newtype == WANFileExt)
        m_cntTy = eContainerType::WAN;
    else if(newtype == WATFileExt)
        m_cntTy = eContainerType::WAT;
    else
    {
        throw std::runtime_error("SpriteContainer::SetContainerType(): Unknown container type!");
    }
}

//void SpriteContainer::SetContainerType(SpriteContainer::eContainerType ty){m_cntTy = ty;}
//SpriteContainer::eContainerType SpriteContainer::GetContainerType() const           {return m_cntTy;}

void SpriteContainer::LoadContainer()
{
    QFile container(m_srcpath);

    if( !container.open(QIODevice::ReadOnly) || !container.exists() || container.error() != QFileDevice::NoError )
    {
        //Error can't load file!
        qWarning( container.errorString().toLocal8Bit().data() );
        QMessageBox msgBox;
        msgBox.setText("Failed to load file!");
        msgBox.setInformativeText(container.errorString());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QByteArray data = container.readAll();
    ContentManager & manager = ContentManager::Instance();

    m_spr.clear();
    //Lets identify the format
    if(m_srcpath.endsWith(PackFileExt))
    {
        fmt::PackFileLoader ldr;
        ldr.Read(data.begin(), data.end());
        m_spr.reserve(ldr.size());

        //Load the raw data into each sprites but don't parse them yet!
        manager.beginInsertRows( QModelIndex(), 0, ldr.size() - 1);

        for( size_t cnt = 0; cnt < ldr.size(); ++cnt )
        {
            Sprite * pspr = new Sprite(this);
            ldr.CopyEntryData( cnt, std::back_inserter(pspr->getRawData()) );
            m_spr.push_back(pspr);
        }
        manager.endInsertRows();

        m_cntTy = eContainerType::PACK;
    }
    else if( m_srcpath.endsWith(WANFileExt) )
    {
        //We load the whole sprite
        manager.beginInsertRows( QModelIndex(), 0, 0);
        m_spr.push_back(new Sprite(this));
        std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front()->getRawData()) );
        manager.endInsertRows();
        m_cntTy = eContainerType::WAN;
    }
    else if( m_srcpath.endsWith(WATFileExt) )
    {
        //We load the whole sprite
        manager.beginInsertRows( QModelIndex(), 0, 0);
        m_spr.push_back(new Sprite(this));
        std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front()->getRawData()) );
        manager.endInsertRows();
        m_cntTy = eContainerType::WAT;
    }

}

int SpriteContainer::WriteContainer()const
{
    QScopedPointer<QSaveFile> pcontainer(new  QSaveFile(m_srcpath));

    if( !pcontainer->open(QIODevice::WriteOnly) || pcontainer->error() != QFileDevice::NoError )
    {
        //Error can't write file!
        qWarning( pcontainer->errorString().toLocal8Bit() );
        QMessageBox msgBox;
        msgBox.setText("Failed to write file!");
        msgBox.setInformativeText(pcontainer->errorString());
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return 0;
    }

    ThreadedWriter * pmthw = new ThreadedWriter(pcontainer.take(), ContentManager::Instance().getContainer());
    connect( &m_workthread, SIGNAL(finished()), pmthw, SLOT(deleteLater()) );
    connect(pmthw, SIGNAL(finished()), &m_workthread, SLOT(quit()));
    connect(pmthw, SIGNAL(finished()), pmthw, SLOT(deleteLater()));

    QThread * pworkThread = const_cast<QThread*>(&m_workthread); //#TODO: Maybe store work threads outside the instance, so we don't need to do shitty casts?
    //
    switch(m_cntTy)
    {
    case eContainerType::PACK:
        {
            connect(&m_workthread, SIGNAL(started()), pmthw, SLOT(WritePack()));
            pmthw->moveToThread(pworkThread);
            break;
        }
    case eContainerType::WAN:
    case eContainerType::WAT:
        {
            connect(&m_workthread, SIGNAL(started()), pmthw, SLOT(WriteSprite()));
            pmthw->moveToThread(pworkThread);
            break;
        }
    default:
        {
            Q_ASSERT(false);
            qFatal("SpriteContainer::WriteContainer(): Tried to write unknown filetype!!");
            break;
        }
    };
    pworkThread->start();
    emit showProgress(pmthw->op1, pmthw->op2);
    qDebug("SpriteContainer::WriteContainer(): progress dialog displayed!\n");
    return 0;
}

void SpriteContainer::ImportContainer(const QString &/*path*/)
{
    Q_ASSERT(false); //Need to be done!
}

void SpriteContainer::ExportContainer(const QString &/*path*/, const QString & exportype) const
{
    Q_ASSERT(false); //Need to be done!
}

Sprite* SpriteContainer::GetSprite(SpriteContainer::sprid_t idx)
{
    return m_spr[idx];
}

SpriteContainer::sprid_t SpriteContainer::AddSprite()
{
    ContentManager & manager = ContentManager::Instance();
    size_t offset = m_spr.size();

    manager.beginInsertRows( QModelIndex(), offset, offset );
    m_spr.push_back(new Sprite(this) );
    manager.endInsertRows();

    return offset;
}

SpriteContainer::iterator SpriteContainer::begin()
{
    return m_spr.begin();
}

SpriteContainer::const_iterator SpriteContainer::begin() const
{
    return m_spr.begin();
}

SpriteContainer::iterator SpriteContainer::end()
{
    return m_spr.end();
}

SpriteContainer::const_iterator SpriteContainer::end() const
{
    return m_spr.end();
}

bool SpriteContainer::empty() const
{
    return m_spr.empty();
}

//    QVariant SpriteContainer::data(const QModelIndex &index, int role) const
//    {
//        if(!ContainerLoaded() || m_spr.empty())
//            return QVariant();

//        if (!index.isValid())
//            return QVariant("root");

//        if (role != Qt::DisplayRole && role != Qt::EditRole)
//            return QVariant();

//        TreeNode *item = const_cast<SpriteContainer*>(this)->getItem(index);
//        return item->nodeData(index.column(), role);
//    }

//    QVariant SpriteContainer::headerData(int, Qt::Orientation, int) const
//    {
//        //nothing really
//        return QVariant();
//    }

//int SpriteContainer::rowCount(const QModelIndex &parent) const
//{
//    if (!isContainerLoaded() || m_spr.empty())
//        return 0;

//    TreeNode *parentItem = const_cast<SpriteContainer*>(this)->getItem(parent);
//    Q_ASSERT(parentItem);

//    //Exclude some items from being displayed as having childrens!
//    if(parentItem->nodeDataTy() == eTreeElemDataType::animSequence ||
//       //parentItem->getNodeDataTy() == eTreeElemDataType::animTable    ||
//       parentItem->nodeDataTy() == eTreeElemDataType::frame)
//        return 0;

//    return parentItem->nodeChildCount();
//}

//bool SpriteContainer::hasChildren(const QModelIndex &parent) const
//{
//    if(!isContainerLoaded() || m_spr.empty())
//        return false;

//    return rowCount(parent) > 0;
////        TreeNode * parentItem = const_cast<SpriteContainer*>(this)->getItem(parent);

////        if(parentItem)
////            return parentItem->nodeChildCount() > 0;
////        else
////            return false;
//}

//TreeNode *SpriteContainer::getItem(const QModelIndex &index)
//{
//    if (index.isValid())
//    {
//        TreeNode *item = reinterpret_cast<TreeNode*>(index.internalPointer());
//        if (item)
//            return item;
//    }
//    return this;
//}

TreeNode *SpriteContainer::getOwnerNode(const QModelIndex &index)
{
    if(!index.isValid())
        return this;
    TreeNode * curnode = reinterpret_cast<TreeNode*>(index.internalPointer());
    TreeNode * parent = nullptr;

    //Climb up the tree until we find the parent sprite, or we get null
    while(curnode)
    {
        parent = curnode->parentNode();
        if(parent && parent->nodeDataTy() == eTreeElemDataType::spritecontainer)
        {
            parent = curnode;
            break;
        }
        if(parent && parent->nodeDataTy() == eTreeElemDataType::sprite)
            break;
        curnode = parent;
    }
    return parent;
}

const TreeNode *SpriteContainer::getOwnerNode(const QModelIndex &index) const
{
    return const_cast<SpriteContainer*>(this)->getOwnerNode(index);
}

bool SpriteContainer::_insertChildrenNodes(int row, int count)
{
    if(row < 0 || row > m_spr.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginInsertRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
        m_spr.insert(row + i, new Sprite(this));
    manager.endInsertRows();
    return true;
}

bool SpriteContainer::_insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow)
{
    if(destrow < 0 || destrow > m_spr.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginInsertRows(QModelIndex(), destrow, destrow + nodes.size() - 1);
    for(int i = 0; i < nodes.size(); ++i)
        m_spr.insert(destrow + i, dynamic_cast<Sprite*>(nodes[i])); //dynamic cast since it'll return null if the TreeNode* isn't a Sprite* instead of causing worst troubles down the line
    manager.endInsertRows();
    return true;
}

bool SpriteContainer::_removeChildrenNodes(int row, int count)
{
    if(row < 0 || row > m_spr.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
        m_spr.removeAt(row + i);
    manager.endRemoveRows();
    return true;
}

bool SpriteContainer::_removeChildrenNodes(const QList<TreeNode *> &nodes)
{
    return _removeChildrenNodes(nodes, false);
}

bool SpriteContainer::_removeChildrenNodes(const QList<TreeNode *> &nodes, bool bdeleteptr)
{
    if(nodes.isEmpty())
        return true;
    if(nodes.size() > m_spr.size())
        return false;
    bool success = true;
    ContentManager & manager = ContentManager::Instance();
    QModelIndexList origindices    = manager.persistentIndexList();
    QModelIndexList changedindices = origindices;

    manager.layoutAboutToBeChanged();

    //Clear out the changed ones and removed the nodes first
    for(int i = 0; i < changedindices.size(); ++i)
    {
        QModelIndex & curidx = changedindices[i];
        int found = nodes.indexOf(static_cast<TreeNode*>(curidx.internalPointer()));
        if(found < 0)
            continue;
        //delete it from the table
        Sprite * pspr = static_cast<Sprite*>(curidx.internalPointer());
        m_spr.removeOne(pspr);
        if(bdeleteptr)
            delete pspr;
        changedindices.removeAt(i); //Clear from changed indices
        origindices.removeAt(i);    //Clear from changed indices
    }

    //Iterated the changed indices and find their new indices by pointer
    for(QModelIndex & idx : changedindices)
    {
        if(!idx.isValid())
            continue;
        int newrow = m_spr.indexOf(static_cast<Sprite*>(idx.internalPointer()));
        if(newrow < 0)
        {
            Q_ASSERT(false);
            throw std::runtime_error("SpriteContainer::_removeChildrenNodes(): Found non-existent node pointer when updating persistent index list!");
        }
        idx = manager.index(newrow, 0);
    }

    //Update persistent indices
    manager.changePersistentIndexList(origindices, changedindices);
    manager.layoutChanged();
    return success;
}

bool SpriteContainer::_deleteChildrenNodes(int row, int count)
{
    if(row < 0 || row > m_spr.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
    {
        delete m_spr[row + i];
        m_spr.removeAt(row + i);
    }
    manager.endRemoveRows();
    return true;
}

bool SpriteContainer::_deleteChildrenNodes(const QList<TreeNode *> &nodes)
{
    return _removeChildrenNodes(nodes, true);
}

bool SpriteContainer::_moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode)
{
    if(!destnode || ((row + count - 1) > m_spr.size()) || (destrow > destnode->nodeChildCount()))
        return false;
    bool                success = true;
    ContentManager &     manager = ContentManager::Instance();
    QList<TreeNode*>    moved;
    moved.reserve(count);

    manager.beginRemoveRows(QModelIndex(), row, row + (count - 1));
    for(int i = 0; i < count; ++i)
    {
        moved.push_back(m_spr[row]);
        m_spr.removeAt(row);
    }
    manager.endRemoveRows();
    return success? success & destnode->_insertChildrenNodes(moved, destrow) : false;
}

//    int SpriteContainer::columnCount(const QModelIndex &) const
//    {
//        //            if (parent.isValid())
//        //                return static_cast<TreeNode*>(parent.internalPointer())->columnCount();
//        //            else
//        return 1;
//    }

void SpriteContainer::appendChild(TreeNode *item)
{
    //QMutexLocker lk(&getMutex());
    Sprite * spritem = nullptr;
    spritem = dynamic_cast<Sprite*>(item);

    if(spritem)
        m_spr.append(spritem);
}

TreeNode *SpriteContainer::nodeChild(int row)
{
    return m_spr[row];
}

int SpriteContainer::nodeChildCount() const
{
    return m_spr.count();
}

int SpriteContainer::nodeIndex() const
{
    return 0; //Always first!
}

//int SpriteContainer::indexOfNode(const TreeNode *ptr) const
//{
//    QMutexLocker lk(&const_cast<SpriteContainer*>(this)->getMutex());
//    const Sprite * ptrspr = dynamic_cast<const Sprite *>(ptr);

//    if( ptrspr )
//        return m_spr.indexOf(*ptrspr);
//    return 0;
//}

//QModelIndex SpriteContainer::modelIndexOfNode(const TreeNode *ptr) const
//{
//    //#TODO: Its pretty useless if its limited to only sprites......
//    QMutexLocker lk(&const_cast<SpriteContainer*>(this)->getMutex());

//    if(ptr->nodeDataTy() != eTreeElemDataType::sprite)
//    {
//        qFatal(QString("SpriteContainer::modelIndexOfNode(): Got non-sprite node! Type is %1").arg(static_cast<int>(ptr->getNodeDataTy())).toStdString().c_str());
//        return QModelIndex();
//    }

//    const Sprite * ptrspr = dynamic_cast<const Sprite *>(ptr);
//    Q_ASSERT(ptrspr);

//    int idx = m_spr.indexOf(*ptrspr);
//    if(idx == -1)
//    {
//        Q_ASSERT(false);
//        throw std::runtime_error("SpriteContainer::modelIndexOfNode(): Couldn't find node in table! Node is possibly not a sprite!");
//    }

//    return spr_manager::ContentManager::Instance().index(idx, 0);
//}

//    QModelIndex spr_manager::SpriteContainer::modelIndex() const
//    {
//        return QModelIndex();
//    }

//    QModelIndex spr_manager::SpriteContainer::modelChildIndex(int row, int column) const
//    {
//        return spr_manager::ContentManager::Instance().index(row, column);
//    }

//    QModelIndex spr_manager::SpriteContainer::modelParentIndex() const
//    {
//        return spr_manager::ContentManager::Instance().parent();
//    }

//    int SpriteContainer::nodeColumnCount() const
//    {
//        return 1;
//    }

TreeNode *SpriteContainer::parentNode()
{
    return m_parentItem;
}

//    Sprite *SpriteContainer::parentSprite()
//    {
//        return nullptr;
//    }

//    QVariant SpriteContainer::nodeData(int column, int role) const
//    {
//        if( (role == Qt::DisplayRole || role == Qt::EditRole) && column != 0)
//            return QVariant(getSrcFnameOnly());
//        return QVariant();
//    }

//    bool SpriteContainer::insertChildrenNodes(int position, int count)
//    {
//        QMutexLocker lk(&getMutex());
//        if(position > m_spr.size())
//            return false;

//        for( int i = 0; i < count; ++i )
//            m_spr.insert(position, Sprite(this));
//        return true;
//    }

//    bool SpriteContainer::removeChildrenNodes(int position, int count)
//    {
//        QMutexLocker lk(&getMutex());
//        if(position + (count-1) > m_spr.size())
//            return false;
//        for(int i = 0; i < count; ++i)
//            m_spr.removeAt(position);
//        return true;
//    }

//    bool SpriteContainer::removeChildrenNodes(QModelIndexList indices)
//    {
//        bool success = true;
//        for(QModelIndex idx : indices)
//        {
//            if(!idx.isValid())
//                continue;
//            TreeNode * node = static_cast<TreeNode*>(idx.internalPointer());
//            Q_ASSERT(node);
//            success = (removeChildrenNodes(node->nodeIndex(), 1) && success);
//        }
//        return success;
//    }

//    bool SpriteContainer::moveChildrenNodes(int srcrow, int count, int destrow)
//    {
//        QMutexLocker lk(&getMutex());

//        if( srcrow + count > m_spr.size() || destrow > m_spr.size() )
//        {
//            Q_ASSERT(false);
//            return false;
//        }

//        if(destrow > srcrow)
//        {
//            for( int i = 0; i < count; ++i )
//                m_spr.move(srcrow, destrow);
//        }
//        else
//        {
//            for( int i = 0; i < count; ++i )
//                m_spr.move(srcrow, destrow + i);
//        }

//        return true;
//    }

bool SpriteContainer::nodeIsMutable() const
{
    return false;
}

QString SpriteContainer::nodeDisplayName() const
{
    return QString();
}

void SpriteContainer::FetchToC(QDataStream &/*fdat*/)
{

}

void SpriteContainer::LoadEntry(SpriteContainer::sprid_t /*idx*/)
{

}

QString SpriteContainer::GetContainerSrcFnameOnly() const
{
    return m_srcpath.mid( m_srcpath.lastIndexOf('/') );
}

const QString &SpriteContainer::GetContainerSrcPath() const
{
    return m_srcpath;
}

void SpriteContainer::SetContainerSrcPath(const QString &path)
{
    m_srcpath = path;
}

int SpriteContainer::GetNbDataColumns() const
{
    return 1;
}


//    QVariant SpriteContainer::data(const QModelIndex &index, int role) const
//    {
//        if(!ContainerLoaded() || m_spr.empty())
//            return QVariant();

//        if (!index.isValid())
//            return QVariant("root");

//        if (role != Qt::DisplayRole && role != Qt::EditRole)
//            return QVariant();

//        TreeNode *item = const_cast<SpriteContainer*>(this)->getItem(index);
//        return item->nodeData(index.column(), role);
//    }

//    QVariant SpriteContainer::headerData(int, Qt::Orientation, int) const
//    {
//        //nothing really
//        return QVariant();
//    }
QVariant SpriteContainer::GetContentData(const QModelIndex &index, int role) const
{
    if(!isContainerLoaded() || m_spr.empty())
        return QVariant();

    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const TreeNode * pnode = static_cast<TreeNode*>(index.internalPointer());
    return pnode->nodeDisplayName();
}

QVariant SpriteContainer::GetContentHeaderData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

bool SpriteContainer::isMultiItemContainer() const
{
    return ContainerIsPackFile();
}

const QString &SpriteContainer::GetTopNodeName() const
{
    return ElemName_Sprite;
}

bool SpriteContainer::canFetchMore(const QModelIndex &index) const
{
    if (!index.isValid() || !isContainerLoaded())
        return false;
    TreeNode * node = reinterpret_cast<TreeNode *>(index.internalPointer());
    if(node->nodeDataTypeName() != ElemName_Sprite)
        return false;
    Sprite * pspr = reinterpret_cast<Sprite *>(index.internalPointer());
    Q_ASSERT(pspr);
    if(!pspr)
        return false;
    return !pspr->wasParsed();
}

void SpriteContainer::fetchMore(const QModelIndex &index)
{
    if (!index.isValid() || !isContainerLoaded())
        return;
    TreeNode * node = reinterpret_cast<TreeNode *>(index.internalPointer());
    if(!node->nodeAllowFetchMore())
        return;
    Sprite * pspr = reinterpret_cast<Sprite *>(index.internalPointer());
    Q_ASSERT(pspr);
    if(!pspr)
        return;
    qDebug() << "Parsing sprite #"<<index.row() <<"!";
    ContentManager & manager = ContentManager::Instance();

    const QList<QPersistentModelIndex> changed{index};
    manager.layoutAboutToBeChanged(changed);
    pspr->ParseSpriteData();
    manager.layoutChanged(changed);
}

//    //Incremental load
//    void SpriteContainer::fetchMore(const QModelIndex &parent)
//    {
//        if (!parent.isValid() || !ContainerLoaded())
//            return;
//        TreeNode * pte = getItem(parent);
//        if(!pte || (pte && pte == this))
//            return;
//        pte->fetchMore(parent);
//    }

//    bool SpriteContainer::canFetchMore(const QModelIndex &parent) const
//    {
//        if (!parent.isValid() || !ContainerLoaded())
//            return false;
//        TreeNode * pte = const_cast<SpriteContainer*>(this)->getItem(parent);
//        if(!pte || (pte && pte == this))
//            return false;
//        return pte->canFetchMore(parent);
//    }

TreeNode *SpriteContainer::clone() const
{
    return new SpriteContainer(*this);
}

int SpriteContainer::indexOfChild(const TreeNode *ptr) const
{
    if(!ptr)
        return -1;
    Sprite * pSprite = static_cast<Sprite*>(const_cast<TreeNode *>(ptr));
    return m_spr.indexOf(pSprite);
}

eTreeElemDataType SpriteContainer::nodeDataTy() const
{
    return eTreeElemDataType::spritecontainer;
}

const QString &SpriteContainer::nodeDataTypeName() const
{
    return ElemName_SpriteContainer;
}

bool FileIsSpriteContainer(const QString & filepath)
{
    const QFileInfo target(filepath);
    if(target.exists())
    {
        const QString extension = target.suffix();
        if( extension == PackFileExt ||
            extension == WANFileExt ||
            extension == WATFileExt )
        {
            //#TODO: Do advanced tests
            return true;
        }
    }
    return false;
}
