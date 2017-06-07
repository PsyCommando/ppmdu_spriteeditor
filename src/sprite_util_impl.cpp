#include "sprite.h"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
#include <src/ppmdu/fmts/wa_sprite.hpp>

const char * ElemName_EffectOffset  = "Effect Offsets";
const char * ElemName_Palette       = "Palette";
const char * ElemName_Images        = "Images";
const char * ElemName_Image         = "Image";
const char * ElemName_FrameCnt      = "Frames";
const char * ElemName_Frame         = "Frame";
const char * ElemName_AnimSequence  = "Anim Sequence";
const char * ElemName_AnimSequences = "Anim Sequences";
const char * ElemName_AnimTable     = "Animation Table";
const char * ElemName_AnimGroup     = "Anim Group";
const char * ElemName_AnimFrame     = "Anim Frame";
const char * ElemName_FramePart     = "Frame Part";
const char * ElemName_SpriteProperty= "Property";

const QStringList CompressionFmtOptions
{
    "PKDPX",
    "AT4PX",
    "Uncompressed",
};


const int         SpritePropertiesNbCols  = static_cast<int>(eSpritePropColumns::NbColumns);
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
    "Unk#6",
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
    "##UNKNOWN## Mess with memory alignement of the sprite's tiles and of the next sprites loaded after.",
    "##UNKNOWN##",
    "##UNKNOWN##",
    "##UNKNOWN## Apparently a boolean value linked to animations.",
    "##UNKNOWN##",
    "##UNKNOWN## This far 0, 1, 3(d79p41a1.wan), 4(as001.wan).. Seems to deal with the palette slot in-game.",
    "##UNKNOWN##",
    "##UNKNOWN## Possibly VRAM Character Mapping. 0 = 2D Mapping(Tiles placed on a 32x32 matrix), 1 = 1D Mapping(Tiles loaded one after another).",
};

const int SpriteNbProperties = static_cast<int>(eSpriteProperties::NbProperties);


filetypes::eCompressionFormats CompOptionToCompFmt( eCompressionFmtOptions opt )
{
    switch(opt)
    {
    case eCompressionFmtOptions::PKDPX:
        return filetypes::eCompressionFormats::PKDPX;

    case eCompressionFmtOptions::AT4PX:
        return filetypes::eCompressionFormats::AT4PX;

    case eCompressionFmtOptions::NONE:
    default:
        return filetypes::eCompressionFormats::INVALID;
    };
}

eCompressionFmtOptions CompFmtToCompOption( filetypes::eCompressionFormats fmt )
{
    switch(fmt)
    {
    case filetypes::eCompressionFormats::PKDPX:
        return eCompressionFmtOptions::PKDPX;

    case filetypes::eCompressionFormats::AT4PX:
        return eCompressionFmtOptions::AT4PX;

    case filetypes::eCompressionFormats::INVALID:
    default:
        return eCompressionFmtOptions::NONE;
    };
}

const QStringList SpriteColorModes
{
  "16 colors",
  "256 colors",
  "bitmap",
};



//=================================================================================================================
//  SpritePropertiesHandler
//=================================================================================================================
SpritePropertiesModel::SpritePropertiesModel(Sprite *parentspr, QObject *parent)
    :QAbstractItemModel(parent), m_spr(parentspr)
{

}

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

int SpritePropertiesModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
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
    case Qt::SizeHintRole:
        {
            break;
        }
    default:
        {

        }
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
    };
    return QVariant();
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
    case eSpriteProperties::Unk6:
        return tr("Unk#6");
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
    case eSpriteProperties::Unk6:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_spr->unk6();
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
    case eSpriteProperties::Unk6:
        {
            m_spr->unk6(data.toUInt());
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

//=================================================================================================================
//  SpritePropertiesDelegate
//=================================================================================================================
SpritePropertiesDelegate::SpritePropertiesDelegate(Sprite *parentspr, QObject *parent)
    :QStyledItemDelegate(parent), m_spr(parentspr)
{

}

SpritePropertiesDelegate::~SpritePropertiesDelegate(){}

QSize SpritePropertiesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize ret = QStyledItemDelegate::sizeHint(option, index);
    if(index.column() == static_cast<int>(eSpritePropColumns::Value))
    {
        ret.setWidth( ret.width() + PropValueColLen);
    }
    return ret;
}

