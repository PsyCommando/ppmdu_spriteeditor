#include "image.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/data/sprite/imagescontainer.hpp>

const QString ElemName_Image = "Image";

//===========================================================================
//  Image
//===========================================================================

Image::Image(TreeNode * parent)
    :TreeNodeTerminal(parent), BaseSequentialIDGen(), m_depth(0), m_unk2(0), m_unk14(0)
{
}

Image::Image(Image && mv)
    :TreeNodeTerminal(std::forward<Image>(mv)), BaseSequentialIDGen(std::forward<Image>(mv))
{
    operator=(mv);
}

Image::Image(const Image & cp)
    :TreeNodeTerminal(cp), BaseSequentialIDGen() //make a brand new id on copies
{
    operator=(cp);
}

Image::~Image()
{
}

void Image::importBrokenImage(const fmt::ImageDB::img_t &img)
{
    m_broken = true;
    m_raw8bppPixels = img.data;
    m_unk14 = img.unk14;
    m_unk2 = img.unk2;
}

Image & Image::operator=(Image && mv)
{
    m_img   = qMove(mv.m_img);
    m_raw8bppPixels   = qMove(mv.m_raw8bppPixels);
    m_depth = mv.m_depth;
    m_unk2  = mv.m_unk2;
    m_unk14 = mv.m_unk14;
    return *this;
}

Image & Image::operator=(const Image & cp)
{
    m_img   = cp.m_img;
    m_raw8bppPixels   = cp.m_raw8bppPixels;
    m_depth = cp.m_depth;
    m_unk2  = cp.m_unk2;
    m_unk14 = cp.m_unk14;
    return *this;
}

void Image::importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled)
{
    m_depth = 4;
    QVector<QRgb> dummy(16);
    if(isTiled)
        m_raw8bppPixels = utils::Untile( w, h, utils::Expand4BppTo8Bpp(img.data) );
    else
        m_raw8bppPixels = utils::Expand4BppTo8Bpp(img.data);
    m_img = utils::RawToImg(w, h, m_raw8bppPixels, dummy);
    m_unk2 = img.unk2;
    m_unk14 = img.unk14;
}

fmt::ImageDB::img_t Image::exportImage4bpp(int & w, int & h, bool tiled)const
{
    w = m_img.width();
    h = m_img.height();
    fmt::ImageDB::img_t img;

    if(tiled)
        img.data = utils::Reduce8bppTo4bpp(utils::TileFromImg(m_img));
    else
        img.data = utils::Reduce8bppTo4bpp(m_img);

    img.unk2 = m_unk2;
    img.unk14 = m_unk14;
    return img;
}

void Image::importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled)
{
    m_depth = 8;
    QVector<QRgb> dummy(256);

    if(isTiled)
        m_raw8bppPixels = utils::Untile(w, h, img.data);
    else
        m_raw8bppPixels = img.data;

    m_img = utils::RawToImg(w, h, m_raw8bppPixels, dummy);
    m_unk2 = img.unk2;
    m_unk14 = img.unk14;
}

fmt::ImageDB::img_t Image::exportImage8bpp(int & w, int & h, bool tiled)const
{
    w = m_img.width();
    h = m_img.height();
    fmt::ImageDB::img_t img;

    if(tiled)
        img.data = utils::TileFromImg(m_img);
    else
        img.data = utils::ImgToRaw(m_img);
    img.unk2 = m_unk2;
    img.unk14 = m_unk14;
    return img;
}

/*
    Generate a displayable QImage using the specified palette and the image data currently stored.
*/
QImage Image::makeImage( const QVector<QRgb> & palette )const
{
    QImage img(m_img);
    img.setColorTable(palette);
    return img;
}

TreeNode *Image::clone() const
{
    return new Image(*this);
}
eTreeElemDataType Image::nodeDataTy() const
{
    return eTreeElemDataType::image;
}
const QString & Image::nodeDataTypeName() const
{
    return ElemName_Image;
}

