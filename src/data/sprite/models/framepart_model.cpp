#include "framepart_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/utility/program_settings.hpp>

MFramePartModel::MFramePartModel(MFrame *pmfrm, Sprite *pspr)
    :TreeNodeModel(nullptr)
{
    m_root = pmfrm;
    m_sprite = pspr;
}

MFramePartModel::~MFramePartModel()
{
}

int MFramePartModel::columnCount(const QModelIndex &/*parent*/) const
{
    if(ProgramSettings::Instance().isAdvancedMode())
        return static_cast<int>(eFramePartColumnsType::NBColumns);
    else
        return static_cast<int>(eFramePartColumnsType::LastColumnsBasicMode) + 1; //Add one since its a count
}

QVariant MFramePartModel::data(const QModelIndex &index, int role) const
{
    const MFramePart *part = static_cast<const MFramePart*>(getItem(index));
    if(!part)
        return QVariant();
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::Preview:        return dataImgPreview   (part, role);
    case eFramePartColumnsType::ImgID:          return dataImgId        (part, role);
    case eFramePartColumnsType::ImgSz:          return dataImgSize      (part, role);
    case eFramePartColumnsType::BlockRange:      return dataBlockRange    (part, role);
    case eFramePartColumnsType::PaletteID:      return dataPaletteID    (part, role);
    case eFramePartColumnsType::VFlip:          return dataVFlip        (part, role);
    case eFramePartColumnsType::HFlip:          return dataHFlip        (part, role);
    case eFramePartColumnsType::XOffset:        return dataXOffset      (part, role);
    case eFramePartColumnsType::YOffset:        return dataYOffset      (part, role);
    case eFramePartColumnsType::Priority:       return dataPriority     (part, role);

    //Advanced stuff
    case eFramePartColumnsType::BlockNum:       return dataBlockNum     (part, role);
    case eFramePartColumnsType::Mosaic:         return dataMosaic       (part, role);
    case eFramePartColumnsType::Mode:           return dataMode         (part, role);

    //RnS stuff
    case eFramePartColumnsType::RnS:            return dataRnS          (part, role);
    case eFramePartColumnsType::RnSCanvasRot:   return dataRotCanvas    (part, role);
    case eFramePartColumnsType::RnSParam:       return dataRnSParam     (part, role);

    //Debug stuff
    case eFramePartColumnsType::Unk0:           return dataUnk0         (part, role);
    default:
        break; //Undefined cases
    };

    return QVariant();
}

