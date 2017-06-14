#include "sprite_img.hpp"
#include <QImage>
#include <QVariant>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPixmap>
#include <QFontMetrics>

#include <src/sprite.h>

//
//
//
const size_t               FramesHeaderNBColumns = static_cast<unsigned int>(eFramesColumnsType::HeaderNBColumns);
const std::vector<QString> FramesHeaderColumnNames
{
    QString(("")),

    //The rest below is for the parts/step to assemble the frame!
    QString(("Img ID")),
    QString(("Unk#0")),
    QString(("Offset")),
    QString(("Flip")),
    QString(("Rotation & Scaling")),
    QString(("Palette")),
    QString(("Priority")),
    QString(("Tile ID")),
};

const char * MFrame::PropPartID = "framePartID";


Sprite * EffectOffsetContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

//
//
//


void ImageContainer::importImages(const fmt::ImageDB::imgtbl_t &imgs, const fmt::ImageDB::frmtbl_t &frms)
{
    static const int TileWidth  = 8;
    static const int TileHeight = 8;
    static const int TileLength = 64;
    removeChildrenNodes(0, nodeChildCount());
    insertChildrenNodes(0, imgs.size());

    for( size_t cntid = 0; cntid < imgs.size(); ++cntid )
    {
        int NbTiles     = imgs[cntid].data.size() / TileLength;
        if(imgs[cntid].data.size() % TileLength != 0)
            NbTiles += 1;
        int nbtilessqrt = qRound(sqrt(NbTiles));
        int w = (nbtilessqrt + (NbTiles % nbtilessqrt)) * TileWidth;
        int h = (nbtilessqrt) * TileHeight;
        const fmt::step_t * pstep = nullptr;

        for( size_t frmid = 0; frmid < frms.size(); ++frmid )
        {
            //bool foundres   = false;
            auto itstep     = frms[frmid].begin();
            int  curTileNum = 0; //We count the tileids so we can use the tile id in case of -1 frame

            //Look through all the frame's parts
            for( size_t stepid= 0; stepid < frms[frmid].size(); ++stepid, ++itstep )
            {
                auto res = itstep->GetResolution();
                if( static_cast<size_t>(itstep->frmidx) == cntid)
                {
                    pstep = &(*itstep);
                    w = res.first;
                    h = res.second;
                    //foundres = true;
                }
                //                    else if( curTileNum == itstep->getTileNum() )
                //                    {
                //                        pstep = &(*itstep);
                //                        qInfo("ImageContainer::importImages8bpp(): Used matching tilenum for -1 frame!!\n");
                //                        w = res.first;
                //                        h = res.second;
                //                        //foundres = true;
                //                    }
                curTileNum = itstep->getTileNum(); //add up tile index
            }

            //                if(!foundres)
            //                {
            //                    //If all fails, try to deduct from tile number
            //                    qInfo("ImageContainer::importImages8bpp(): Deducting image resolution using number of tiles!!\n");
            //                    int NbTiles = imgs[cntid].data.size() / TileLength;

            //                    int nbtilessqrt = qRound(sqrt(NbTiles));
            //                    if( NbTiles % nbtilessqrt == 0 )
            //                    {
            //                        //if square
            //                        w = nbtilessqrt * TileWidth;
            //                        h = w;
            //                    }
            //                    else
            //                    {
            //                        //not square
            //                        w = nbtilessqrt + (NbTiles % nbtilessqrt) * TileWidth;
            //                        h = nbtilessqrt * TileHeight;
            //                    }
            //                }
        }

        //
        if( (pstep && pstep->isColorPal256()) ||
            (!pstep && parentSprite()->is256Colors()) ) //Assume 8bpp when the sprite is set to 256
            m_container[cntid].importImage8bpp(imgs[cntid], w, h, parentSprite()->isTiled() );
        else
            m_container[cntid].importImage4bpp(imgs[cntid], w, h, parentSprite()->isTiled() ); //default to 16 colors
    }
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages()
{
    Q_ASSERT(false);
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
    {
        images[cntid] = std::move(m_container[cntid].exportImage4bpp(w, h, parentSprite()->isTiled()));
    }
    return std::move(images);
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages4bpp()
{
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
    {
        images[cntid] = std::move(m_container[cntid].exportImage4bpp(w, h, parentSprite()->isTiled()));
    }
    return std::move(images);
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages8bpp()
{
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
    {
        images[cntid] = std::move(m_container[cntid].exportImage8bpp(w, h, parentSprite()->isTiled()));
    }
    return std::move(images);
}

Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parentNode())->parentSprite();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}


//===========================================================================
//  Image
//===========================================================================

QVariant Image::imgData(int column, int role)const
{
    QVariant res;
    switch(column)
    {
    case 0: //preview
        if( role == Qt::DecorationRole )
            res.setValue(makeImage(parentSprite()->getPalette()));
        else if( role == Qt::SizeHintRole )
            res.setValue( QSize(m_img.size().width() *2, m_img.size().height() *2) );
    break;
    case 1:
        if( role == Qt::DisplayRole || role == Qt::EditRole )
            res.setValue(getImageUID());
    break;
    case 2: //depth
        res.setValue(QString("%1bpp").arg(m_depth));
    break;
    case 3: //resolution
        res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
    break;
    };
    return std::move(res);
}

QVariant Image::imgDataCondensed(int role) const
{
    QVariant res;
    if( role == Qt::DecorationRole )
        res.setValue(makeImage(parentSprite()->getPalette()));
    else if( role == Qt::DisplayRole )
        res.setValue(QString("ID:%1 %2bpp %3x%4").arg(nodeIndex()).arg(m_depth).arg(m_img.width()).arg(m_img.height()));
    else if(role == Qt::EditRole)
        res.setValue(nodeIndex());
    return std::move(res);
}

Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}







