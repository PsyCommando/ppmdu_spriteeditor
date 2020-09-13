#include "imagescontainer.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

const char * ElemName_Images = "Images";

void ImageContainer::clone(const TreeElement *other)
{
    const ImageContainer * ptr = static_cast<const ImageContainer*>(other);
    if(!ptr)
        throw std::runtime_error("ImageContainer::clone(): other is not a ImageContainer!");
    (*this) = *ptr;
}

const QString &ImageContainer::ComboBoxStyleSheet()
{
    static const QString SSheet("QComboBox QAbstractItemView::item {margin-top: 2px;}");
    return SSheet;
}

void ImageContainer::importImages(const fmt::ImageDB::imgtbl_t &imgs, const fmt::ImageDB::frmtbl_t &frms)
{
    static const int TileWidth  = 8;
    static const int TileHeight = 8;
    static const int TileLength = 64;
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, imgs.size());

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
            //int  curTileNum = 0; //We count the tileids so we can use the tile id in case of -1 frame

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
                //curTileNum = itstep->getTileNum(); //add up tile index
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
    return images;
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
    return images;
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
    return images;
}

QVariant ImageContainer::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

QVariant ImageContainer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const Image *img = static_cast<const Image*>(getItem(index));
    return img->imgData(index.column(), role);
}

bool ImageContainer::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    Image *img = static_cast<Image*>(getItem(index));
    return img->setImgData(index.column(), value, role);
}

QVariant ImageContainer::headerData(int section, Qt::Orientation orientation, int role) const
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
