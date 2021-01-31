#include "sprite_container.hpp"
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <QSaveFile>
#include <QtConcurrent>
#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <QFuture>
#include <list>
#include <QApplication>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/packfile.hpp>
#include <src/ui/windows/dialogprogressbar.hpp>
#include <src/utility/container_threaded_writer.hpp>
#include <src/data/content_factory.hpp>
#include <src/data/content_manager.hpp>
#include <src/data/contents_selection_manager.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/fmts/file_format_identifier.hpp>
#include <src/data/sprite/unknown_item.hpp>

const QString NullString;
const QString PackFileExt = "bin";
const QString WANFileExt  = "wan";
const QString WATFileExt  = "wat";

const QString ElemName_SpriteContainer = "Container";
const QString ContentName_Sprite = "sprite";

bool FileIsSpriteContainer(const QString & filepath);

//Define the container as a handler for sprite containers
DEFINE_CONTAINER(SpriteContainer, ContentName_Sprite, FileIsSpriteContainer, ContentName_Sprite);

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

//=================================================================================================
//  SpriteContainer
//=================================================================================================
//const QList<QString> SpriteContainer::SpriteContentCategories=
//{
//    "Images",
//    "Frames",
//    "Sequences",
//    "Animations",
//    "Palette",
//    "Attachment Offsets",
//};

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
    m_nodes.reserve(cp.m_nodes.size());
    Q_FOREACH(TreeNode * spr, m_nodes)
        m_nodes.push_back(spr->clone());
    return *this;
}
SpriteContainer& SpriteContainer::operator=(SpriteContainer && mv)
{
    setParent(mv.parent());
    TreeNode::operator=(mv);
    m_srcpath   = mv.m_srcpath;
    m_nodes     = qMove(mv.m_nodes);
    mv.m_nodes.clear();
    m_cntTy     = mv.m_cntTy;
    return *this;
}

void SpriteContainer::Initialize()
{
    if(isContainerLoaded())
        return; //Don't do anything when we're already filed up!
    if(isContainerSingleSprite())
    {
        //We want to add a single empty sprite for a single sprite!
        AddSprite();
    }
    connect(this, &SpriteContainer::compressionChanged, &ContentManager::Instance(), &ContentManager::contentChanged);
}

bool SpriteContainer::isContainerLoaded() const
{
    return !m_nodes.empty() || (m_cntTy != eContainerType::NONE);
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
    qDeleteAll(m_nodes);
}

bool SpriteContainer::isContainerPackFile() const
{
    return isContainerLoaded() && m_cntTy == eContainerType::PACK;
}

bool SpriteContainer::isContainerSingleSprite() const
{
    return !isContainerPackFile();
}

