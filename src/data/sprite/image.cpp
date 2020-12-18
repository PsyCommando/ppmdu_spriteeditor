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
    :TreeNodeTerminal(std::move(mv)), BaseSequentialIDGen(std::move(mv))
{
    operator=(std::move(mv));
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
    m_img   = qMove(mv.m_img);
    m_raw   = qMove(mv.m_raw);
    m_depth = mv.m_depth;
    m_unk2  = mv.m_unk2;
    m_unk14 = mv.m_unk14;
    return *this;
}

Image & Image::operator=(const Image & cp)
{
    m_img   = cp.m_img;
    m_raw   = cp.m_raw;
    m_depth = cp.m_depth;
    m_unk2  = cp.m_unk2;
    m_unk14 = cp.m_unk14;
    return *this;
}

//Sprite *Image::parentSprite()
//{
//    return static_cast<ImageContainer*>(parentNode())->parentSprite();
//}

void Image::importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled)
{
    m_depth = 4;
    QVector<QRgb> dummy(16);
    if(isTiled)
        m_raw = utils::Untile( w, h, utils::Expand4BppTo8Bpp(img.data) );
    else
        m_raw = img.data;
    m_img = utils::RawToImg(w, h, m_raw, dummy);
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
        m_raw = utils::Untile(w, h, img.data);
    else
        m_raw = img.data;

    m_img = utils::RawToImg(w, h, m_raw, dummy);
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
    return QString("ID:%1, %2 x %3, %4bpp").arg(getID()).arg(m_img.width()).arg(m_img.height()).arg(m_depth);
}

int Image::getByteSize()const
{
    return m_raw.size();
}

int Image::getTileSize()const
{
//    if(m_depth == 8)
        return getByteSize() / fmt::NDS_TILE_SIZE_8BPP;
//    else if(m_depth == 4)
//        return getByteSize() / fmt::NDS_TILE_SIZE_4BPP;
//    else
//        throw std::runtime_error("Image::getTileSize() : Bad image depth");
}

std::vector<uint8_t> Image::getTile(int id)const
{
    //Tiles are always 8bpp tiles since data is stored as 8bpp
    const unsigned int tilesz = /*(m_depth == 8)?*/ fmt::NDS_TILE_SIZE_8BPP /*: (m_depth == 4)? fmt::NDS_TILE_SIZE_4BPP : 0*/;
    auto itbeg = m_raw.begin();
    auto itend = m_raw.end();
    std::advance(itbeg, id * tilesz);

    //If end of tile not past the end of the image, set it up properly
    if((id + 1) * tilesz < m_raw.size())
    {
        itend = itbeg;
        std::advance(itend, tilesz);
    }

    return std::vector<uint8_t>(itbeg, itend);
}
