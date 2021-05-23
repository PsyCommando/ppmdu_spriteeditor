#include "framepart.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>

const QString   ElemName_FramePart = "Frame Part";
const std::map<eFramePartColumnsType, QString>  FramePartHeaderColumnNames
{
    {eFramePartColumnsType::Preview,        "Preview"},
    {eFramePartColumnsType::ImgID,          "Image ID"},
    {eFramePartColumnsType::ImgSz,          "Image Size"},
    {eFramePartColumnsType::BlockRange,      "Blocks Used"},
    {eFramePartColumnsType::BlockNum,       "Block Num"},
    {eFramePartColumnsType::ImgIsRef,       "Reference"},
    {eFramePartColumnsType::PaletteID,      "Palette"},
    {eFramePartColumnsType::XOffset,        "X"},
    {eFramePartColumnsType::YOffset,        "Y"},
    {eFramePartColumnsType::VFlip,          "V-Flip"},
    {eFramePartColumnsType::HFlip,          "H-Flip"},
    {eFramePartColumnsType::Mosaic,         "Mosaic"},
    {eFramePartColumnsType::Mode,           "Mode"},
    {eFramePartColumnsType::Priority,       "Priority"},

    //Rotation and scaling
    {eFramePartColumnsType::RnS,            "RnS"},
    {eFramePartColumnsType::RnSParam,       "RnS - Param"},
    {eFramePartColumnsType::RnSCanvasRot,   "RnS - Canvas Rotate"},

    //Research stuff
    {eFramePartColumnsType::Unk0,           "Unk#0"},
};

const QStringList FRAME_PART_PRIORITY_NAMES
{
    "0 - Highest",
    "1 - High",
    "2 - Low",
    "3 - Lowest",
};

const QStringList FRAME_PART_MODE_NAMES
{
    "Normal",
    "Alpha Blended",
    "Windowed",
    "Bitmap",
};

//*******************************************************************
//  MFramePart
//*******************************************************************

MFramePart::MFramePart(TreeNode *mframe)
    :partparent_t(mframe)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
    m_data.attr0 = 0;
    m_data.attr1 = 0;
    m_data.attr2 = 0;
    m_data.setXOffset(255);
    m_data.setYOffset(255);
    m_data.setFrameIndex(-1);
}

MFramePart::MFramePart(TreeNode *mframe, const fmt::step_t &part)
    :partparent_t(mframe), m_data(part)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

MFramePart::MFramePart(const MFramePart &cp)
    : partparent_t(cp.m_parentItem), utils::BaseSequentialIDGen<MFramePart>()
{
    operator=(cp);
}

MFramePart::MFramePart(MFramePart &&mv)
    : partparent_t(mv.m_parentItem), utils::BaseSequentialIDGen<MFramePart>()
{
    operator=(mv);
}

MFramePart::~MFramePart()
{
}

MFramePart &MFramePart::operator=(const MFramePart &cp)
{
    m_flags = cp.m_flags;
    m_data = cp.m_data;
    //we can't copy a unique ID, so don't do it
    return *this;
}

MFramePart &MFramePart::operator=(MFramePart &&mv)
{
    m_parentItem = mv.m_parentItem;
    mv.m_parentItem = nullptr;
    m_flags = mv.m_flags;
    m_data = qMove(mv.m_data);

    //Swap the ids around so we can take its id without having it recycled
    id_t oldid = m_id;
    m_id = mv.m_id;
    mv.m_id = oldid;
    return *this;
}

bool MFramePart::operator==(const MFramePart &other) const
{
    return m_data.attr0 == other.m_data.attr0 &&
           m_data.attr1 == other.m_data.attr1 &&
           m_data.attr2 == other.m_data.attr2;
}

bool MFramePart::operator!=(const MFramePart &other) const
{
    return !operator==(other);
}

TreeNode *MFramePart::clone() const
{
    return new MFramePart(*this);
}

eTreeElemDataType MFramePart::nodeDataTy() const
{
    return eTreeElemDataType::framepart;
}

const QString &MFramePart::nodeDataTypeName() const
{
    return ElemName_FramePart;
}

Qt::ItemFlags MFramePart::nodeFlags(int column) const
{
    Qt::ItemFlags newflags = TreeNodeTerminal::nodeFlags(column);

    switch(static_cast<eFramePartColumnsType>(column))
    {
        case eFramePartColumnsType::BlockRange:
        case eFramePartColumnsType::ImgSz:
        case eFramePartColumnsType::Preview:
        {
            //Can't edit those
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, false);
            break;
        }
        case eFramePartColumnsType::ImgIsRef:
        case eFramePartColumnsType::RnS:
        case eFramePartColumnsType::Mosaic:
        {
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, false); //Checkable needs editable off to work properly
            newflags.setFlag(Qt::ItemIsUserCheckable);
            break;
        }
        case eFramePartColumnsType::RnSCanvasRot:
        {
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, false); //Checkable needs editable off to work properly
            newflags.setFlag(Qt::ItemIsUserCheckable, m_data.isRotAndScalingOn());
            break;
        }
        case eFramePartColumnsType::HFlip:
        case eFramePartColumnsType::VFlip:
        {
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, false); //Checkable needs editable off to work properly
            newflags.setFlag(Qt::ItemIsUserCheckable, !m_data.isRotAndScalingOn());
            break;
        }
        case eFramePartColumnsType::RnSParam:
        {
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, m_data.isRotAndScalingOn());
            break;
        }
        default:
            break;
    }
    return newflags;
}

