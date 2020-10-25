#include "images_list_model.hpp"
#include <src/data/sprite/image.hpp>
#include <src/data/sprite/imagescontainer.hpp>
#include <src/data/sprite/sprite.hpp>

//
//
//
//ImagesManager::ImagesManager(ImageContainer *parent)
//    :QAbstractItemModel(), m_parentcnt(parent)
//{}

//ImagesManager::~ImagesManager()
//{
//    qDebug("ImagesManager::~ImagesManager()\n");
//}

//QModelIndex ImagesManager::index(int row, int column, const QModelIndex &parent) const
//{
//    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
//    TreeElement *childItem  = parentItem->nodeChild(row);
//    if (childItem)
//        return createIndex(row, column, childItem);
//    else
//        return QModelIndex();
//}

//QModelIndex ImagesManager::parent(const QModelIndex &child) const
//{
//    TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
//    TreeElement *parentItem = childItem->parentNode();
//    Q_ASSERT(parentItem != nullptr);

//    if (parentItem == m_parentcnt)
//        return QModelIndex();

//    return createIndex(parentItem->nodeIndex(), 0, parentItem);
//}

//int ImagesManager::rowCount(const QModelIndex &parent) const
//{
//    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
//    return parentItem->nodeChildCount();
//}

//int ImagesManager::columnCount(const QModelIndex &parent) const
//{
//    if (parent.isValid())
//        return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
//    else
//        return m_parentcnt->nodeColumnCount();
//}

//bool ImagesManager::hasChildren(const QModelIndex &parent) const
//{
//    TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
//    if(parentItem)
//        return parentItem->nodeChildCount() > 0;
//    else
//        return false;
//}

//QVariant ImagesManager::data(const QModelIndex &index, int role) const
//{
//    Q_ASSERT(hasChildren(QModelIndex()));
//    if (!index.isValid())
//        return QVariant("root");

//    if (role != Qt::DisplayRole &&
//        role != Qt::DecorationRole &&
//        role != Qt::SizeHintRole &&
//        role != Qt::EditRole)
//        return QVariant();

//    Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
//    return img->imgData(index.column(), role);
//}


//bool ImagesManager::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    Q_ASSERT(hasChildren(QModelIndex()));
//    if (!index.isValid())
//        return false;

//    if (role != Qt::EditRole)
//        return false;

//    Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
//    return img->setImgData(index.column(), value, role);
//}

//QVariant ImagesManager::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//    {
//        return QVariant( QString("%1").arg(section) );
//    }
//    else if( orientation == Qt::Orientation::Horizontal )
//    {
//        switch(section)
//        {
//        case 0:
//            return QVariant( QString("") );
//        case 1:
//            return QVariant( QString("Bit Depth") );
//        case 2:
//            return QVariant( QString("Resolution") );
//        };
//    }
//    return QVariant();
//}

//bool ImagesManager::insertRows(int row, int count, const QModelIndex &parent)
//{
//    TreeElement *parentItem = getItem(parent);
//    bool success;

//    beginInsertRows(parent, row, row + count - 1);
//    success = parentItem->insertChildrenNodes(row, count);
//    endInsertRows();

//    return success;
//}

//bool ImagesManager::removeRows(int row, int count, const QModelIndex &parent)
//{
//    TreeElement *parentItem = getItem(parent);
//    bool success = true;

//    beginRemoveRows(parent, row, row + count - 1);
//    success = parentItem->removeChildrenNodes(row, count);
//    endRemoveRows();

//    return success;
//}

//bool ImagesManager::moveRows(const QModelIndex &/*sourceParent*/, int /*sourceRow*/, int /*count*/, const QModelIndex &/*destinationParent*/, int /*destinationChild*/)
//{
//    Q_ASSERT(false);
//    return false;
//}

//TreeElement *ImagesManager::getItem(const QModelIndex &index)
//{
//    if (index.isValid())
//    {
//        TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
//        if (item)
//            return item;
//    }
//    return m_parentcnt;
//}


ImageListModel::ImageListModel(ImageContainer* parent, Sprite* psprite)
    :TreeNodeModel(nullptr), m_root(parent), m_sprite(psprite)
{
}

ImageListModel::~ImageListModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

