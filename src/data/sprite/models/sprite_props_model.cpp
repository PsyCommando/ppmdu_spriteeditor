#include "sprite_props_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/sprite_props_handler.hpp>
#include <src/data/sprite/models/sprite_props_delegate.hpp>

const QStringList SpritePropertiesColNames
{
    QString("Value"),
    QString("Description"),
};

const QStringList SpritePropertiesNames
{
    "Sprite Type",
    "Compression",
    "Color Mode",
    "Unk#7",
    "Unk#8",
    "Unk#9",
    "Unk#10",
    "Unk#11",
    "Unk#12",
    "Unk#13",
};

const QStringList SpritePropertiesDescriptions
{
    "**Use with caution!** Setting this will change the sprite type and its content!\n It will remove superflous data if you change an existing sprite's type!",
    "Sets the compression used on the sprite container when written to file. Its advised not to changed this unless you know what you're doing!",
    "If set to 256 the game will attempt to treat the content of the sprite as 256 colors/8bpp images. Otherwise, treats it as 4bpp/16colors images.",
    "##UNKNOWN##",
    "##UNKNOWN##",
    "##UNKNOWN## Apparently a boolean value linked to animations.",
    "##UNKNOWN##",
    "##UNKNOWN## This far 0, 1, 3(d79p41a1.wan), 4(as001.wan).. Seems to deal with the palette slot in-game.",
    "##UNKNOWN##",
    "##UNKNOWN## Possibly VRAM Character Mapping. 0 = 2D Mapping(Tiles placed on a 32x32 matrix), 1 = 1D Mapping(Tiles loaded one after another).",
};

const QString   ElemName_SpriteProperty = "Property";
const int       SpritePropertiesNbCols  = static_cast<int>(eSpritePropColumns::NbColumns);
const int       SpriteNbProperties      = static_cast<int>(eSpriteProperties::NbProperties);


//=================================================================================================================
//  SpritePropertiesModel
//=================================================================================================================
SpritePropertiesModel::SpritePropertiesModel(Sprite *parentspr, QObject *parent)
    :QAbstractItemModel(parent), m_spr(parentspr)
{}

SpritePropertiesModel::~SpritePropertiesModel(){}

int SpritePropertiesModel::getNbProperties() const
{
    return SpriteNbProperties;
}

Qt::ItemFlags SpritePropertiesModel::flags(const QModelIndex &index) const
{
    if( index.column() == static_cast<int>(eSpritePropColumns::Value) )
    {
        return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}

QModelIndex SpritePropertiesModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid())
        return QModelIndex();
    return createIndex(row, column,  nullptr);
}

QModelIndex SpritePropertiesModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int SpritePropertiesModel::rowCount(const QModelIndex &/*parent*/) const
{
    return getNbProperties();
}

int SpritePropertiesModel::columnCount(const QModelIndex &) const
{
    return SpritePropertiesNbCols;
}

bool SpritePropertiesModel::hasChildren(const QModelIndex &parent) const
{
    if(parent.isValid())
        return false;
    return true;
}

QVariant SpritePropertiesModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        {
            return dataDisplay(index.row(), index.column());
        }
    case Qt::EditRole:
        {
            if(index.column() == static_cast<int>(eSpritePropColumns::Value))
                return getDataForProperty(index.row(), Qt::EditRole);
            break;
        }
    case Qt::SizeHintRole: [[fallthrough]];
    default:
        break;
    };
    return QVariant();
}

bool SpritePropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::EditRole && index.column() == static_cast<int>(eSpritePropColumns::Value))
    {
        setDataForProperty(static_cast<eSpriteProperties>(index.row()), value);
    }
    return false;
}

QVariant SpritePropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return SpritePropertiesColNames.at(section);
    else if(role == Qt::DisplayRole && orientation == Qt::Vertical)
    {
        return getNameForProperty(section);
    }
    return QVariant();
}

QVariant SpritePropertiesModel::dataDisplay(int propid, int column) const
{
    switch(static_cast<eSpritePropColumns>(column))
    {
    case eSpritePropColumns::Value:
        return getDataForProperty(propid, Qt::DisplayRole);
    case eSpritePropColumns::Description:
        return getDescForProperty(propid);
    default:
        return QVariant();
    };
}