QString Image::getImageDescription()const
{
    return QString("ID:%1, %2 x %3, %4bpp").arg(getID()).arg(m_img.width()).arg(m_img.height()).arg(getImageOriginalDepth());
}

int Image::getByteSize()const
{
    return m_raw8bppPixels.size();
}

int Image::getTileSize()const
{
    const int tiles     = getByteSize() / fmt::NDS_TILE_SIZE_8BPP;
    const int remainder = getByteSize() % fmt::NDS_TILE_SIZE_8BPP;
    return remainder == 0? tiles : tiles + 1; //Is at least 1 tile if has more than 0 byte

//    if(getImageOriginalDepth() == 8)
//        return getByteSize() / fmt::NDS_TILE_SIZE_8BPP;
//    else if(getImageOriginalDepth() == 4)
//        return getByteSize() / fmt::NDS_TILE_SIZE_4BPP;
//    else
        //        throw BaseException("Image::getTileSize() : Bad image depth");
}

int Image::getCharBlockLen() const
{
    const int blocks    = getTileSize() / fmt::NDS_TILES_PER_CHAR_BLOCK;
    const int remainder = getTileSize() % fmt::NDS_TILES_PER_CHAR_BLOCK;
    return remainder == 0? blocks : blocks + 1; //Is at least 1 block if has more than 0 tile
}

std::vector<uint8_t> Image::getCharBlock(int id) const
{
    return std::vector<uint8_t>(getCharBlockBeg(id), getCharBlockEnd(id));
}

std::vector<uint8_t>::const_iterator Image::getCharBlockBeg(int blocknum) const
{
    try
    {
        if(blocknum >= getCharBlockLen())
            return m_raw8bppPixels.end();
        return std::next(m_raw8bppPixels.begin(), (blocknum * fmt::NDS_TILES_PER_CHAR_BLOCK) * fmt::NDS_TILE_SIZE_8BPP);
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getCharBlockBeg(): Caught exception"));
    }
}

std::vector<uint8_t>::const_iterator Image::getCharBlockEnd(int blocknum) const
{
    try
    {
        if((blocknum + 1) > getCharBlockLen())
            return m_raw8bppPixels.end();
        return std::next(getCharBlockBeg(blocknum), (fmt::NDS_TILES_PER_CHAR_BLOCK * fmt::NDS_TILE_SIZE_8BPP));
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getCharBlockEnd(): Caught exception"));
    }
}

std::vector<uint8_t> Image::getTile(int id)const
{
    //Tiles are always 8bpp tiles since data is stored as 8bpp
//    const unsigned int tilesz = /*(m_depth == 8)?*/ fmt::NDS_TILE_SIZE_8BPP /*: (m_depth == 4)? fmt::NDS_TILE_SIZE_4BPP : 0*/;
//    auto itbeg = m_raw.begin();
//    auto itend = m_raw.end();
//    std::advance(itbeg, id * tilesz);

//    //If end of tile not past the end of the image, set it up properly
//    if((id + 1) * tilesz < m_raw.size())
//    {
//        itend = itbeg;
//        std::advance(itend, tilesz);
//    }

    return std::vector<uint8_t>(getTileBeg(id), getTileEnd(id));
}

std::vector<uint8_t>::const_iterator Image::getTileBeg(int tnum)const
{
    try
    {
        if(tnum >= getTileSize())
            return m_raw8bppPixels.end();
        return std::next(m_raw8bppPixels.begin(), tnum * fmt::NDS_TILE_SIZE_8BPP);
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getTileBeg(): Caught exception"));
    }
}

std::vector<uint8_t>::const_iterator Image::getTileEnd(int tnum)const
{
    try
    {
        if((tnum + 1) > getTileSize())
            return m_raw8bppPixels.end();
        return std::next(getTileBeg(tnum), fmt::NDS_TILE_SIZE_8BPP);
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getTileEnd(): Caught exception"));
    }
}