QWidget *SpritePropertiesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Qt takes ownership of the widget we create!
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
        return QStyledItemDelegate::createEditor(parent,option,index);

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = new QComboBox(parent);
            pspritetypes->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pspritetypes->setAutoFillBackground(true);
            pspritetypes->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

            for(const auto & s : fmt::SpriteTypeNames)
                pspritetypes->addItem(QString::fromStdString(s));
            pspritetypes->removeItem(fmt::SpriteTypeNames.size()-1); //remove the last item since we don't want to allow the user to pick it!
            return pspritetypes;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = new QComboBox(parent);
            pcompty->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pcompty->setAutoFillBackground(true);
            pcompty->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

            for(const auto & s : CompressionFmtOptions)
                pcompty->addItem(s);
            return pcompty;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = new QComboBox(parent);
            pcolmode->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pcolmode->setAutoFillBackground(true);
            pcolmode->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
            for(const auto & s : SpriteColorModes)
                pcolmode->addItem(s);
            return pcolmode;
        }
    case eSpriteProperties::Unk6:
    case eSpriteProperties::Unk7:
    case eSpriteProperties::Unk8:
    case eSpriteProperties::Unk9:
    case eSpriteProperties::Unk10:
    case eSpriteProperties::Unk11:
    case eSpriteProperties::Unk12:
    case eSpriteProperties::Unk13:
    default:
        {
            //nothing here
        }
    };

    return QStyledItemDelegate::createEditor(parent,option,index);
}

void SpritePropertiesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //Qt takes ownership of the widget we create!
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = static_cast<QComboBox*>(editor);
            Q_ASSERT(pspritetypes);
            pspritetypes->setCurrentIndex(static_cast<int>(m_spr->type()));
            return;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcompty);
            pcompty->setCurrentIndex( static_cast<int>(CompFmtToCompOption(m_spr->getTargetCompression())) );
            return;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcolmode);
            pcolmode->setCurrentIndex((m_spr->is256Colors())? 1 : 0);
            return;
        }
    case eSpriteProperties::Unk6:
    case eSpriteProperties::Unk7:
    case eSpriteProperties::Unk8:
    case eSpriteProperties::Unk9:
    case eSpriteProperties::Unk10:
    case eSpriteProperties::Unk11:
    case eSpriteProperties::Unk12:
    case eSpriteProperties::Unk13:
    default:
        {
            //nothing here
        }
    };
    QStyledItemDelegate::setEditorData(editor, index);
}

void SpritePropertiesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
    {
        QStyledItemDelegate::setModelData(editor,model,index);
        return;
    }

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = static_cast<QComboBox*>(editor);
            Q_ASSERT(pspritetypes);
            model->setData( index, pspritetypes->currentIndex() );
            return;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcompty);
            model->setData( index, pcompty->currentIndex() );
            return;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcolmode);
            model->setData(index, pcolmode->currentIndex());
            return;
        }
    case eSpriteProperties::Unk6:
    case eSpriteProperties::Unk7:
    case eSpriteProperties::Unk8:
    case eSpriteProperties::Unk9:
    case eSpriteProperties::Unk10:
    case eSpriteProperties::Unk11:
    case eSpriteProperties::Unk12:
    case eSpriteProperties::Unk13:
    default:
        {
            //nothing here
        }
    };
    QStyledItemDelegate::setModelData(editor,model,index);
}

void SpritePropertiesDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}


//=================================================================================================================
//  SpriteOverviewModel
//=================================================================================================================
const QVector<SpriteOverviewModel::stats_t> SpriteOverviewModel::StatEntries
{
    {"Nb Images",       [](Sprite* spr){return spr->getImages().nodeChildCount();} },
    {"Nb Frames",       [](Sprite* spr){return spr->getFrames().nodeChildCount();} },
    {"Nb Sequences",    [](Sprite* spr){return spr->getAnimSequences().nodeChildCount();} },
    {"Nb Animations",   [](Sprite* spr){return spr->getAnimTable().nodeChildCount();} },
    {"Palette",         [](Sprite* spr)
        {
            if(!spr->getPalette().empty())
                return QString("%1 colors").arg( spr->getPalette().size() );
            else
                return QString("none");
        } },
    {"Size",            [](Sprite* spr){return spr->getRawData().size();} },
};

SpriteOverviewModel::SpriteOverviewModel(Sprite *spr)
    :QAbstractItemModel(nullptr), m_spr(spr)
{}

QModelIndex SpriteOverviewModel::index(int row, int, const QModelIndex &parent) const
{
    if( !parent.isValid() )
        return QModelIndex();

    return createIndex( row, 0, row );
}

QModelIndex SpriteOverviewModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int SpriteOverviewModel::rowCount(const QModelIndex &parent) const
{
    if( !parent.isValid() )
        return StatEntries.size();
    else
        return 0;
}

int SpriteOverviewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant SpriteOverviewModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if(index.internalId() >= 0 && index.internalId() < StatEntries.size())
        return StatEntries.at(index.internalId()).second(m_spr);

    return QVariant();
}

QVariant SpriteOverviewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole || orientation != Qt::Orientation::Vertical )
        return QVariant();

    if(section >= 0 && section < StatEntries.size())
        return StatEntries.at(section).first;

    return QVariant();
}
