#include "imagescontainer.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

const QString ElemName_Images = "Images";


ImageContainer::~ImageContainer()
{
    //qDebug("ImageContainer::~ImageContainer()\n");
}

TreeNode *ImageContainer::clone() const
{
    return new ImageContainer(*this);
}

eTreeElemDataType ImageContainer::nodeDataTy() const
{
    return eTreeElemDataType::images;
}

const QString &ImageContainer::nodeDataTypeName() const
{
    return ElemName_Images;
}

QString ImageContainer::nodeDisplayName()const
{
    return nodeDataTypeName();
}

void ImageContainer::DumpAllImages(const QString &dirpath, const QVector<QRgb> & palette) const
{
    for(const Image * p : *this)
    {
        QImage img = p->makeImage(palette);
        img.save(QString("%1/%2.png").arg(dirpath).arg(p->getID()), "PNG", 9);
    }
}

const QString &ImageContainer::ComboBoxStyleSheet()
{
    static const QString SSheet("QComboBox QAbstractItemView::item {margin-top: 2px;}");
    return SSheet;
}

Image *ImageContainer::appendNewImage()
{
    int newidx = nodeChildCount();
    _insertChildrenNodes(newidx, 1);
    return m_container[newidx];
}

void ImageContainer::importImages(const fmt::ImageDB::imgtbl_t &imgs, const fmt::ImageDB::frmtbl_t &frms)
{
    const Sprite * spr = static_cast<const Sprite*>(parentNode());
    Q_ASSERT(spr);

    //Resize the internal container first
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, imgs.size());

    int newimgcharblock = 0;
    for(int cntimage = 0;cntimage < m_container.size(); ++cntimage)
        newimgcharblock += m_container[cntimage]->getCharBlockLen();

    for( size_t cntid = 0; cntid < imgs.size(); ++cntid )
    {
        const auto & imgref = imgs[cntid];
        const int imgbytelen = imgref.data.size();
        bool is256col = spr->is256Colors();
        int TileByteLen = is256col? fmt::NDS_TILE_SIZE_8BPP : fmt::NDS_TILE_SIZE_4BPP;
        int NbTiles  = imgbytelen / TileByteLen;

        if(imgbytelen % TileByteLen != 0)
        {
            qWarning() << "ImageContainer::importImages(): Encountered an image with less bytes than the expected tile size!"
                       << "Expected tiles: " << TileByteLen <<" bytes, " <<(is256col? "8bpp" : "4bpp" ) <<" bpp."
                       << "Got image that's " <<imgbytelen <<" bytes long.."
                       << "Unk14: " << imgref.unk14 <<", Unk2: " <<imgref.unk2 <<"!";
            //Somtimes some sprites are smaller than a single tile with the expected image depth..
            //mainly in effect sprites
            if(is256col && (imgbytelen * 2) == TileByteLen)
            {
                TileByteLen = fmt::NDS_TILE_SIZE_4BPP; //If its just a 4bpp image in a 8bpp sprite, handle it as 4bpp
                is256col = false;
            }
            else
            {
                //Import but mark it as broken
                m_container[cntid]->importBrokenImage(imgs[cntid]);
                newimgcharblock += fmt::TilesToCharBlocks(NbTiles);
                continue;
            }
        }

        int w = 0;//((nbtilessqrt + (NbTiles % nbtilessqrt)) * fmt::NDS_TILE_PIXEL_WIDTH);
        int h = 0;//(nbtilessqrt) * TileHeight;

        //1. Try to get the resolution from any frame using the image
        //const fmt::step_t * pstep = nullptr;
        for( size_t frmid = 0; frmid < frms.size() && w == 0 && h == 0; ++frmid )
        {
            auto itstep = frms[frmid].begin();
            //Look through all the frame's parts
            for( size_t stepid= 0; stepid < frms[frmid].size() && w == 0 && h == 0; ++stepid, ++itstep )
            {
                auto res = itstep->GetResolution();
                if(static_cast<size_t>(itstep->frmidx) == cntid ||
                    (spr->getTileMappingMode() == fmt::eSpriteTileMappingModes::Mapping1D && itstep->isReference() && itstep->getCharBlockNum() == newimgcharblock))
                {
                    //pstep = &(*itstep);
                    w = res.first;
                    h = res.second;
                    break;
                }
            }
        }

        const int testbytesz = (is256col)? (w * h) : (w * h) / 2; //Make sure the obtained value is sane
        if(testbytesz > static_cast<int>(imgbytelen))
        {
            qDebug() <<"ImageContainer::importImages() : An image has a meta-frame refering to it with a too large resolution for its content!!"
                     <<w <<" x " <<h << " pixels, " << (is256col? "8bpp" : "4bpp" )
                     << ". Byte size: " <<testbytesz <<", available bytes: " <<imgbytelen
                     <<".. Reverting to default size value!";
            w = 0;
            h = 0;
        }

        //2. Attempt to guess resolution if we don't have a frame
        if(w == 0 && h == 0)
        {
            if(spr->getTileMappingMode() == fmt::eSpriteTileMappingModes::Mapping1D)
            {
                //In 1d mapping mode we just lay everything down in the 256x256 virtual char block
                const int totalpixelwidth = NbTiles * fmt::NDS_TILE_PIXEL_WIDTH; //total width of all tiles one next to the other
                //const int nbtilesrows = (totalpixelwidth / 256) + ((totalpixelwidth % 256 > 0)? 1 : 0); //nb of tiles vertically
                w = totalpixelwidth;
                h = fmt::NDS_TILE_PIXEL_HEIGHT;
                qDebug() << "ImageContainer::importImages(): Importing image in 1D mode with no frame assigned!!!";
            }
            else if(spr->getTileMappingMode() == fmt::eSpriteTileMappingModes::Mapping2D)
            {
                //Make the image square if the nb of tiles is a integer square root
                double sqrttnb = sqrt(NbTiles);
                if((trunc(sqrttnb) - sqrttnb) == 0)
                {
                    //No decimal part, make it square
                    w = static_cast<int>(sqrttnb) * fmt::NDS_TILE_PIXEL_WIDTH;
                    h = static_cast<int>(sqrttnb) * fmt::NDS_TILE_PIXEL_HEIGHT;
                    qDebug() << "ImageContainer::importImages(): Importing image in 2D mode with no frame assigned, and a square amount of tiles!!!";
                }
                else
                {
                    //has decimal part so cannot be square
                    qDebug() << "ImageContainer::importImages(): Importing image in 2D mode with no frame assigned and a rectangular shape!!!";
                }
            }
        }

        //3. Depending on the format of the parent sprite, we'll import the image in 8bpp or 4bpp format
        //const bool is256col = (pstep && pstep->isColorPal256()) || (!pstep && spr->is256Colors());
        if(is256col)
            m_container[cntid]->importImage8bpp(imgs[cntid], w, h, true);
        else
            m_container[cntid]->importImage4bpp(imgs[cntid], w, h, true);
        newimgcharblock += fmt::TilesToCharBlocks(NbTiles);
    }
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages()
{
    Q_ASSERT(false);
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    const Sprite * spr = static_cast<const Sprite*>(parentNode());
    Q_ASSERT(spr);
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        images[cntid] = m_container[cntid]->exportImage4bpp(w, h, true);
    return images;
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages4bpp()
{
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    const Sprite * spr = static_cast<const Sprite*>(parentNode());
    Q_ASSERT(spr);
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        images[cntid] = m_container[cntid]->exportImage4bpp(w, h, true);
    return images;
}

fmt::ImageDB::imgtbl_t ImageContainer::exportImages8bpp()
{
    int w = 0;
    int h = 0;
    fmt::ImageDB::imgtbl_t images(nodeChildCount());
    const Sprite * spr = static_cast<const Sprite*>(parentNode());
    Q_ASSERT(spr);
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        images[cntid] = m_container[cntid]->exportImage8bpp(w, h, true);
    return images;
}

QVector<uint8_t> ImageContainer::getTiles(fmt::frmid_t tilenum, fmt::frmid_t len)const
{
    QVector<uint8_t> tiles;
    auto itinsert = std::back_inserter(tiles);
    int cntTotalTiles = 0;
    bool iscopying = false;

    for(int cntimg = 0; cntimg < m_container.size() && len != 0; ++cntimg)
    {
        Image * pcur = m_container[cntimg];
        int imgtilelen = pcur->getTileSize();
        if(!iscopying && tilenum < (cntTotalTiles + imgtilelen))
        {
            iscopying = true;
            //We start copying tiles from this image!
            for(int i = tilenum - cntTotalTiles; i < imgtilelen && len != 0; ++i, --len)
            {
                std::copy(pcur->getTileBeg(i), pcur->getTileEnd(i), itinsert);
            }
        }
        else if((cntTotalTiles + imgtilelen) > tilenum)
        {
            //We're currently copying tiles over, so keep going with this new image!
            for(int i = 0; i < imgtilelen && len != 0; ++i, --len)
            {
                std::copy(pcur->getTileBeg(i), pcur->getTileEnd(i), itinsert);
            }
        }
        cntTotalTiles += imgtilelen;
    }


//    Image * pfound = nullptr;
//    int cntimg = 0;
//    int cnttile = 0;
//    bool copyingtiles = false;
//    for(; tiles.size() < len && cntimg < m_container.size(); ++cntimg)
//    {
//        Image * pcur = m_container[cntimg];
//        if(cnttile == tilenum)
//        {
//            copyingtiles = true;
//            pcur->getTiles();
//        }
//        else if((cnttile + pcur->getTileSize()) > tilenum)
//        {
//            //the tile we want is inside this image..
//            qDebug()<<"Requested to get tile within an image! tilenum: " << tilenum <<", imgid: " << cntimg;
//        }
//        cnttile += pcur->getTileSize();
//    }
    return tiles;
}

QVector<uint8_t> ImageContainer::getCharBlocks(fmt::frmid_t num, fmt::frmid_t len) const
{
    //return getTiles(fmt::CharBlocksToTiles(num), fmt::CharBlocksToTiles(len));


    QVector<uint8_t> blocks;
    auto itinsert = std::back_inserter(blocks);
    int cntTotalBlocks = 0;
    bool iscopying = false;

    for(int cntimg = 0; cntimg < m_container.size() && len != 0; ++cntimg)
    {
        Image * pcur = m_container[cntimg];
        int imgBlockLen = pcur->getCharBlockLen();
        if(!iscopying && num < (cntTotalBlocks + imgBlockLen))
        {
            iscopying = true;
            //We start copying tiles from this image!
            for(int i = num - cntTotalBlocks; i < imgBlockLen && len != 0; ++i, --len)
            {
                std::copy(pcur->getCharBlockBeg(i), pcur->getCharBlockEnd(i), itinsert);
            }
        }
        else if((cntTotalBlocks + imgBlockLen) > num)
        {
            //We're currently copying tiles over, so keep going with this new image!
            for(int i = 0; i < imgBlockLen && len != 0; ++i, --len)
            {
                std::copy(pcur->getCharBlockBeg(i), pcur->getCharBlockEnd(i), itinsert);
            }
        }
        cntTotalBlocks += imgBlockLen;
    }
    return blocks;
}

Image *ImageContainer::getImageByTileNum(fmt::frmid_t tilenum)
{
    Image * pfound = nullptr;
    int cntimg = 0;
    int cnttile = 0;
    for(; cnttile < tilenum && cntimg < m_container.size(); ++cntimg)
    {
        Image * pcur = m_container[cntimg];
        if(cnttile == tilenum)
        {
            pfound = pcur;
            break;
        }
        else if((cnttile + pcur->getTileSize()) > tilenum)
        {
            //the tile we want is inside this image..
            qDebug()<<"Requested to get tile within an image! tilenum: " << tilenum <<", imgid: " << cntimg;
        }
        cnttile += pcur->getTileSize();
    }
    return pfound;
}

const Image *ImageContainer::getImageByTileNum(fmt::frmid_t tilenum) const
{
    return const_cast<ImageContainer*>(this)->getImageByTileNum(tilenum);
}

QVector<uint8_t> ImageContainer::getTileData(int id, int len) const
{
    return getTileDataFromImage(0, id, len);
}

QVector<uint8_t> ImageContainer::getTileDataFromImage(int imgidx, int id, int len) const
{
    const Sprite * spr = static_cast<const Sprite*>(parentNode());
    QVector<uint8_t> data;

    const int tileLen = spr->is256Colors() ? fmt::NDS_TILE_SIZE_8BPP : fmt::NDS_TILE_SIZE_4BPP;
    const int tileEnd = (id + len);
    data.reserve(len * tileLen);

    int curImg = imgidx;
    int totalTiles = 0;
    int totalBytes = 0;
    //First get to start of the first tile
    //Then copy all tiles up to "len"
    for(;curImg < nodeChildCount(); ++curImg)
    {
        const Image * img = m_container[curImg];
        for(int cntImgTiles = 0; cntImgTiles < img->getTileSize() ; ++totalTiles, ++cntImgTiles)
        {
            if(totalTiles >= id && totalTiles < tileEnd)
            {
                std::vector<uint8_t> atile = img->getTile(cntImgTiles);
                data.append(QVector<uint8_t>(atile.begin(), atile.end()));
            }
            else if(totalTiles >= tileEnd)
                break; //Break when we have everything we need
            totalBytes += tileLen;
        }
    }

    //If the tile is out of range, or if its not completely full
    if(data.size() < (len * tileLen))
    {
        //make sure we end up with data in there
        const int diff = (len * tileLen) - data.size();
        auto itback = std::back_insert_iterator(data);
        std::fill_n( itback, diff, 0); //Make sure we add zeros and not garbage
    }
    return data;
}

//QVariant ImageContainer::nodeData(int column, int role) const
//{
//    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(ElemName());
//    return QVariant();
//}

//Sprite *ImageContainer::parentSprite()
//{
//    return static_cast<Sprite*>(parentNode());
//}

//QVariant ImageContainer::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant("root");

//    if (role != Qt::DisplayRole &&
//            role != Qt::DecorationRole &&
//            role != Qt::SizeHintRole &&
//            role != Qt::EditRole)
//        return QVariant();

//    const Image *img = static_cast<const Image*>(getItem(index));
//    QVariant res;

//    switch(static_cast<Image::eColumnType>(index.column()))
//    {
//        case Image::eColumnType::Preview: //preview
//        {
//            if( role == Qt::DecorationRole )
//                res.setValue(makeImage(parentSprite()->getPalette()));
//            else if( role == Qt::SizeHintRole )
//            {
//                QSize imgsz = img->getImageSize();
//                imgsz.scale(2, 2, Qt::AspectRatioMode::KeepAspectRatio);
//                res.setValue(imgsz);
//            }
//            break;
//        }
//        case Image::eColumnType::UID:
//        {
//            if( role == Qt::DisplayRole || role == Qt::EditRole )
//                res.setValue(img->getImageUID());
//            break;
//        }
//        case Image::eColumnType::Depth: //depth
//        {
//            if( role == Qt::DisplayRole )
//                res.setValue(QString("%1bpp").arg(img->getImageOriginalDepth()));
//            break;
//        }
//        case Image::eColumnType::Resolution: //resolution
//        {
//            if( role == Qt::DisplayRole )
//            {
//                const QSize imgsz = img->getImageSize();
//                res.setValue(QString("%1x%2").arg(imgsz.width()).arg(imgsz.height()));
//            }
//            break;
//        }
//        case Image::eColumnType::direct_Unk2:
//        {
//            if( role == Qt::DisplayRole || role == Qt::EditRole )
//                res.setValue(img->getUnk2());
//            break;
//        }
//        case Image::eColumnType::direct_Unk14:
//        {
//            if( role == Qt::DisplayRole || role == Qt::EditRole )
//                res.setValue(img->getUnk14());
//            break;
//        }
//        default:
//            break; //Get rid of warnings
//    };
//    return res;
//}

//bool ImageContainer::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    if (!index.isValid() || role != Qt::EditRole)
//        return false;
//    Image *img = static_cast<Image*>(getItem(index));
//    bool  succ = false;

//    switch(static_cast<Image::eColumnType>(index.column()))
//    {
//        case Image::eColumnType::direct_Unk2:
//        {
//            using unk2t = decltype(img->getUnk2());
//            unk2t res = static_cast<unk2t>(value.toUInt(&succ));
//            if(succ)
//                img->setUnk2(res);
//            break;
//        }
//        case Image::eColumnType::direct_Unk14:
//        {
//            using unk14t = decltype(img->getUnk14());
//            unk14t res = static_cast<unk14t>(value.toUInt(&succ));
//            if(succ)
//                img->setUnk14(res);
//            break;
//        }
//        case Image::eColumnType::Preview:   [[fallthrough]]; //preview
//        case Image::eColumnType::UID:       [[fallthrough]];
//        case Image::eColumnType::Depth:     [[fallthrough]]; //depth
//        case Image::eColumnType::Resolution:[[fallthrough]]; //resolution
//        default: //To get rid of warnings
//            break;
//    };

//    emit dataChanged(index, index, {role});
//    return succ;
//}

//QVariant ImageContainer::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//    {
//        return QVariant( QString("%1").arg(section) );
//    }
//    else if( orientation == Qt::Orientation::Horizontal )
//    {
//        switch(static_cast<Image::eColumnType>(section))
//        {
//        case Image::eColumnType::Preview:
//            return QVariant( QString("") );
//        case Image::eColumnType::UID:
//            return QVariant( QString("UID") );
//        case Image::eColumnType::Depth:
//            return QVariant( QString("Bit Depth") );
//        case Image::eColumnType::Resolution:
//            return QVariant( QString("Resolution") );
//        default:
//            return QVariant(); //Avoid warning for unhandled enum values
//        };
//    }
//    return QVariant();
//}