const QString &SpriteContainer::GetContainerType() const
{
    switch(GetContainerTypeEnum())
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

SpriteContainer::eContainerType SpriteContainer::GetContainerTypeEnum() const
{
    return m_cntTy;
}

void SpriteContainer::SetContainerType(const QString &newtype)
{
    if(newtype == PackFileExt)
        SetContainerType(eContainerType::PACK);
    else if(newtype == WANFileExt)
        SetContainerType(eContainerType::WAN);
    else if(newtype == WATFileExt)
        SetContainerType(eContainerType::WAT);
    else
    {
        throw std::runtime_error("SpriteContainer::SetContainerType(): Unknown container type!");
    }
}

void SpriteContainer::SetContainerType(SpriteContainer::eContainerType newtype)
{
    m_cntTy = newtype;
}

//void SpriteContainer::SetContainerType(SpriteContainer::eContainerType ty){m_cntTy = ty;}
//SpriteContainer::eContainerType SpriteContainer::GetContainerType() const           {return m_cntTy;}

void SpriteContainer::LoadContainer()
{
    using namespace std::chrono_literals;
    QFile container(m_srcpath);

    if( !container.open(QIODevice::ReadOnly) || !container.exists() || container.error() != QFileDevice::NoError )
    {
        //Error can't load file!
        const QString error = container.errorString().toLocal8Bit();
        qWarning() << error;
        QMessageBox msgBox;
        msgBox.setText("Failed to load file!");
        msgBox.setInformativeText(error);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QByteArray data = container.readAll();
    ContentManager & manager = ContentManager::Instance();
    fmt::eSpriteType sprCntType = fmt::eSpriteType::INVALID;
    eCompressionFmtOptions sprComp = eCompressionFmtOptions::NONE;

    m_nodes.clear();
    //Lets identify the format
    if(m_srcpath.endsWith(PackFileExt))
    {
        fmt::PackFileLoader ldr;
        ldr.Read(data.begin(), data.end());
        m_nodes.reserve(ldr.size());

        //Load the raw data into each sprites but don't parse them yet!
        manager.beginInsertRows( QModelIndex(), 0, ldr.size() - 1);
        for( size_t cnt = 0; cnt < ldr.size(); ++cnt )
        {
            std::vector<uint8_t> rawfile;
            rawfile.reserve(ldr.GetEntryInfo(cnt).length);
            ldr.CopyEntryData(cnt, std::back_inserter(rawfile));

            TreeNode * tn = nullptr;
            //Check what kind of data it is
            if(fmt::DataIsSprite(rawfile.begin(), rawfile.end()))
            {
                //Make it into a sprite
                Sprite * spr = new Sprite(this, std::move(rawfile));
                tn = spr;
            }
            else
            {
                //Otherwise put it in a unknown item container
                UnknownItemNode * unk = new UnknownItemNode(this, std::move(rawfile));
                tn = unk;
            }

            if(tn)
                m_nodes.push_back(tn);
        }
        manager.endInsertRows();

        m_cntTy = eContainerType::PACK;
    }
    else if( m_srcpath.endsWith(WANFileExt) )
    {
        std::vector<uint8_t> rawfile(data.begin(), data.end());
        //We load the whole sprite
        manager.beginInsertRows( QModelIndex(), 0, 0);
        m_nodes.push_back(new Sprite(this, std::move(rawfile)));
        manager.endInsertRows();
        m_cntTy = eContainerType::WAN;
    }
    else if( m_srcpath.endsWith(WATFileExt) )
    {
        std::vector<uint8_t> rawfile(data.begin(), data.end());
        //We load the whole sprite
        manager.beginInsertRows( QModelIndex(), 0, 0);
        m_nodes.push_back(new Sprite(this, std::move(rawfile)));
        manager.endInsertRows();
        m_cntTy = eContainerType::WAT;
    }

    //Setup expected container wide formats
    if(!m_nodes.empty())
    {
        Sprite * firstspr = nullptr; //Get first sprite parsed so we can get the compression and sprite type for the container
        Q_FOREACH(TreeNode * node, m_nodes)
        {
            if(node->nodeDataTy() == eTreeElemDataType::sprite)
            {
                Sprite * spr = static_cast<Sprite*>(node);
                if(spr->canParse() && !spr->wasParsed())
                    spr->ParseSpriteData();
                if(!firstspr)
                    firstspr = spr;
            }
//            else if(node->nodeDataTypeName() == ElemName_UnknownItem)
//            {
//            }
//            else
//                Q_ASSERT(false);

        }

        if(firstspr)
        {
            sprComp = CompFmtToCompOption(firstspr->getTargetCompression());
            sprCntType = firstspr->type();
        }
    }
    m_cntsprty = sprCntType;
    m_cntCompression = sprComp;
}

int SpriteContainer::WriteContainer()const
{
    QScopedPointer<QSaveFile> pcontainer(new  QSaveFile(m_srcpath));

    if( !pcontainer->open(QIODevice::WriteOnly) || pcontainer->error() != QFileDevice::NoError )
    {
        //Error can't write file!
        const QString error = pcontainer->errorString().toLocal8Bit();
        qWarning() << error;
        QMessageBox msgBox;
        msgBox.setText("Failed to write file!");
        msgBox.setInformativeText(error);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return 0;
    }

    ContainerThreadedWriter * pmthw = new ContainerThreadedWriter(pcontainer.take(), ContentManager::Instance().getContainer());
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
    Q_ASSERT(false); //#TODO: Need to be done!
}

void SpriteContainer::ExportContainer(const QString &/*path*/, const QString & /*exportype*/) const
{
    Q_ASSERT(false); //#TODO: Need to be done!
}

Sprite* SpriteContainer::GetSprite(SpriteContainer::sprid_t idx)
{
    return (m_nodes[idx]->nodeDataTy() == eTreeElemDataType::sprite) ? static_cast<Sprite*>(m_nodes[idx]) : nullptr;
}

SpriteContainer::sprid_t SpriteContainer::AddSprite()
{
    return AddSprite(m_nodes.size());
}

SpriteContainer::sprid_t SpriteContainer::AddSprite(SpriteContainer::sprid_t idx)
{
    ContentManager & manager = ContentManager::Instance();
    manager.beginInsertRows(QModelIndex(), idx, idx);
    Sprite * spr = new Sprite(this);
    spr->convertSpriteToType(getExpectedSpriteType());
    spr->setTargetCompression(CompOptionToCompFmt(GetExpectedCompression()));
    m_nodes.push_back(spr);
    manager.endInsertRows();
    return idx;
}

void SpriteContainer::RemSprite()
{
    _removeChildrenNode(m_nodes.back(), false);
}

void SpriteContainer::RemSprites(const QModelIndexList &remove)
{
    QList<TreeNode*> nodes;
    Q_FOREACH(const QModelIndex & idx, remove)
        nodes.push_back(static_cast<TreeNode*>(idx.internalPointer()));
    _removeChildrenNodes(nodes, false);
}

//SpriteContainer::sprid_t SpriteContainer::AddSprite()
//{
//    ContentManager & manager = ContentManager::Instance();
//    size_t offset = m_spr.size();

//    manager.beginInsertRows( QModelIndex(), offset, offset );
//    Sprite * spr = new Sprite(this);
//    spr->convertSpriteToType(getExpectedSpriteType());
//    spr->setTargetCompression(CompOptionToCompFmt(GetExpectedCompression()));
//    m_spr.push_back(spr);
//    manager.endInsertRows();

//    return offset;
//}

eCompressionFmtOptions SpriteContainer::GetExpectedCompression() const
{
    return m_cntCompression;
}

void SpriteContainer::SetExpectedCompression(eCompressionFmtOptions compression)
{
    m_cntCompression = compression;
    forEachSprites([&, this](Sprite* spr){spr->setTargetCompression(CompOptionToCompFmt(m_cntCompression));});
    emit compressionChanged(m_cntCompression);
}

fmt::eSpriteType SpriteContainer::getExpectedSpriteType() const
{
    return m_cntsprty;
}

void SpriteContainer::setExpectedSpriteType(fmt::eSpriteType sprty)
{
    m_cntsprty = sprty;
    forEachSprites([&, this](Sprite* spr){spr->convertSpriteToType(m_cntsprty);});
}

QMenu *SpriteContainer::MakeActionMenu(QWidget *parent)
{
    return new SpriteContainerMenu(this, parent);
}

SpriteContainer::iterator SpriteContainer::begin()
{
    return m_nodes.begin();
}

SpriteContainer::const_iterator SpriteContainer::begin() const
{
    return m_nodes.begin();
}

SpriteContainer::iterator SpriteContainer::end()
{
    return m_nodes.end();
}

SpriteContainer::const_iterator SpriteContainer::end() const
{
    return m_nodes.end();
}

bool SpriteContainer::empty() const
{
    return m_nodes.empty();
}

SpriteContainer::rawdata_iterator SpriteContainer::getItemRawDataBeg(TreeNode * node)
{
    if(node->nodeDataTy() == eTreeElemDataType::sprite)
    {
        return static_cast<Sprite*>(node)->getRawData().begin();
    }
    else if(node->nodeDataTy() == eTreeElemDataType::unknown_item)
    {
        return static_cast<UnknownItemNode*>(node)->raw().begin();
    }
    return SpriteContainer::rawdata_iterator();
}

SpriteContainer::rawdata_iterator SpriteContainer::getItemRawDataEnd(TreeNode * node)
{
    if(node->nodeDataTy() == eTreeElemDataType::sprite)
    {
        return static_cast<Sprite*>(node)->getRawData().end();
    }
    else if(node->nodeDataTy() == eTreeElemDataType::unknown_item)
    {
        return static_cast<UnknownItemNode*>(node)->raw().end();
    }
    return SpriteContainer::rawdata_iterator();
}

SpriteContainer::rawdata_const_iterator SpriteContainer::getItemRawDataBeg(const TreeNode * node) const
{
    return const_cast<SpriteContainer*>(this)->getItemRawDataBeg(const_cast<TreeNode*>(node));
}

SpriteContainer::rawdata_const_iterator SpriteContainer::getItemRawDataEnd(const TreeNode * node) const
{
    return const_cast<SpriteContainer*>(this)->getItemRawDataEnd(const_cast<TreeNode*>(node));
}

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

bool SpriteContainer::_insertChildrenNode(TreeNode *node, int destrow)
{
    ContentManager & manager = ContentManager::Instance();
    manager.beginInsertRows(QModelIndex(), destrow, destrow);
    m_nodes.insert(destrow, node);
    manager.endInsertRows();
    return true;
}

bool SpriteContainer::_insertChildrenNodes(int row, int count)
{
    if(row < 0 || row > m_nodes.size())
        return false;
    //ContentManager & manager = ContentManager::Instance();
    //manager.beginInsertRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
        _insertChildrenNode(new Sprite(this), row + i);
    //manager.endInsertRows();
    return true;
}

bool SpriteContainer::_insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow)
{
    if(destrow < 0 || destrow > m_nodes.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginInsertRows(QModelIndex(), destrow, destrow + nodes.size() - 1);
    for(int i = 0; i < nodes.size(); ++i)
        m_nodes.insert(destrow + i, nodes[i]);
    manager.endInsertRows();
    return true;
}

bool SpriteContainer::_removeChildrenNode(TreeNode *node)
{
    return _removeChildrenNode(node, false);
}

bool SpriteContainer::_removeChildrenNode(TreeNode *node, bool bdeleteptr)
{
    ContentManager & manager = ContentManager::Instance();
    int pos = node->nodeIndex();
    manager.beginRemoveRows(QModelIndex(), pos, pos);
    m_nodes.removeAt(pos);
    if(bdeleteptr)
        delete node;
    manager.endRemoveRows();
    return true;
}

bool SpriteContainer::_removeChildrenNodes(int row, int count)
{
    if(row < 0 || row >= m_nodes.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
        m_nodes.removeAt(row + i);
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
    if(nodes.size() > m_nodes.size())
        return false;
    //bool success = true;
    //ContentManager & manager = ContentManager::Instance();
    //QModelIndexList origindices    = manager.persistentIndexList();
    //QModelIndexList changedindices = origindices;

    //manager.layoutAboutToBeChanged();

    Q_FOREACH(TreeNode* pnode, nodes)
    {
        if(!_removeChildrenNode(pnode, bdeleteptr))
            return false;
    }

//    //Clear out the changed ones and removed the nodes first
//    for(int i = 0; i < changedindices.size(); ++i)
//    {
//        QModelIndex & curidx = changedindices[i];
//        int found = nodes.indexOf(static_cast<TreeNode*>(curidx.internalPointer()));
//        if(found < 0)
//            continue;
//        //delete it from the table
//        Sprite * pspr = static_cast<Sprite*>(curidx.internalPointer());
//        m_spr.removeOne(pspr);
//        if(bdeleteptr)
//            delete pspr;
//        changedindices.removeAt(i); //Clear from changed indices
//        origindices.removeAt(i);    //Clear from changed indices
//    }

//    //Iterated the changed indices and find their new indices by pointer
//    for(QModelIndex & idx : changedindices)
//    {
//        if(!idx.isValid())
//            continue;
//        int newrow = m_spr.indexOf(static_cast<Sprite*>(idx.internalPointer()));
//        if(newrow < 0)
//        {
//            Q_ASSERT(false);
//            throw std::runtime_error("SpriteContainer::_removeChildrenNodes(): Found non-existent node pointer when updating persistent index list!");
//        }
//        idx = manager.index(newrow, 0);
//    }

    //Update persistent indices
//    manager.changePersistentIndexList(origindices, changedindices);
//    manager.layoutChanged();
    return true;
}

bool SpriteContainer::_deleteChildrenNode(TreeNode *node)
{
    return _removeChildrenNode(node, true);
}

bool SpriteContainer::_deleteChildrenNodes(int row, int count)
{
    if(row < 0 || row >= m_nodes.size())
        return false;
    ContentManager & manager = ContentManager::Instance();
    manager.beginRemoveRows(QModelIndex(), row, row + count - 1);
    for(int i = 0; i < count; ++i)
    {
        delete m_nodes[row + i];
        m_nodes.removeAt(row + i);
    }
    manager.endRemoveRows();
    return true;
}

bool SpriteContainer::_deleteChildrenNodes(const QList<TreeNode *> &nodes)
{
    return _removeChildrenNodes(nodes, true);
}

bool SpriteContainer::_moveChildrenNodes(const QModelIndexList &indices, int destrow, QModelIndex destparent)
{
    QList<TreeNode*> tomove;
    Q_FOREACH(const QModelIndex & idx, indices)
    {
        tomove.push_back(static_cast<TreeNode*>(idx.internalPointer()));
    }
    return _moveChildrenNodes(tomove, destrow, destparent);
}

//#TODO: This could be greatly improved in terms of performances. Just needed something that would work for now!
bool SpriteContainer::_moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent)
{
    ContentManager & manager = ContentManager::Instance();
    TreeNode* destnode = static_cast<TreeNode*>(destparent.internalPointer());
    if(nodes.size() > destnode->nodeChildCount() || destrow > destnode->nodeChildCount())
    {
        return false;
    }

    int cntinsert = 0;
    Q_FOREACH(TreeNode * pnode, nodes)
    {
        int removedidx = pnode->nodeIndex();
        if(!manager.beginMoveRows(QModelIndex(), removedidx, removedidx, destparent, destrow + cntinsert))
            return false;
        m_nodes.removeAt(removedidx);
        bool hasfailed = !destnode->_insertChildrenNode(pnode, destrow + cntinsert);
        manager.endMoveRows();
        if(hasfailed)
            return false;
        ++cntinsert;
    }
    return true;
}

bool SpriteContainer::_moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode)
{
    if(!destnode || ((row + count - 1) >= m_nodes.size()) || (destrow > destnode->nodeChildCount()))
        return false;
    ContentManager &     manager = ContentManager::Instance();
    QList<TreeNode*>    tomove;
    tomove.reserve(count);

    //manager.beginRemoveRows(QModelIndex(), row, row + (count - 1));
    for(int i = 0; i < count; ++i)
    {
        tomove.push_back(m_nodes[row]);
        //m_spr.removeAt(row);
    }
    //manager.endRemoveRows();

    return _moveChildrenNodes(tomove, destrow, manager.index(destnode->nodeIndex(), 0));
    //return success? success & destnode->_insertChildrenNodes(moved, destrow) : false;
}

void SpriteContainer::appendChild(TreeNode *item)
{
    m_nodes.append(item);
}

TreeNode *SpriteContainer::nodeChild(int row)
{
    if(m_nodes.empty() || row >= m_nodes.size())
        return nullptr;
    else
        return m_nodes.at(row); //QList at() avoids checking for writing
}

int SpriteContainer::nodeChildCount() const
{
    return m_nodes.count();
}

int SpriteContainer::nodeIndex() const
{
    return 0; //Always first!
}

TreeNode *SpriteContainer::parentNode()
{
    return m_parentItem;
}

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

void SpriteContainer::forEachSprites(std::function<void (Sprite *)> &&fun)
{
    Q_FOREACH(auto * node, m_nodes)
    {
        if(node->nodeDataTy() == eTreeElemDataType::sprite)
            fun(static_cast<Sprite*>(node));
    }
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

QVariant SpriteContainer::GetContentData(const QModelIndex &index, int role) const
{
    if(!isContainerLoaded() || m_nodes.empty())
        return QVariant();

    if (!index.isValid())
        return QVariant("root");

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        const TreeNode * pnode = static_cast<TreeNode*>(index.internalPointer());
        return pnode->nodeDisplayName();
    }
    else if(role == Qt::DecorationRole)
    {
        const TreeNode * pnode = static_cast<TreeNode*>(index.internalPointer());
        return pnode->nodeDecoration();
    }
    return QVariant();
}

QVariant SpriteContainer::GetContentHeaderData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
    return QVariant();
}

