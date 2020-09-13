#include "framepart.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framesdelegate.hpp>

const char * ElemName_FramePart = "Frame Part";

//*******************************************************************
//  MFramePart
//*******************************************************************

MFramePart::MFramePart(TreeElement *parent)
    :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags())
{
    setNodeDataTy(eTreeElemDataType::framepart);
    m_data.attr0 = 0;
    m_data.attr1 = 0;
    m_data.attr2 = 0;
    m_data.setXOffset(255);
    m_data.setYOffset(255);
    m_data.setFrameIndex(-1);
}

MFramePart::MFramePart(TreeElement *parent, const fmt::step_t &part)
    :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags()), m_data(part)
{
    setNodeDataTy(eTreeElemDataType::framepart);
}

MFramePart::~MFramePart()
{
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

Sprite * MFramePart::parentSprite()
{
    return static_cast<MFrame*>(parentNode())->parentSprite();
}

const Sprite *MFramePart::parentSprite() const
{
    return const_cast<MFramePart*>(this)->parentSprite();
}

int MFramePart::nodeColumnCount() const
{
    return FramesHeaderNBColumns;
}

Qt::ItemFlags MFramePart::nodeFlags(int column) const
{
    if(column == static_cast<int>(eFramesColumnsType::Preview) )
        return Qt::ItemFlags(m_flags).setFlag(Qt::ItemFlag::ItemIsEditable, false); //The preview is never editable!
    return m_flags;
}

QVariant MFramePart::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    switch(static_cast<eFramesColumnsType>(column))
    {
    case eFramesColumnsType::Preview:       return dataImgPreview(role);
    case eFramesColumnsType::ImgID:         return dataImgId(role);
    case eFramesColumnsType::TileNum:       return dataTileNum(role);
    case eFramesColumnsType::PaletteID:     return dataPaletteID(role);
    case eFramesColumnsType::Unk0:          return dataUnk0(role);
    case eFramesColumnsType::Offset:        return dataOffset(role);
    case eFramesColumnsType::Flip:          return dataFlip(role);
    case eFramesColumnsType::RotNScaling:   return dataRotNScaling(role);
    case eFramesColumnsType::Mosaic:        return dataMosaic(role);
    case eFramesColumnsType::Mode:          return dataMode(role);
    case eFramesColumnsType::Priority:      return dataPriority(role);

    case eFramesColumnsType::direct_VFlip:   return dataDirectVFlip(role);
    case eFramesColumnsType::direct_HFlip:   return dataDirectHFlip(role);
    case eFramesColumnsType::direct_XOffset: return dataDirectXOffset(role);
    case eFramesColumnsType::direct_YOffset: return dataDirectYOffset(role);

        //Undefined cases
    default:
        break;
    };

    return QVariant();
}

QImage MFramePart::drawPart(bool transparencyenabled) const
{
    int imgindex = -1;

    if(m_data.getFrameIndex() < 0)
    {
        qInfo("MFramePart::drawPart(): was asked to draw a -1 frame!!\n");

        if(nodeIndex() > 0)
        {
            const MFrame * frm = static_cast<const MFrame*>(parentNode());
            if(!frm)
            {
                qWarning("MFramePart::drawPart(): Can't access parent frame!!\n");
                return QImage(); //Can't do much here!
            }

            //find previous non -1 frame part!
            for( int cntlkb = nodeIndex(); cntlkb >= 0; --cntlkb )
            {
                const fmt::step_t * p = frm->getPart(cntlkb);
                Q_ASSERT(p);
                if(p->getFrameIndex() >= 0)
                {
                    imgindex = p->getFrameIndex();
                    break;
                }
            }
        }

        //If we didn't find a replacement frame, just return an empty image.
        if(imgindex < 0)
            return QImage();
        //Otherwise continue with the valid index!
    }
    else
    {
        imgindex = m_data.getFrameIndex();
    }

    //auto res = m_data.GetResolution();
    QImage imgo;
    const Sprite* spr = parentSprite();
    Q_ASSERT(spr);
    const Image * pimg = spr->getImage( imgindex );
    if(!pimg)
    {
        qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
        return QImage();
    }

    if(spr->unk13() == 1)
        qDebug("MFramePart::drawPart(): This part probably won't be drawn correctly, since it appears to be set to 1D mapping!\n");


    if(transparencyenabled)
    {
        QVector<QRgb> newpal = spr->getPalette();
        if(newpal.size() > 0)
            newpal.front() = (~(0xFF << 24)) & newpal.front(); //Format is ARGB
        imgo = pimg->makeImage(newpal);
    }
    else
    {
        imgo = pimg->makeImage(spr->getPalette());
    }

    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
    applyTransforms(imgo);
    return imgo;
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

QSize MFramePart::calcTextSize(const QString &str)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.horizontalAdvance(str), fm.height());
}

