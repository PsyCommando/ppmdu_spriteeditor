#include "framepart_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/ppmdu/utils/imgutils.hpp>

MFramePartModel::MFramePartModel(MFrame *pmfrm, Sprite *pspr)
    :TreeNodeModel(nullptr)
{
    m_root = pmfrm;
    m_sprite = pspr;
}

MFramePartModel::~MFramePartModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

int MFramePartModel::columnCount(const QModelIndex &parent) const
{
    return static_cast<int>(eFramePartColumnsType::HeaderNBColumns);
}

QVariant MFramePartModel::data(const QModelIndex &index, int role) const
{
//    if (!index.isValid())
//        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const MFramePart *part = static_cast<const MFramePart*>(getItem(index));
    if(!part)
        return QVariant();
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::Preview:       return dataImgPreview        (part, role);
    case eFramePartColumnsType::ImgID:         return dataImgId             (part, role);
    case eFramePartColumnsType::TileNum:       return dataTileNum           (part, role);
    case eFramePartColumnsType::PaletteID:     return dataPaletteID         (part, role);
    case eFramePartColumnsType::Unk0:          return dataUnk0              (part, role);
    case eFramePartColumnsType::Offset:        return dataOffset            (part, role);
    case eFramePartColumnsType::Flip:          return dataFlip              (part, role);
    case eFramePartColumnsType::RotNScaling:   return dataRotNScaling       (part, role);
    case eFramePartColumnsType::Mosaic:        return dataMosaic            (part, role);
    case eFramePartColumnsType::Mode:          return dataMode              (part, role);
    case eFramePartColumnsType::Priority:      return dataPriority          (part, role);
    //For direct access to the individual values that are grouped together, not displayed by the model!!!
    case eFramePartColumnsType::direct_VFlip:   return dataDirectVFlip      (part, role);
    case eFramePartColumnsType::direct_HFlip:   return dataDirectHFlip      (part, role);
    case eFramePartColumnsType::direct_XOffset: return dataDirectXOffset    (part, role);
    case eFramePartColumnsType::direct_YOffset: return dataDirectYOffset    (part, role);

        //Undefined cases
    default:
        break;
    };
    return QVariant();
}

bool MFramePartModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || role != Qt::EditRole)
        return false;

    MFramePart* pfpart = static_cast<MFramePart*>(getRootNode()->nodeChild(index.row()));
    fmt::step_t& part = pfpart->getPartData();
    Q_ASSERT(pfpart);
    bool bok = false;

    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::ImgID:
        {
            part.setFrameIndex(value.toInt(&bok));
            break;
        }
    case eFramePartColumnsType::Offset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                part.setXOffset(offs.first);
                part.setYOffset(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert offset value to a QPair!\n");
            break;
        }
    case eFramePartColumnsType::Flip:
        {
            bok = value.canConvert<QPair<bool,bool>>();
            if(bok)
            {
                QPair<bool,bool> offs = value.value<QPair<bool,bool>>();
                part.setVFlip(offs.first);
                part.setHFlip(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert flip value to a QPair!\n");
            break;
        }
    case eFramePartColumnsType::RotNScaling:
        {
            part.setRotAndScaling(value.toBool());
            bok = true; //gotta set this to true because toBool doesn't return success or not
            break;
        }
    case eFramePartColumnsType::PaletteID:
        {
            part.setPalNb(value.toInt(&bok));
            break;
        }
    case eFramePartColumnsType::Mode:
        {
            part.setObjMode(static_cast<fmt::step_t::eObjMode>(value.toInt(&bok)));
            break;
        }
    case eFramePartColumnsType::Priority:
        {
            part.setPriority(value.toInt(&bok));
            break;
        }
    case eFramePartColumnsType::TileNum:
        {
            part.setTileNum(value.toInt(&bok));
            break;
        }

    //direct access columns
    case eFramePartColumnsType::direct_HFlip:
        {
            part.setHFlip(value.toBool());
            break;
        }
    case eFramePartColumnsType::direct_VFlip:
        {
            part.setVFlip(value.toBool());
            break;
        }
    case eFramePartColumnsType::direct_XOffset:
        {
            part.setXOffset(value.toUInt(&bok));
            break;
        }
    case eFramePartColumnsType::direct_YOffset:
        {
            part.setYOffset(value.toUInt(&bok));
            break;
        }
        //Undefined cases
    //case eFramePartColumnsType::Preview:
    //case eFramePartColumnsType::TotalSize:
    //case eFramePartColumnsType::Unk0:
    default:
        return false;
    };

    if(bok && index.model())
    {
        emit dataChanged(index, index, QVector<int>{role});
    }
    return bok;
}