bool SpriteContainer::isMultiItemContainer() const
{
    return isContainerPackFile();
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
    return node->nodeAllowFetchMore() && !node->nodeCanFetchMore();

//    Sprite * pspr = reinterpret_cast<Sprite *>(index.internalPointer());
//    Q_ASSERT(pspr);
//    if(!pspr)
//        return false;
//    return pspr->canParse() && !pspr->wasParsed();
}

void SpriteContainer::fetchMore(const QModelIndex &index)
{
    if (!index.isValid() || !isContainerLoaded())
        return;
    TreeNode * node = reinterpret_cast<TreeNode *>(index.internalPointer());
    if(!node->nodeAllowFetchMore() || node->nodeDataTy() != eTreeElemDataType::sprite)
        return;
    Sprite * pspr = reinterpret_cast<Sprite *>(index.internalPointer());
    Q_ASSERT(pspr);
    if(!pspr)
        return;
    qDebug() << "Parsing sprite #"<<index.row() <<"!";
    ContentManager & manager = ContentManager::Instance();

    const QList<QPersistentModelIndex> changed{index};
    emit manager.layoutAboutToBeChanged(changed);
    if(pspr->canParse() && !pspr->wasParsed())
        pspr->ParseSpriteData();
    emit manager.layoutChanged(changed);
}