//int ImageListModel::rowCount(const QModelIndex &/*parent*/) const
//{
////    if(!parent.isValid())
////        return m_root->nodeChildCount();
//    //return static_cast<TreeNode*>(parent.internalPointer())->nodeChildCount();
//    return m_root->nodeChildCount();
//}

int ImageListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return static_cast<int>(Image::eColumnType::NbColumns);
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
//    if (!index.isValid())
//        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const Image *img = static_cast<const Image*>(getItem(index));
    QVariant res;

    switch(static_cast<Image::eColumnType>(index.column()))
    {
        case Image::eColumnType::Preview: //preview
        {
            if( role == Qt::DecorationRole )
                res.setValue(img->makeImage(getOwnerSprite()->getPalette()));
            else if( role == Qt::SizeHintRole )
            {
                QSize imgsz = img->getImageSize();
                imgsz.scale(32, 32, Qt::AspectRatioMode::KeepAspectRatio);
                res = imgsz;
            }
            break;
        }
        case Image::eColumnType::UID:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getImageUID());
            break;
        }
        case Image::eColumnType::Depth: //depth
        {
            if( role == Qt::DisplayRole )
                res.setValue(QString("%1bpp").arg(img->getImageOriginalDepth()));
            break;
        }
        case Image::eColumnType::Resolution: //resolution
        {
            if( role == Qt::DisplayRole )
            {
                const QSize imgsz = img->getImageSize();
                res.setValue(QString("%1x%2").arg(imgsz.width()).arg(imgsz.height()));
            }
            break;
        }
        case Image::eColumnType::direct_Unk2:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getUnk2());
            break;
        }
        case Image::eColumnType::direct_Unk14:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getUnk14());
            break;
        }
        default:
            break; //Get rid of warnings
    };
    return res;
}

bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(hasChildren(QModelIndex()));
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    Image *img = static_cast<Image*>(getItem(index));
    bool  succ = false;

    switch(static_cast<Image::eColumnType>(index.column()))
    {
        case Image::eColumnType::direct_Unk2:
        {
            using unk2t = decltype(img->getUnk2());
            unk2t res = static_cast<unk2t>(value.toUInt(&succ));
            if(succ)
                img->setUnk2(res);
            break;
        }
        case Image::eColumnType::direct_Unk14:
        {
            using unk14t = decltype(img->getUnk14());
            unk14t res = static_cast<unk14t>(value.toUInt(&succ));
            if(succ)
                img->setUnk14(res);
            break;
        }
        case Image::eColumnType::Preview:   [[fallthrough]]; //preview
        case Image::eColumnType::UID:       [[fallthrough]];
        case Image::eColumnType::Depth:     [[fallthrough]]; //depth
        case Image::eColumnType::Resolution:[[fallthrough]]; //resolution
        default: //To get rid of warnings
            succ = false;
            break;
    };

    emit dataChanged(index, index, {role});
    return succ;
}

QVariant ImageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return QVariant( QString("%1").arg(section) );
    }
    else if( orientation == Qt::Orientation::Horizontal )
    {
        switch(static_cast<Image::eColumnType>(section))
        {
        case Image::eColumnType::Preview:
            return QVariant( QString("") );
        case Image::eColumnType::UID:
            return QVariant( QString("UID") );
        case Image::eColumnType::Depth:
            return QVariant( QString("Bit Depth") );
        case Image::eColumnType::Resolution:
            return QVariant( QString("Resolution") );
        default:
            return QVariant(); //Avoid warning for unhandled enum values
        };
    }
    return QVariant();
}

TreeNodeModel::node_t *ImageListModel::getRootNode()
{
    return m_root;
}

Sprite *ImageListModel::getOwnerSprite()
{
    return m_sprite;
}

QVariant ImageListModelCondensed::data(const QModelIndex &index, int role) const
{
    const Image* img = static_cast<const Image*>(getItem(index));
    QVariant res;
    if( role == Qt::DecorationRole )
        res.setValue(img->makeImage(getOwnerSprite()->getPalette()));
    else if( role == Qt::DisplayRole )
    {
        QSize resolution = img->getImageSize();
        QString condensed = "ID:%1 %2bpp %3x%4";
        condensed = condensed
                .arg(img->nodeIndex())
                .arg(img->getImageOriginalDepth())
                .arg(resolution.width())
                .arg(resolution.height());
        res = condensed;
    }
    else if(role == Qt::EditRole)
        res.setValue(img->nodeIndex());
    return res;
}
