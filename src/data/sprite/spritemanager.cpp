#if 0
#include "spritemanager.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/extfmt/riff_palette.hpp>
#include <src/extfmt/text_palette.hpp>
#include <src/extfmt/gpl_palette.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <QSaveFile>
#include <src/data/sprite/sprite_container.hpp>

const QString ContainerTypeNone = "None";

SpriteManager &SpriteManager::Instance()
{
    static SpriteManager s_manager;
    return s_manager;
}

SpriteManager::SpriteManager()
    :QAbstractItemModel(nullptr), m_container(nullptr)
{
}

SpriteManager::~SpriteManager()
{
    qDebug("SpriteManager::~SpriteManager(): Deleting sprite manager!\n");
    //Since the class will be deleted later by the model, release ownership here instead of deleting!
    //            delete m_container.take();
}

SpriteContainer * SpriteManager::OpenContainer(const QString &fname)
{
    m_container.reset(new SpriteContainer(fname));
    m_container->LoadContainer();
    m_container->Initialize();
    return m_container.data();
}

void SpriteManager::CloseContainer()
{
    qInfo("SpriteManager::CloseContainer(): Close container called!");
    Reset();
}

int SpriteManager::SaveContainer()
{
    if(!IsContainerLoaded())
        throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
    return SaveContainer(m_container->GetContainerSrcPath());
}

int SpriteManager::SaveContainer(const QString &fname)
{
    if(!IsContainerLoaded())
        throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
    m_container->SetContainerSrcPath(fname);
    return m_container->WriteContainer();
}

void SpriteManager::ExportContainer(const QString &fname)
{
    if(!IsContainerLoaded())
        throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
    //m_container->ExportContainer(fname, exportType);
    Q_ASSERT(false);
}

SpriteContainer * SpriteManager::ImportContainer(const QString &fname)
{
    m_container.reset(new SpriteContainer(fname));
    m_container->ImportContainer(fname);
    m_container->Initialize();
    return m_container.data();
}

SpriteContainer * SpriteManager::NewContainer(const QString & type)
{
    m_container.reset(new SpriteContainer());
    m_container->SetContainerType(type);
    m_container->Initialize();
    return m_container.data();
}

//int SpriteManager::GetNbSpritesInContainer() const
//{
//    if(m_container)
//        return m_container->nodeChildCount();
//    else
//        return 0;
//}

QString SpriteManager::GetContainerSrcFile() const
{
    if(m_container)
        return m_container->GetContainerSrcPath();
    else
        return QString();
}

const QString &SpriteManager::GetContainerType() const
{
    if(!m_container)
        return ContainerTypeNone;
    return m_container->GetContainerType();
}

void SpriteManager::Reset()
{
    m_container.reset();
}

//void SpriteManager::AddSpriteToContainer(Sprite &&spr)
//{
//    if(ContainerIsPackFile() && m_container)
//    {
//        SpriteContainer::sprid_t id = m_container->AddSprite();
//        qDebug() << "SpriteManager::AddSpriteToContainer(): Adding new sprite to slot#" <<id <<"!\n";
//        spr.setParentNode(m_container.data());
//        m_container->GetSprite(id) = qMove(spr);
//    }
//    else
//        qCritical() << "SpriteManager::AddSpriteToContainer() : Tried to add a sprite to a single sprite!!\n";
//}

//void SpriteManager::RemSpriteFromContainer(QModelIndex index)
//{
//    if(!m_container)
//    {
//        qWarning("No container loaded!\n");
//        return;
//    }
//    m_container->_deleteChildrenNodes(index.row(), 1); //#TODO: Change for a remove instead once undo stack is properly functional
//    //m_container->removeRows( index.row(), 1, QModelIndex(), this );
//}

//bool SpriteManager::DumpSprite(const QModelIndex &index, const QString & path)
//{
//    if(IsContainerLoaded())
//    {
//        Sprite * spr = static_cast<Sprite*>(m_container->GetSprite(index.row()));
//        if(!spr)
//        {
//            Q_ASSERT(false);
//            qWarning("SpriteManager::DumpSprite(): Index isn't a sprite! \n");
//            return false;
//        }

//        qDebug() <<"SpriteManager::DumpSprite(): Dumping sprite #" <<index.row() <<"!\n";
//        spr->CommitSpriteData();
//        QSaveFile save(path);
//        save.write( (char*)(spr->getRawData().data()), spr->getRawData().size());
//        return save.commit();
//    }
//    else
//        qCritical() << "SpriteManager::DumpSprite() : Tried to dump a sprite while the container was not loaded!\n";
//    return false;
//}