QVariant SpritePropertiesModel::getNameForProperty(int propid) const
{
    switch(static_cast<eSpriteProperties>(propid))
    {
    case eSpriteProperties::SpriteType:
        return tr("Sprite Type");
    case eSpriteProperties::Compression:
        return tr("Compression");
    case eSpriteProperties::ColorMode:
        return tr("Color Mode");
    case eSpriteProperties::Unk7:
        return tr("Unk#7");
    case eSpriteProperties::Unk8:
        return tr("Unk#8");
    case eSpriteProperties::Unk9:
        return tr("Unk#9");
    case eSpriteProperties::Unk10:
        return tr("Unk#10");
    case eSpriteProperties::Unk11:
        return tr("Unk#11");
    case eSpriteProperties::Unk12:
        return tr("Unk#12");
    case eSpriteProperties::Unk13:
        return tr("Unk#13");
    default:
        Q_ASSERT(false);
    };
    return QVariant();
}

QVariant SpritePropertiesModel::getDataForProperty(int propid, int role) const
{

    switch(static_cast<eSpriteProperties>(propid))
    {
    case eSpriteProperties::SpriteType:
        {
            if(role == Qt::DisplayRole)
                return QString::fromStdString( fmt::SpriteTypeNames.at(static_cast<size_t>(m_spr->type())) );
            else if(role == Qt::EditRole)
                return static_cast<int>(m_spr->type());
            break;
        }
    case eSpriteProperties::Compression:
        {
            eCompressionFmtOptions translated = CompFmtToCompOption(m_spr->getTargetCompression());
            if(role == Qt::DisplayRole)
                return CompressionFmtOptions.at(static_cast<size_t>(translated));
            else if(role == Qt::EditRole)
                return static_cast<int>(translated);
            break;
        }
    case eSpriteProperties::ColorMode:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                return SpriteColorModes.at( m_spr->is256Colors() ? 1 : 0 );
            }
            break;
        }
    case eSpriteProperties::Unk7:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk7();
            break;
        }
    case eSpriteProperties::Unk8:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk8();
            break;
        }
    case eSpriteProperties::Unk9:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk9();
            break;
        }
    case eSpriteProperties::Unk10:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk10();
            break;
        }
    case eSpriteProperties::Unk11:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk11();
            break;
        }
    case eSpriteProperties::Unk12:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk12();
            break;
        }
    case eSpriteProperties::Unk13:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk13();
            break;
        }
    default:
        Q_ASSERT(false);
    };
    return QVariant();
}

QVariant SpritePropertiesModel::getDescForProperty(int propid) const
{
    return QVariant(SpritePropertiesDescriptions.at(propid));
}

void SpritePropertiesModel::setDataForProperty(eSpriteProperties propid, const QVariant &data)
{
    switch(propid)
    {
    case eSpriteProperties::SpriteType:
        {
            m_spr->convertSpriteToType(static_cast<fmt::eSpriteType>(data.toInt()));
            break;
        }
    case eSpriteProperties::Compression:
        {
            m_spr->setTargetCompression(CompOptionToCompFmt( static_cast<eCompressionFmtOptions>(data.toInt()) ) );
            break;
        }
    case eSpriteProperties::ColorMode:
        {
            m_spr->setIs256Colors(data.toBool());
            break;
        }
    case eSpriteProperties::Unk7:
        {
            m_spr->unk7(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk8:
        {
            m_spr->unk8(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk9:
        {
            m_spr->unk9(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk10:
        {
            m_spr->unk10(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk11:
        {
            m_spr->unk11(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk12:
        {
            m_spr->unk12(data.toUInt());
            break;
        }
    case eSpriteProperties::Unk13:
        {
            m_spr->unk13(data.toUInt());
            break;
        }
    default:
        Q_ASSERT(false);
    };
}

//SpritePropertiesModel *Sprite::model()
//{
//    if(!m_propshndlr)
//        return nullptr;
//    return m_propshndlr->model();
//}

//const SpritePropertiesModel *Sprite::model() const
//{
//    if(!m_propshndlr)
//        return nullptr;
//    return m_propshndlr->model();
//}