bool MFramePartModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || (role != Qt::EditRole && role != Qt::CheckStateRole))
        return false;

    MFramePart* pfpart = static_cast<MFramePart*>(getRootNode()->nodeChild(index.row()));
    fmt::step_t& part = pfpart->getPartData();
    Q_ASSERT(pfpart);
    bool bok = false;

    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
        case eFramePartColumnsType::ImgID:
        {
            if(role == Qt::EditRole)
            {
                const fmt::frmid_t imgid = value.toInt(&bok);
                if(!bok)
                    break;
                part.setFrameIndex(imgid);
            }
            break;
        }
        case eFramePartColumnsType::PaletteID:
        {
            if(role == Qt::EditRole)
            {
                const unsigned int palid = value.toInt(&bok);
                if(!bok)
                    break;
                part.setPalNb(palid);
            }
            break;
        }
        case eFramePartColumnsType::Mode:
        {
            if(role == Qt::EditRole)
            {
                const fmt::step_t::eObjMode mode = static_cast<fmt::step_t::eObjMode>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setObjMode(mode);
            }
            break;
        }
        case eFramePartColumnsType::Mosaic:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setMosaicOn(chkst == Qt::CheckState::Checked);
            }
            break;
        }
        case eFramePartColumnsType::ImgIsRef:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                if(chkst)
                    part.setFrameIndex(-1);
                else
                    part.setFrameIndex(0); //That's a biiit hacky tbh
            }
            break;
        }
        case eFramePartColumnsType::Priority:
        {
            if(role == Qt::EditRole)
            {
                const int priority = value.toInt(&bok);
                if(!bok)
                    break;
                part.setPriority(priority);
            }
            break;
        }
        case eFramePartColumnsType::BlockNum:
        {
            if(role == Qt::EditRole)
            {
                const int tileid = value.toInt(&bok);
                if(!bok)
                    break;
                part.setBlockNum(tileid);
            }
            break;
        }
        case eFramePartColumnsType::HFlip:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setHFlip(chkst == Qt::CheckState::Checked);
            }
            break;
        }
        case eFramePartColumnsType::VFlip:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setVFlip(chkst == Qt::CheckState::Checked);
            }
            break;
        }
        case eFramePartColumnsType::XOffset:
        {
            if(role == Qt::EditRole)
            {
                const unsigned int x = value.toUInt(&bok);
                if(!bok)
                    break;
                part.setXOffset(x);
            }
            break;
        }
        case eFramePartColumnsType::YOffset:
        {
            if(role == Qt::EditRole)
            {
                const unsigned int y = value.toUInt(&bok);
                if(!bok)
                    break;
                part.setYOffset(y);
            }
            break;
        }
        case eFramePartColumnsType::RnSParam:
        {
            if(role == Qt::EditRole)
            {
                const uint8_t param = value.toUInt(&bok);
                if(!bok)
                    break;
                part.setRnSParam(param);
            }
            break;
        }
        case eFramePartColumnsType::RnS:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setRotAndScaling(chkst == Qt::CheckState::Checked);
            }
            break;
        }
        case eFramePartColumnsType::RnSCanvasRot:
        {
            if(role == Qt::CheckStateRole)
            {
                const Qt::CheckState chkst = static_cast<Qt::CheckState>(value.toInt(&bok));
                if(!bok)
                    break;
                part.setRnSCanvasRot(chkst == Qt::CheckState::Checked);
            }
            break;
        }
        case eFramePartColumnsType::BlockRange:
        {
            if(role == Qt::EditRole)
            {
                const QPair<uint16_t, uint16_t> blk = value.value<QPair<uint16_t, uint16_t>>();
                part.setBlockNum(blk.first);
            }
            break;
        }
        default:
            break;
    };

    if(bok && index.model())
    {
        emit dataChanged(index, index, QVector<int>{role});
    }
    return bok;
}

QVariant MFramePartModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || section >= columnCount(QModelIndex()))
        return QVariant();

    const eFramePartColumnsType columnkey = static_cast<eFramePartColumnsType>(section);
    if( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Orientation::Horizontal)
            return FramePartHeaderColumnNames.at(columnkey);
    }
    else if(role == Qt::SizeHintRole)
    {
        if( orientation == Qt::Orientation::Horizontal)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.horizontalAdvance(FramePartHeaderColumnNames.at(columnkey))+4, fm.height()+4);
        }
    }
    return QVariant();
}

TreeNodeModel::node_t *MFramePartModel::getRootNode()
{
    return m_root;
}

Sprite *MFramePartModel::getOwnerSprite()
{
    return m_sprite;
}

QVariant MFramePartModel::dataImgPreview(const MFramePart * part, int role) const
{
    if(role != Qt::DecorationRole && role != Qt::SizeHintRole)
        return QVariant();

    QImage imgpart = part->drawPart(m_sprite);
    QVariant result;
    if(role == Qt::DecorationRole)
        result = QVariant(imgpart);
    else if(role == Qt::SizeHintRole)
        result = imgpart.size();
    return result;
}

QVariant MFramePartModel::dataImgId(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(part->isPartReference())
            return QString(tr("Block Reference"));
        else
            return QString("Img#%1").arg(static_cast<int>(part->getFrameIndex()));
    }
    else if(role == Qt::EditRole)
    {
        //Just output image id
        return static_cast<int>(part->getFrameIndex());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataImgId(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 150 );
        //sz.setHeight(sz.height() + 72);
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataImgIsRef(const MFramePart *part, int role) const
{
    if(role == Qt::CheckStateRole)
        return part->isPartReference()? Qt::Checked : Qt::Unchecked;
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataImgIsRef(part, Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 20);
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataBlockRange(const MFramePart *part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("[%L1 - %L2[").arg(static_cast<int>(part->getBlockNum())).arg(static_cast<int>(part->getBlockNum() + part->getBlockLen()));
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataBlockNum(part, Qt::DisplayRole).toString());
    }
    else if(role == Qt::EditRole)
    {
        QVariant ret;
        ret.setValue(QPair<uint16_t,uint16_t>(part->getBlockNum(), part->getBlockNum() + part->getBlockLen()));
        return ret;
    }
    return QVariant();
}