//void SpriteManager::DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type)
//{
//    if(!spr)
//        throw std::range_error("SpriteManager::DumpPalette(): Invalid sprite index!!");

//    std::vector<uint8_t> fdata;
//    std::vector<uint32_t> pal;
//    for(const QRgb & col : spr->getPalette())
//    {
//        pal.push_back(static_cast<uint32_t>(col));
//    }

//    switch(type)
//    {
//    case ePaletteDumpType::RIFF_Pal:
//        {
//            qDebug("Exporting RIFF Palette\n");
//            fdata = utils::ExportTo_RIFF_Palette(pal, utils::ARGBToComponents); //sice QRgb is ARGB, we use this decoder!
//            qDebug("Exporting RIFF Palette, conversion complete!\n");
//            break;
//        }
//    case ePaletteDumpType::TEXT_Pal:
//        {
//            fdata = utils::ExportPaletteAsTextPalette(pal, utils::ARGBToComponents);
//            break;
//        }
//    case ePaletteDumpType::GIMP_PAL:
//        {
//            fdata = utils::ExportGimpPalette(pal, utils::ARGBToComponents);
//            break;
//        }
//    default:
//        throw std::invalid_argument("SpriteManager::DumpPalette(): Invalid palette destination type!");
//    };

//    qDebug("Exporting RIFF Palette, Writing to file!\n");
//    QSaveFile sf(path);
//    if(!sf.open( QSaveFile::WriteOnly ))
//        throw std::runtime_error(QString("SpriteManager::DumpPalette(): Couldn't open file \"%1\" for writing!\n").arg(path).toStdString());

//    if( sf.write( (char*)fdata.data(), fdata.size() ) < static_cast<qint64>(fdata.size()) )
//        qWarning("SpriteManager::DumpPalette(): The amount of bytes written to file differs from the expected filesize!\n");

//    qDebug("Exporting RIFF Palette, written! Now commiting\n");

//    if(!sf.commit())
//        throw std::runtime_error(QString("SpriteManager::DumpPalette(): Commit to \"%1\" failed!\n").arg(path).toStdString());
//    qDebug("Exporting RIFF Palette, commited!\n");
//}

//void SpriteManager::DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type)
//{
//    if(!sprite.isValid())
//        throw std::invalid_argument("SpriteManager::DumpPalette(): Invalid sprite index!!");

//    Sprite * spr = static_cast<Sprite*>(m_container->getItem(sprite));
//    DumpPalette(spr, path, type);
//}

//void SpriteManager::ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type)
//{
//    if(!spr)
//        throw std::range_error("SpriteManager::ImportPalette(): Invalid sprite index!!");

//    std::vector<uint8_t> fdata = utils::ReadFileToByteVector(path.toStdString());

//    switch(type)
//    {
//    case ePaletteDumpType::RIFF_Pal:
//        {
//            //can't use move here since we need the implicit convertion on copy
//            std::vector<uint32_t> imported = utils::ImportFrom_RIFF_Palette(fdata, utils::RGBToARGB);
//            spr->setPalette(QVector<QRgb>(imported.begin(), imported.end())); //since QRgb is ARGB, we use this encoder!
//            break;
//        }
//    case ePaletteDumpType::TEXT_Pal:
//        {
//            //can't use move here since we need the implicit convertion on copy
//            std::vector<uint32_t> imported = utils::ImportPaletteAsTextPalette(fdata, utils::RGBToARGB);
//            spr->setPalette( QVector<QRgb>(imported.begin(), imported.end()) );
//            break;
//        }
//    case ePaletteDumpType::GIMP_PAL:
//        {
//            //can't use move here since we need the implicit convertion on copy
//            std::vector<uint32_t> imported = utils::ImportGimpPalette(fdata, utils::RGBToARGB);
//            spr->setPalette( QVector<QRgb>(imported.begin(), imported.end()) );
//            break;
//        }
//    case ePaletteDumpType::PNG_PAL:
//        {
//            //can't use move here since we need the implicit convertion on copy
//            QImage png(path, "png");
//            spr->setPalette(png.colorTable());
//            break;
//        }
//    default:
//        throw std::invalid_argument("SpriteManager::ImportPalette(): Invalid palette type!");
//    };