TreeNode *SpriteContainer::clone() const
{
    return new SpriteContainer(*this);
}

int SpriteContainer::indexOfChild(const TreeNode *ptr) const
{
    if(!ptr)
        return -1;
    TreeNode * node = const_cast<TreeNode*>(ptr);
    return m_nodes.indexOf(node);
}

eTreeElemDataType SpriteContainer::nodeDataTy() const
{
    return eTreeElemDataType::spritecontainer;
}

const QString &SpriteContainer::nodeDataTypeName() const
{
    return ElemName_SpriteContainer;
}

//=================================================================================================
// SpriteContainerMenu
//=================================================================================================
SpriteContainerMenu::SpriteContainerMenu(SpriteContainer * container, QWidget * parent)
    :QMenu(parent)
{
    m_container = container;
    InitContent();
}

SpriteContainerMenu::~SpriteContainerMenu()
{
}

const char * PROP_COMPRESSION_FMT {"compression_fmt"};

void SpriteContainerMenu::InitContent()
{
    if(m_container->isMultiItemContainer())
        setTitle(tr("Sprite pack"));
    else
        setTitle(tr("Sprite"));

    //Sprite Adding/Removal
    if(m_container->isMultiItemContainer())
    {
        addAction(tr("&Add sprite.."),      this, &SpriteContainerMenu::OnAddSprite);
        addAction(tr("&Remove sprite.."),   this, &SpriteContainerMenu::OnRemSprite);
    }

    //Compression select
    addSeparator()->setText(tr("Compression"));

    m_actionGrp.reset(new QActionGroup(this));
    m_actionGrp->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

    //Fill up compression options
    for(int i = 0; i < CompressionFmtOptions.size(); ++i)
    {
        QAction * paction = addAction(CompressionFmtOptions[i]);
        paction->setCheckable(true);
        m_actionGrp->addAction(paction);
        if(m_container->GetExpectedCompression() == static_cast<eCompressionFmtOptions>(i))
            paction->setChecked(true);
        paction->setProperty(PROP_COMPRESSION_FMT, i);
        addAction(paction);
    }

    connect(m_actionGrp.data(), &QActionGroup::triggered,              this, &SpriteContainerMenu::OnCompressionChanged);
    connect(m_container,        &SpriteContainer::compressionChanged,  this, &SpriteContainerMenu::updateMenu);
}

