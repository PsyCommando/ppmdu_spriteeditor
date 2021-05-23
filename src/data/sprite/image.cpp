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
    :TreeNodeTerminal(parent), BaseSequentialIDGen()
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

Image & Image::operator=(Image && mv)
{
    m_img       = qMove(mv.m_img);
    m_rawPixels = qMove(mv.m_rawPixels);
    m_depth     = mv.m_depth;
    m_unk2      = mv.m_unk2;
    m_unk14     = mv.m_unk14;
    return *this;
}

Image & Image::operator=(const Image & cp)
{
    m_img       = cp.m_img;
    m_rawPixels = cp.m_rawPixels;
    m_depth     = cp.m_depth;
    m_unk2      = cp.m_unk2;
    m_unk14     = cp.m_unk14;
    return *this;
}

void Image::importBrokenImage(const fmt::ImageDB::img_t &img)
{
    m_broken = true;
    m_rawPixels = img.data;
    m_unk14 = img.unk14;
    m_unk2 = img.unk2;
}

void Image::importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h)
{
    m_depth = fmt::eColorDepths::_4Bpp;
    m_rawPixels = img.data;
    generateCachedImage(w,h);
    m_unk2 = img.unk2;
    m_unk14 = img.unk14;
}

fmt::ImageDB::img_t Image::exportImage4bpp(int & w, int & h)const
{
    assert(m_depth == fmt::eColorDepths::_4Bpp);
    w = m_img.width();
    h = m_img.height();
    fmt::ImageDB::img_t img;
    img.data = m_rawPixels;
    img.unk2 = m_unk2;
    img.unk14 = m_unk14;
    return img;
}

void Image::importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h)
{
    m_depth = fmt::eColorDepths::_8Bpp;
    m_rawPixels = img.data;
    generateCachedImage(w,h);
    m_unk2 = img.unk2;
    m_unk14 = img.unk14;
}

fmt::ImageDB::img_t Image::exportImage8bpp(int & w, int & h)const
{
    assert(m_depth == fmt::eColorDepths::_8Bpp);
    w = m_img.width();
    h = m_img.height();

    fmt::ImageDB::img_t img;
    img.data    = m_rawPixels;
    img.unk2    = m_unk2;
    img.unk14   = m_unk14;
    return img;
}

void Image::generateCachedImage(int w, int h)
{
    switch(m_depth)
    {
    case fmt::eColorDepths::_4Bpp:
        m_img = utils::Raw4bppToImg(w, h, utils::Untile4bpp(w, h, m_rawPixels));
        break;
    case fmt::eColorDepths::_8Bpp:
        m_img = utils::Raw8bppToImg(w, h, utils::Untile8bpp(w, h, m_rawPixels));
        break;
    default:
        assert(false);
        throw BaseException(QString("Image::generateCachedImage(): Bad image depth %1!").arg(static_cast<int>(m_depth)));
    }
}

/*
    Generate a displayable QImage using the specified palette and the image data currently stored.
*/
QImage Image::makeImage(const QVector<QRgb> & palette)const
{
    QImage img(m_img);
    img.setColorTable(palette);
    return img;
}

QPixmap Image::makePixmap(const QVector<QRgb> &palette) const
{
    return QPixmap::fromImage(makeImage(palette));
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
    return QString("ID:%1, %2 x %3, %4bpp").arg(getID()).arg(m_img.width()).arg(m_img.height()).arg(static_cast<int>(getImageOriginalDepth()));
}

int Image::getByteSize()const
{
    return m_rawPixels.size();
}

int Image::getTileSize()const
{
    const int TILE_LEN = (m_depth == fmt::eColorDepths::_4Bpp)? fmt::NDS_TILE_SIZE_4BPP : fmt::NDS_TILE_SIZE_4BPP;
    const int tiles     = getByteSize() / TILE_LEN;
    const int remainder = getByteSize() % TILE_LEN;
    return remainder == 0? tiles : tiles + 1; //Is at least 1 tile if has more than 0 byte
}

int Image::getBlockLen() const
{
    //Blocks are always 0x80 bytes long
    const int blocks = getByteSize() / fmt::WAN_BLOCK_SIZE;
    return (blocks == 0 && getByteSize() > 0)? 1 : blocks; //If we get 0 blocks, but there's more than 0 bytes in this image, return 1 block, otherwise 0. No images can be smaller than one block!
//    const fmt::eColorDepths depth = getImageOriginalDepth();
//    Q_ASSERT(depth != fmt::eColorDepths::Invalid);
//    int nbblocks = 0;
//    if(depth == fmt::eColorDepths::_4Bpp)
//        nbblocks = fmt::TilesToBlocks_4bpp(getTileSize());
//    else if(depth == fmt::eColorDepths::_8Bpp)
//        nbblocks = fmt::TilesToBlocks_8bpp(getTileSize());
//    else
//        throw BaseException(QString("Unexpected orginal depth %1, for image uid %2!").arg(static_cast<int>(depth)).arg(getImageUID()));
//    return nbblocks;
}

std::vector<uint8_t> Image::getBlock(int id) const
{
    return std::vector<uint8_t>(getBlockBeg(id), getBlockEnd(id));
}

std::vector<uint8_t>::const_iterator Image::getBlockBeg(int blocknum) const
{
    try
    {
        if(blocknum >= getBlockLen())
            return m_rawPixels.end();
        return std::next(m_rawPixels.begin(), blocknum * fmt::WAN_BLOCK_SIZE);
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getBlockBeg(): Caught exception"));
    }
}

std::vector<uint8_t>::const_iterator Image::getBlockEnd(int blocknum) const
{
    try
    {
        if((blocknum + 1) > getBlockLen())
            return m_rawPixels.end();
        return std::next(getBlockBeg(blocknum), fmt::WAN_BLOCK_SIZE);
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getBlockEnd(): Caught exception"));
    }
}

std::vector<uint8_t> Image::getTile(int id)const
{
    return std::vector<uint8_t>(getTileBeg(id), getTileEnd(id));
}

std::vector<uint8_t>::const_iterator Image::getTileBeg(int tnum)const
{
    try
    {
        if(tnum >= getTileSize())
            return m_rawPixels.end();
        if(m_depth == fmt::eColorDepths::_8Bpp)
            return std::next(m_rawPixels.begin(), tnum * fmt::NDS_TILE_SIZE_8BPP);
        else if(m_depth == fmt::eColorDepths::_4Bpp)
            return std::next(m_rawPixels.begin(), tnum * fmt::NDS_TILE_SIZE_4BPP);
        else
            throw BaseException(QString("Bad image depth %1 !").arg(static_cast<int>(m_depth)));
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
            return m_rawPixels.end();
        if(m_depth == fmt::eColorDepths::_8Bpp)
            return std::next(getTileBeg(tnum), tnum * fmt::NDS_TILE_SIZE_8BPP);
        else if(m_depth == fmt::eColorDepths::_4Bpp)
            return std::next(getTileBeg(tnum), tnum * fmt::NDS_TILE_SIZE_4BPP);
        else
            throw BaseException(QString("Bad image depth %1 !").arg(static_cast<int>(m_depth)));
    }
    catch(const std::exception & e)
    {
        std::throw_with_nested(std::runtime_error("Image::getTileEnd(): Caught exception"));
    }
}