void MFramePart::applyTransforms(QImage &srcimg) const
{
    srcimg = srcimg.transformed( QTransform().scale( m_data.isHFlip()? -1 : 1,
                                                    m_data.isVFlip()? -1 : 1) ) ;
}

QVariant MFramePart::dataImgPreview(int role) const
{
    Sprite *parentspr = const_cast<MFramePart*>(this)->parentSprite();
    if(role == Qt::DecorationRole)
    {
        //#TODO: Draw only this part/step
        if( m_data.getFrameIndex() >= 0 && m_data.getFrameIndex() < parentspr->getImages().nodeChildCount() )
        {
            Image * pimg = parentspr->getImage(m_data.getFrameIndex());
            if(!pimg)
                qCritical("MFrame::dataImgPreview(): Invalid image at index %d!\n", m_data.getFrameIndex());
            else
                return QVariant(pimg->makeImage(parentspr->getPalette()));
        }
        else
        {
            //#TODO: draw last step
            return QVariant("COPY PREV");
        }
    }
    else if(role == Qt::SizeHintRole)
    {
        if( m_data.getFrameIndex() >= 0 && m_data.getFrameIndex() < parentspr->getImages().nodeChildCount() )
        {
            Image * pimg = parentspr->getImage(m_data.getFrameIndex());
            return (pimg->getImageSize());
//            return (pimg->getImageSize()).width();
        }
    }
    return QVariant();
}

QVariant MFramePart::dataImgId(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("Img#%1").arg(static_cast<int>(m_data.getFrameIndex()));
    }
    else if(role == Qt::EditRole)
    {
        //Just output image id
        return static_cast<int>(m_data.getFrameIndex());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataImgId(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 150 );
        //sz.setHeight(sz.height() + 72);
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataUnk0(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.unk0);
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataUnk0(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("(%1, %2)").arg(m_data.getXOffset()).arg(m_data.getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<int,int>(m_data.getXOffset(), m_data.getYOffset()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataOffset(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectXOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(m_data.getXOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(m_data.getXOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectXOffset(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectYOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(m_data.getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(m_data.getYOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectYOffset(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        QString flipval;
        if( !m_data.isVFlip() && !m_data.isHFlip() )
            flipval = "Not flipped";
        else
        {
            if(m_data.isVFlip())
                flipval += "V ";
            if(m_data.isHFlip())
                flipval += "H ";
        }
        return flipval;
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<bool,bool>(m_data.isVFlip(), m_data.isHFlip()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataFlip(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectVFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isVFlip();
    }
    else if(role == Qt::EditRole)
    {
        return m_data.isVFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectVFlip(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectHFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isHFlip();
    }
    else if(role == Qt::EditRole)
    {
        return m_data.isHFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectHFlip(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataRotNScaling(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isRotAndScalingOn();
    }
    else if(role == Qt::EditRole)
    {
        //#TODO: will need a custom struct here to properly send over RnS data!
        return m_data.isRotAndScalingOn();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRotNScaling(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataPaletteID(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getPalNb());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataPaletteID(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataPriority(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return MFrameDelegate::prioritiesNames().at(m_data.getPriority());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataPriority(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 50 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataTileNum(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getTileNum());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataTileNum(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataMosaic(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_data.isMosaicOn();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMosaic(Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 20);
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataMode(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return MFrameDelegate::modeNames().at(static_cast<int>(m_data.getObjMode()));
    }
    else if(role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getObjMode());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMode(Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 60);
        return sz;
    }
    return QVariant();
}