void SpriteContainerMenu::updateMenu()
{
    //Helper to block signals and unblock via RAII (Avoid signal staying stuck in case of exception)
    struct autoblock
    {
        QActionGroup * ag;
        autoblock(QActionGroup * _ag):ag(_ag)   {ag->blockSignals(true); Q_FOREACH(QAction * act, ag->actions()){act->blockSignals(true);}}
        ~autoblock()                            {ag->blockSignals(false);Q_FOREACH(QAction * act, ag->actions()){act->blockSignals(false);}}
    } ab(m_actionGrp.data());

    //Sync the radio button with the container's compression
    eCompressionFmtOptions  curopt  = m_container->GetExpectedCompression();
    QList<QAction*>         opt     = m_actionGrp->actions();
    for(int i = 0; i < opt.size(); ++i)
    {
        if(opt[i]->property(PROP_COMPRESSION_FMT).value<int>() == static_cast<int>(curopt) && !opt[i]->isChecked())
            opt[i]->setChecked(true);
    }
}

void SpriteContainerMenu::OnCompressionChanged(QAction * selected)
{
    eCompressionFmtOptions cmpfmt = static_cast<eCompressionFmtOptions>(selected->property(PROP_COMPRESSION_FMT).value<int>());
    m_container->SetExpectedCompression(cmpfmt);
}

void SpriteContainerMenu::OnAddSprite()
{
    QItemSelectionModel * model = ContentsSelectionManager::SelectionModel();
    if(model->hasSelection())
        m_container->AddSprite(model->selectedRows().last().row()); //Add after last selected item
    else if(model->currentIndex().isValid())
        m_container->AddSprite(model->currentIndex().row());
    else
        m_container->AddSprite();
}

void SpriteContainerMenu::OnRemSprite()
{
    QItemSelectionModel * model = ContentsSelectionManager::SelectionModel();
    if(model->hasSelection())
        m_container->RemSprites(model->selectedRows());
    else if(model->currentIndex().isValid())
        m_container->RemSprites(QModelIndexList{model->currentIndex()});
    else
        m_container->RemSprite();
}
