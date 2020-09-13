#include "image.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/data/sprite/imagescontainer.hpp>


const char * ElemName_Image = "Image";

//===========================================================================
//  Image
//===========================================================================

Image::Image(TreeElement * parent)
    :BaseTreeTerminalChild(parent), BaseSequentialIDGen(), m_depth(0), m_unk2(0), m_unk14(0)
{
    setNodeDataTy(eTreeElemDataType::image);
}

Image::Image(Image && mv)
    :BaseTreeTerminalChild(std::move(mv)), BaseSequentialIDGen(std::move(mv))
{
    operator=(std::move(mv));
}

Image::Image(const Image & cp)
    :BaseTreeTerminalChild(cp), BaseSequentialIDGen() //make a brand new id on copies
{
    operator=(cp);
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

void Image::clone(const TreeElement *other)
{
    const Image * ptr = static_cast<const Image*>(other);
    if(!ptr)
        throw std::runtime_error("Image::clone(): other is not a Image!");
    (*this) = *ptr;
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parentNode())->parentSprite();
}

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

QVariant Image::imgData(int column, int role)const
{
    QVariant res;
    switch(static_cast<eColumnType>(column))
    {
        case eColumnType::Preview: //preview
        {
            if( role == Qt::DecorationRole )
                res.setValue(makeImage(parentSprite()->getPalette()));
            else if( role == Qt::SizeHintRole )
                res.setValue( QSize(m_img.size().width() *2, m_img.size().height() *2) );
            break;
        }
        case eColumnType::UID:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(getImageUID());
            break;
        }
        case eColumnType::Depth: //depth
        {
            if( role == Qt::DisplayRole )
                res.setValue(QString("%1bpp").arg(m_depth));
            break;
        }
        case eColumnType::Resolution: //resolution
        {
            if( role == Qt::DisplayRole )
                res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
            break;
        }
        case eColumnType::direct_Unk2:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(m_unk2);
            break;
        }
        case eColumnType::direct_Unk14:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(m_unk14);
            break;
        }
        default:
            break; //Get rid of warnings
    };
    return res;
}

bool Image::setImgData(int column, const QVariant &value, int role)
{

    if(role != Qt::EditRole)
        return false;

    bool succ = false;
    switch(static_cast<eColumnType>(column))
    {
        case eColumnType::Preview:  [[fallthrough]]; //preview
        case eColumnType::UID:      [[fallthrough]];
        case eColumnType::Depth:    [[fallthrough]]; //depth
        case eColumnType::Resolution: //resolution
        {
            succ = false;
            break;
        }
        case eColumnType::direct_Unk2:
        {
            decltype(m_unk2) res = static_cast<decltype(m_unk2)>(value.toUInt(&succ));
            if(succ)
                m_unk2 = res;
            break;
        }
        case eColumnType::direct_Unk14:
        {
            decltype(m_unk14) res = static_cast<decltype(m_unk14)>(value.toUInt(&succ));
            if(succ)
                m_unk14 = res;
            break;
        }
        default:
            break; //To get rid of warnings
    };
    return succ;
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
    return res;
}
