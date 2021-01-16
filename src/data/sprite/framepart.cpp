#include "framepart.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>

const QString   ElemName_FramePart = "Frame Part";
const std::map<eFramePartColumnsType, QString>  FramePartHeaderColumnNames
{
    {eFramePartColumnsType::Preview,        "Preview"},
    {eFramePartColumnsType::ImgID,          "Image ID"},
    {eFramePartColumnsType::ImgSz,          "Image Size"},
    {eFramePartColumnsType::TileNum,        "Tile ID"},
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
    "Blended",
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
        case eFramePartColumnsType::ImgSz:
        case eFramePartColumnsType::Preview:
        {
            //Can't edit those
            newflags.setFlag(Qt::ItemFlag::ItemIsEditable, false);
            break;
        }
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
    const int TILESZ = fmt::NDS_TILE_SIZE_8BPP * 4;
    QImage          imgo;
    QVector<QRgb>   newpal = getPartPalette(spr->getPalette()); //Grab the part of the palette we care about
    const MFrame * parent = static_cast<const MFrame*>(parentNode());
    QVector<uint8_t> tilebuffer = parent->generateTilesBuffer(spr, nodeIndex());

    //If there's transparency, setup first color to be transparent
    if(transparencyenabled)
    {
        if(newpal.size() > 0)
            newpal.front() = (~(0xFF << 24)) & newpal.front(); //Format is ARGB
    }

    //Actual frame
    if(getFrameIndex() < 0)
    {
        //-1 frame
        //QVector<uint8_t> tiles = spr->getImages().getTileDataFromImage(firstFrmIdx, getTileNum(), getTileLen());
        const int frmbytelen = getTileLen() * TILESZ;
        const int frmbytebeg = getTileNum() * TILESZ;
        auto itTileBeg = tilebuffer.begin();
        auto itTileEnd = tilebuffer.end();

        if(frmbytebeg > tilebuffer.size())
            throw BaseException("WAH");
        std::advance(itTileBeg, frmbytebeg);
        if((frmbytelen + frmbytebeg) < tilebuffer.size())
        {
            itTileEnd = itTileBeg;
            std::advance(itTileEnd, frmbytelen);
        }
        QVector<uint8_t> tiles = QVector<uint8_t>(itTileBeg, itTileEnd);
        auto res = GetResolution(); //get the actual resolution
        imgo = utils::RawToImg(res.first, res.second, tiles, newpal);
        imgo = imgo.copy(); //test
        //imgo.save(QString("D:/Users/Guill/Documents/CodingProjects/ppmdu_spriteeditor/workdir/test_%1_%2.png").arg(parent->nodeIndex()).arg(nodeIndex()), "PNG");
    }
    else
    {
        //valid frame
        const int imgidx = getFrameIndex();
        const Image * pimg = spr->getImage(imgidx);
        if(!pimg)
        {
            qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
            return QImage();
        }
        imgo = pimg->makeImage(newpal);
    }

    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
    applyTransforms(imgo);

    return imgo;
}
//{
//    int imgindex = -1;

//    if(m_data.getFrameIndex() < 0)
//    {
//        qInfo("MFramePart::drawPart(): was asked to draw a -1 frame!!\n");

//        if(nodeIndex() > 0)
//        {
//            const MFrame * frm = static_cast<const MFrame*>(parentNode());
//            if(!frm)
//            {
//                qWarning("MFramePart::drawPart(): Can't access parent frame!!\n");
//                return QImage(); //Can't do much here!
//            }

//            //find previous non -1 frame part!
//            for( int cntlkb = nodeIndex(); cntlkb >= 0; --cntlkb )
//            {
//                const fmt::step_t * p = frm->getPart(cntlkb);
//                Q_ASSERT(p);
//                if(p->getFrameIndex() >= 0)
//                {
//                    imgindex = p->getFrameIndex();
//                    break;
//                }
//            }
//        }

//        //If we didn't find a replacement frame, just return an empty image.
//        if(imgindex < 0)
//            return QImage();
//        //Otherwise continue with the valid index!
//    }
//    else
//    {
//        imgindex = m_data.getFrameIndex();
//    }

//    //auto res = m_data.GetResolution();
//    QImage imgo;
//    Q_ASSERT(spr);
//    const Image * pimg = spr->getImage( imgindex );
//    if(!pimg)
//    {
//        qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
//        return QImage();
//    }

//    if(spr->unk13() == 1)
//        qDebug("MFramePart::drawPart(): This part probably won't be drawn correctly, since it appears to be set to 1D mapping!\n");


//    if(transparencyenabled)
//    {
//        QVector<QRgb> newpal = spr->getPalette();
//        if(newpal.size() > 0)
//            newpal.front() = (~(0xFF << 24)) & newpal.front(); //Format is ARGB
//        imgo = pimg->makeImage(newpal);
//    }
//    else
//    {
//        imgo = pimg->makeImage(spr->getPalette());
//    }

//    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
//    applyTransforms(imgo);
//    return imgo;
//}

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

uint16_t MFramePart::getTileLen() const
{
    const auto res = GetResolution();
    const int pixellen = (res.first * res.second);
    const int clampedlen = std::max(pixellen / fmt::NB_PIXELS_WAN_TILES, 1);
    return static_cast<uint16_t>(clampedlen); //Since tiles are 16x16, and 8x8 exists, we gotta make sure we don't return 0
}

QVector<QRgb> MFramePart::getPartPalette(const QVector<QRgb> &src) const
{
    QVector<QRgb> newpal = src;
    //Parts may use palette 0 to 16
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
            newpal = QVector<QRgb>(itpalbeg, itpalend);
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