//
//
//
ImagesManager::ImagesManager(ImageContainer *parent)
    :QAbstractItemModel(), m_parentcnt(parent)
{}

ImagesManager::~ImagesManager()
{
    qDebug("ImagesManager::~ImagesManager()\n");
}

QModelIndex ImagesManager::index(int row, int column, const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    TreeElement *childItem  = parentItem->nodeChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ImagesManager::parent(const QModelIndex &child) const
{
    TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
    TreeElement *parentItem = childItem->parentNode();
    Q_ASSERT(parentItem != nullptr);

    if (parentItem == m_parentcnt)
        return QModelIndex();

    return createIndex(parentItem->nodeIndex(), 0, parentItem);
}

int ImagesManager::rowCount(const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    return parentItem->nodeChildCount();
}

int ImagesManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
    else
        return m_parentcnt->nodeColumnCount();
}

bool ImagesManager::hasChildren(const QModelIndex &parent) const
{
    TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    if(parentItem)
        return parentItem->nodeChildCount() > 0;
    else
        return false;
}

QVariant ImagesManager::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(hasChildren(QModelIndex()));
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
    return img->imgData(index.column(), role);
}

QVariant ImagesManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal )
    {
        switch(section)
        {
        case 0:
            return std::move(QVariant( QString("") ));
        case 1:
            return std::move(QVariant( QString("Bit Depth") ));
        case 2:
            return std::move(QVariant( QString("Resolution") ));
        };
    }
    return QVariant();
}

bool ImagesManager::insertRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildrenNodes(row, count);
    endInsertRows();

    return success;
}

bool ImagesManager::removeRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildrenNodes(row, count);
    endRemoveRows();

    return success;
}

bool ImagesManager::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    Q_ASSERT(false);
    return false;
}

TreeElement *ImagesManager::getItem(const QModelIndex &index)
{
    if (index.isValid())
    {
        TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_parentcnt;
}


//======================================================================================================
//  ImageSelectorModel
//======================================================================================================
QVariant ImageSelectorModel::NullFirstEntry::imgDataCondensed(int role) const
{
    QVariant res;
    if( role == Qt::DecorationRole )
        res.setValue(QImage(32, 32, QImage::Format::Format_ARGB32_Premultiplied));
    else if( role == Qt::DisplayRole )
        res.setValue(QString("ID:-1 Nodraw Frame"));
    else if(role == Qt::EditRole)
        res.setValue(-1);
    return std::move(res);
}

ImageSelectorModel::ImageSelectorModel(ImageContainer *pcnt)
    :ImagesManager(pcnt), m_minusoneimg(new NullFirstEntry(pcnt))
{}

QVariant ImageSelectorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    if(index.column() != 0)
        return QVariant();

    //We fuse all columns together!
    const Image *img = static_cast<const Image*>(getItem(index));
    return img->imgDataCondensed(role);
}

QModelIndex ImageSelectorModel::index(int row, int column, const QModelIndex &parent) const
{
    //insert our minus one frame entry at the beginning!
    if(row == 0)
        return createIndex(0, column, m_minusoneimg.data());
    else if(row < 0)
        return QModelIndex();

    //Otherwise, subtract 1 from the index, so it matches the actual item indices in the imagecontainer object!
    row -= 1;
    //And let the original handle it normally
    return ImagesManager::index(row, column, parent);
}

QVariant ImageSelectorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal && section == 0 )
    {
        return std::move(QVariant( QString("Image") ));
    }
    return QVariant();
}

int ImageSelectorModel::rowCount(const QModelIndex &parent) const
{
    return m_parentcnt->rowCount(parent) + 1; //We always have an extra row!
}

bool ImageSelectorModel::hasChildren(const QModelIndex &/*parent*/) const
{
    return true; //We always have children!
}

TreeElement *ImageSelectorModel::getItem(const QModelIndex &index)
{
    if (index.isValid())
    {
        if( index.row() == 0 ) //special case for -1 frames
            return m_minusoneimg.data();

        TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_parentcnt;
}