QVariant MFramePartModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || static_cast<size_t>(section) >= FramePartHeaderColumnNames.size() )
        return QVariant();

    if( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Orientation::Horizontal)
            return FramePartHeaderColumnNames[section];
    }
    else if(role == Qt::SizeHintRole)
    {
        if( orientation == Qt::Orientation::Horizontal)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.horizontalAdvance(FramePartHeaderColumnNames[section])+4, fm.height()+4);
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
    if(role == Qt::DecorationRole)
    {
        //#TODO: Draw only this part/step
        if( part->getFrameIndex() >= 0 && part->getFrameIndex() < m_sprite->getImages().nodeChildCount() )
        {
            Image * pimg = m_sprite->getImage(part->getFrameIndex());
            if(!pimg)
                throw std::runtime_error(QString("MFrame::dataImgPreview(): Invalid image at index %1!\n").arg(part->getFrameIndex()).toStdString());
            else
                return QVariant(pimg->makeImage(m_sprite->getPalette()));
        }
        else
        {
            //#TODO: draw last step
            return QVariant("COPY PREV");
        }
    }
    else if(role == Qt::SizeHintRole)
    {
        if( part->getFrameIndex() >= 0 && part->getFrameIndex() < m_sprite->getImages().nodeChildCount() )
        {
            Image * pimg = m_sprite->getImage(part->getFrameIndex());
            return (pimg->getImageSize());
//            return (pimg->getImageSize()).width();
        }
    }
    return QVariant();
}

QVariant MFramePartModel::dataImgId(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
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

QVariant MFramePartModel::dataOffset(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("(%1, %2)").arg(part->getXOffset()).arg(part->getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<int,int>(part->getXOffset(), part->getYOffset()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataOffset(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataDirectXOffset(const MFramePart * part, int role) const
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
        QSize sz = calcTextSize(dataDirectXOffset(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataDirectYOffset(const MFramePart * part, int role) const
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
        QSize sz = calcTextSize(dataDirectYOffset(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataFlip(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        QString flipval;
        if( !part->isVFlip() && !part->isHFlip() )
            flipval = "";
        else
        {
            if(part->isVFlip())
                flipval += "V ";
            if(part->isHFlip())
                flipval += "H ";
        }
        return flipval;
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<bool,bool>(part->isVFlip(), part->isHFlip()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataFlip(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataDirectVFlip(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return part->isVFlip();
    }
    else if(role == Qt::EditRole)
    {
        return part->isVFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectVFlip(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataDirectHFlip(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return part->isHFlip();
    }
    else if(role == Qt::EditRole)
    {
        return part->isHFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectHFlip(part, Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataRotNScaling(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole)
    {
        return part->isRotAndScalingOn();
    }
    else if(role == Qt::EditRole)
    {
        //#TODO: will need a custom struct here to properly send over RnS data!
        return part->isRotAndScalingOn();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRotNScaling(part, Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePartModel::dataPaletteID(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(part->getPalNb());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataPaletteID(part, Qt::DisplayRole).toString());
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

QVariant MFramePartModel::dataTileNum(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(part->getTileNum());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataTileNum(part, Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePartModel::dataMosaic(const MFramePart * part, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return part->isMosaicOn();
    }
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