//}

//bool SpriteManager::ContainerIsPackFile()const
//{
//    if(m_container)
//        return m_container->ContainerIsPackFile();
//    return false;
//}

//bool SpriteManager::ContainerIsSingleSprite()const
//{
//    if(m_container)
//        return m_container->ContainerIsSingleSprite();
//    return false;
//}

QString SpriteManager::getContentShortName()const
{
    return GetContainerType();
}

//QStringList &SpriteManager::getAnimSlotNames()
//{
//    return m_animslotnames;
//}

//const QStringList &SpriteManager::getAnimSlotNames() const
//{
//    return m_animslotnames;
//}

SpriteContainer *SpriteManager::getContainer()
{
    return m_container.data();
}

void SpriteManager::ForceTreeModelRefrech() const
{

}

//Incremental load
Qt::ItemFlags SpriteManager::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !IsContainerLoaded())
        return QAbstractItemModel::flags(index);

    TreeNode * pnode = static_cast<TreeNode*>(index.internalPointer());
    if(!pnode)
        return QAbstractItemModel::flags(index);

    return pnode->nodeFlags();
}

QVariant SpriteManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !IsContainerLoaded() || index.column() != 0)
        return QVariant();
    return m_container->GetContentData(index, role);
}

QVariant SpriteManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(!IsContainerLoaded())
        return QVariant();
    return m_container->GetContentHeaderData(section, orientation, role);
}

int SpriteManager::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid() || !IsContainerLoaded())
        return 0;
    const TreeNode *parentItem = static_cast<const TreeNode*>(parent.internalPointer());
    Q_ASSERT(parentItem);
    return parentItem->nodeChildCount();
}

bool SpriteManager::hasChildren(const QModelIndex &parent) const
{
    if(!parent.isValid() || !IsContainerLoaded())
        return false;
    const TreeNode *parentItem = static_cast<const TreeNode*>(parent.internalPointer());
    Q_ASSERT(parentItem);
    return parentItem->nodeHasChildren();
}

TreeNode *SpriteManager::getItem(const QModelIndex &index) const
{
    if (index.isValid() || !IsContainerLoaded())
    {
        TreeNode *item = reinterpret_cast<TreeNode*>(index.internalPointer());
        return item;
    }
    return m_container.data();
}

QModelIndex SpriteManager::modelIndexOf(TreeNode *ptr) const
{
    if (!ptr || !IsContainerLoaded())
        return QModelIndex();
    TreeNode * parent = ptr->parentNode();
    return createIndex(parent->indexOfChild(ptr), 0, ptr);
}

QModelIndex SpriteManager::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() || !IsContainerLoaded())
        return QModelIndex();
    Q_ASSERT(column == 0);

    TreeNode *parentItem = static_cast<TreeNode*>(parent.internalPointer());
    Q_ASSERT(parentItem);
    TreeNode *childItem  = parentItem->nodeChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex SpriteManager::parent(const QModelIndex &index) const
{
    if (!IsContainerLoaded() || m_container->empty() || !index.isValid())
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

int SpriteManager::columnCount(const QModelIndex &/*parent*/) const
{
    //All element in this model have the same amount of columns
    if(!IsContainerLoaded())
        return 0;
    return m_container->GetNbDataColumns();
}

bool SpriteManager::removeRows(int position, int rows, const QModelIndex &/*parent*/)
{
    if (!IsContainerLoaded())
        return false;
    //changing the container handles updating the model in this case
    return m_container->_removeChildrenNodes(position, rows);
}

bool SpriteManager::insertRows(int position, int rows, const QModelIndex &/*parent*/)
{
    if (!IsContainerLoaded())
        return false;
    //changing the container handles updating the model in this case
    return m_container->_insertChildrenNodes(position, rows);
}

void SpriteManager::fetchMore(const QModelIndex &parent)
{
    if (!IsContainerLoaded())
        return;
    if(canFetchMore(parent))
        m_container->fetchMore(parent);
    else
    {
        qWarning() << "Couldn't fetchMore sprite #" << parent.row() <<"\n";
    }
}

bool SpriteManager::canFetchMore(const QModelIndex &parent) const
{
    if(!parent.isValid() || !IsContainerLoaded())
        return false;
    return m_container->canFetchMore(parent);
}

bool SpriteManager::IsContainerLoaded() const
{
    return (m_container != nullptr) && m_container->isContainerLoaded();
}


#endif