QVariant MFramePartModel::dataImgSize(const MFramePart * part, int role)const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        auto res = part->GetResolution();
        return QString("%1x%2 (%3 blocks)").arg(res.first).arg(res.second).arg(part->getBlockLen());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataImgSize(part, Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 16);
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataUnk0(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(part->getPartData().unk0);
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataUnk0(part, Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePartModel::dataXOffset(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(part->getXOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(part->getXOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataXOffset(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataYOffset(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(part->getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(part->getYOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataYOffset(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataVFlip(const MFramePart * part, int role) const
{
    if(role == Qt::CheckStateRole)
        return part->isVFlip()? Qt::Checked : Qt::Unchecked;
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataVFlip(part, Qt::CheckStateRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataHFlip(const MFramePart * part, int role) const
{
    if(role == Qt::CheckStateRole)
        return part->isHFlip()? Qt::Checked : Qt::Unchecked;
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataHFlip(part, Qt::CheckStateRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataRnSParam(const MFramePart *part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(part->isRotAndScalingOn())
            return part->getRnSParam();
        else
            return QString{tr("Disabled")};
    }
    else if(role == Qt::EditRole)
    {
        if(part->isRotAndScalingOn())
            return part->getRnSParam();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRnSParam(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 16 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataRotCanvas(const MFramePart *part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(!part->isRotAndScalingOn())
            return QString{tr("Disabled")};
    }
    else if(role == Qt::CheckStateRole)
    {
        if(part->isRotAndScalingOn())
            return part->isRnSRotCanvas()? Qt::Checked : Qt::Unchecked;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRnS(part, Qt::CheckStateRole).toString());
        sz.setWidth( sz.width() + 16 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataRnS(const MFramePart * part, int role) const
{
    if(role == Qt::CheckStateRole)
    {
        return part->isRotAndScalingOn()? Qt::Checked : Qt::Unchecked;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRnS(part, Qt::CheckStateRole).toString());
        sz.setWidth( sz.width() + 16 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataPaletteID(const MFramePart * part, int role) const
{
    if(role == Qt::DecorationRole)
    {
        if(part->isColorPal256())
            return QVariant(); //Don't display the whole thing
        else
            return m_sprite->MakePreviewSubPalette(part->getPalNb());
    }
    if(role == Qt::DisplayRole)
    {
        if(part->isColorPal256())
            return QVariant();
        else
            return QString("Palette #%1").arg(part->getPalNb());
    }
    else if(role == Qt::EditRole)
    {
        return static_cast<int>(part->getPalNb());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize txtsz = calcTextSize(dataPaletteID(part, Qt::DisplayRole).toString());
        txtsz.setWidth(txtsz.width() + 256);
        return txtsz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataPriority(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return FRAME_PART_PRIORITY_NAMES.at(part->getPriority());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataPriority(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 50 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataBlockNum(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString(tr("block %1")).arg(static_cast<int>(part->getBlockNum()));
    }
    else if(role == Qt::EditRole)
    {
        return static_cast<int>(part->getBlockNum());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataBlockNum(part, Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePartModel::dataMosaic(const MFramePart * part, int role) const
{
    if(role == Qt::CheckStateRole)
        return part->isMosaicOn()? Qt::Checked : Qt::Unchecked;
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMosaic(part, Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 20);
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataMode(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return FRAME_PART_MODE_NAMES.at(static_cast<int>(part->getObjMode()));
    }
    else if(role == Qt::EditRole)
    {
        return static_cast<int>(part->getObjMode());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMode(part, Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 60);
        return sz;
    }
    return QVariant();
}

QSize MFramePartModel::calcTextSize(const QString &text)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.horizontalAdvance(text), fm.height());
}