QImage MFramePart::drawPart(const Sprite * spr, bool transparencyenabled) const
{
    QImage imgo = getSrcImageData(spr);

    //If there's transparency, setup first color to be transparent
    if(transparencyenabled && imgo.colorCount() > 0)
    {
        QRgb alphacol = (~(0xFF << 24)) & imgo.color(0); //Format is ARGB
        imgo.setColor(0, alphacol);
    }

    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
    applyTransforms(imgo);
    return imgo;
}

QPixmap MFramePart::drawPartToPixmap(const Sprite *spr, bool transparencyenabled) const
{
    //#TODO: optimize maybe?
    return QPixmap::fromImage(drawPart(spr, transparencyenabled));
}

void MFramePart::importPart(const fmt::step_t &part)
{
    m_data = part;
}

fmt::step_t MFramePart::exportPart() const
{
    return m_data;
}

fmt::step_t &MFramePart::getPartData()
{
    return m_data;
}

const fmt::step_t &MFramePart::getPartData() const
{
    return m_data;
}

bool MFramePart::isPartReference() const
{
    return m_data.isReference();
}

void MFramePart::setIsReference(bool isref)
{
    if(isref)
        setFrameIndex(-1);
    else if(getFrameIndex() < 0)
        setFrameIndex(0);
    //We don't want to clear the frame index for an existing frame accidently here!! So do nothing if the frame index was already bigger than 0
}

uint16_t MFramePart::getBlockLen() const
{
    const auto res = GetResolution();
    const int totalnbpixels = (res.first * res.second);
    const int nbtiles = totalnbpixels / fmt::NDS_TILE_PIXEL_COUNT;
    const int clampedlen = (isColorPal256())? std::max(nbtiles / fmt::WAN_BLOCK_NB_TILES_8BPP, 1) :
                                              std::max(nbtiles / fmt::WAN_BLOCK_NB_TILES_4BPP, 1);
    return static_cast<uint16_t>(clampedlen);
}

QVector<QRgb> MFramePart::getPartPalette(const QVector<QRgb> &src) const
{
    QVector<QRgb> newpal = src;
    //If the part isn't 8bpp we use sub-palettes
    if(!isColorPal256())
    {
        //Offset the palette so we use the correct colors
        const int paloffset = getPalNb() * fmt::SPR_PAL_NB_COLORS_SUBPAL;
        if(paloffset < newpal.size())
        {
            auto itpalbeg = newpal.begin();
            std::advance(itpalbeg, paloffset);
            auto itpalend = newpal.end();
            const int palend = (paloffset + fmt::SPR_PAL_NB_COLORS_SUBPAL);
            if(palend < newpal.size())
            {
                itpalend = itpalbeg;
                std::advance(itpalend, palend);
            }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            newpal = QVector<QRgb>(itpalbeg, itpalend);
#else
            const int distance = std::distance(itpalbeg, itpalend);
            newpal = QVector<QRgb>(distance);
            std::copy(itpalbeg, itpalend, newpal.begin());
#endif
        }
        else
            qWarning() << "MFramePart::getPartPalette(): Palette for frame part " << nodeIndex() <<" is out of the palette range!!";//if we're out of range of the palette just pop a warning
    }
    return newpal;
}

void MFramePart::applyTransforms(QImage &srcimg) const
{
    srcimg = srcimg.transformed( QTransform().scale( m_data.isHFlip()? -1 : 1,
                                                     m_data.isVFlip()? -1 : 1) ) ;
}

QImage MFramePart::getSrcImageData(const Sprite * spr) const
{
    QImage imgo;
    const QVector<QRgb> pal = getPartPalette(spr->getPalette());
    const MFrame * parent = static_cast<const MFrame*>(parentNode());
    int imgidx = getFrameIndex();
    if(imgidx >= 0)
    {
        //Frames with valid image id
        const Image * pimg = spr->getImage(imgidx);
        if(!pimg)
        {
            qWarning("MFramePart::getSrcImageData(): Invalid image reference!!\n");
            return QImage();
        }
        imgo = pimg->makeImage(pal);
    }
    else
    {
        //-1 image id frames
        const MFramePart * ref = parent->getPartForBlockNum(getBlockNum());
        if(ref)
        {
            //-- Handle block references to previous blocks --
            imgidx = ref->getFrameIndex();
            const Image * pimg = spr->getImage(imgidx);
            if(!pimg)
                return QImage();
            imgo = pimg->makeImage(pal);
        }
        else if(spr->type() == fmt::eSpriteType::Effect) //if(spr->getTileMappingMode() == fmt::eSpriteTileMappingModes::Mapping1D)
        {
            //-- Handle block reference to all image blocks (Only used in some sprite types!!)--
            const uint16_t blocknum = getBlockNum();
            const uint16_t blocklen = getBlockLen();
            std::vector<uint8_t> bytes = spr->getBlocks(blocknum, blocklen);
            const auto resolution = GetResolution();
            if(!bytes.empty())
            {
                //Need to untile the image and convert to something displayable
                if(isColorPal256())
                    imgo = utils::Raw8bppToImg(resolution.first, resolution.second, utils::Untile8bpp(resolution.first, resolution.second, bytes), pal);
                else
                    imgo = utils::Raw4bppToImg(resolution.first, resolution.second, utils::Untile4bpp(resolution.first, resolution.second, bytes), pal);
            }
            else
                qWarning() << QString("MFramePart::getSrcImageData(): Empty image block range (num: %1, len: %2) for frame#%3!").arg(blocknum).arg(blocklen).arg(nodeIndex());
        }
        else
        {
            qWarning() <<"MFramePart::getSrcImageData(): Bad block reference in frame#" <<nodeIndex() <<"!";
        }
    }
    return imgo;
}

