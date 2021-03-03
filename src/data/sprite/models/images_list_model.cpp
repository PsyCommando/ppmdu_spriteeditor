#include "images_list_model.hpp"
#include <src/data/sprite/image.hpp>
#include <src/data/sprite/imagescontainer.hpp>
#include <src/data/sprite/sprite.hpp>

const std::map<ImageListModel::eColumns, QString> ImageListModel::ColumnNames
{
    {ImageListModel::eColumns::Preview,     "Preview"},
    {ImageListModel::eColumns::UID,         "UID"},
    {ImageListModel::eColumns::Depth,       "Bit Depth"},
    {ImageListModel::eColumns::Resolution,  "Resolution"},
    //The names for the extra values aren't here, because they're not displayed
};

ImageListModel::ImageListModel(ImageContainer* parent, Sprite* psprite)
    :TreeNodeModel(nullptr), m_root(parent), m_sprite(psprite)
{
}

ImageListModel::~ImageListModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

int ImageListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return static_cast<int>(ColumnNames.size());
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const Image *img = static_cast<const Image*>(getItem(index));
    QVariant res;

    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::Preview: //preview
        {
            if( role == Qt::DecorationRole )
                res.setValue(img->makeImage(getOwnerSprite()->getPalette()));
            else if( role == Qt::SizeHintRole )
            {
                QSize imgsz = img->getImageSize();
                imgsz.scale(32, 32, Qt::AspectRatioMode::KeepAspectRatio);
                res = imgsz;
            }
            break;
        }
        case eColumns::UID:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getImageUID());
            break;
        }
        case eColumns::Depth: //depth
        {
            if( role == Qt::DisplayRole )
                res.setValue(QString("%1bpp").arg(static_cast<int>(img->getImageOriginalDepth())));
            break;
        }
        case eColumns::Resolution: //resolution
        {
            if( role == Qt::DisplayRole )
            {
                const QSize imgsz = img->getImageSize();
                res.setValue(QString("%1x%2").arg(imgsz.width()).arg(imgsz.height()));
            }
            break;
        }
        case eColumns::direct_Unk2:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getUnk2());
            break;
        }
        case eColumns::direct_Unk14:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                res.setValue(img->getUnk14());
            break;
        }
        default:
            break; //Get rid of warnings
    };
    return res;
}

bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(hasChildren(QModelIndex()));
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    Image *img = static_cast<Image*>(getItem(index));
    bool  succ = false;

    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::direct_Unk2:
        {
            using unk2t = decltype(img->getUnk2());
            unk2t res = static_cast<unk2t>(value.toUInt(&succ));
            if(succ)
                img->setUnk2(res);
            break;
        }
        case eColumns::direct_Unk14:
        {
            using unk14t = decltype(img->getUnk14());
            unk14t res = static_cast<unk14t>(value.toUInt(&succ));
            if(succ)
                img->setUnk14(res);
            break;
        }
        case eColumns::Preview:
        case eColumns::UID:
        case eColumns::Depth:
        case eColumns::Resolution:
        default: //To get rid of warnings
            succ = false;
            break;
    };

    emit dataChanged(index, index, {role});
    return succ;
}

QVariant ImageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return QString("%1").arg(section);
    }
    else if( orientation == Qt::Orientation::Horizontal )
    {
        eColumns col = static_cast<eColumns>(section);
        auto itcol = ColumnNames.find(col);
        if(itcol != ColumnNames.end())
            return itcol->second;
    }
    return QVariant();
}

TreeNodeModel::node_t *ImageListModel::getRootNode()
{
    return m_root;
}

Sprite *ImageListModel::getOwnerSprite()
{
    return m_sprite;
}

QVariant ImageListModelCondensed::data(const QModelIndex &index, int role) const
{
    const Image* img = static_cast<const Image*>(getItem(index));
    QVariant res;
    if( role == Qt::DecorationRole )
        res.setValue(img->makeImage(getOwnerSprite()->getPalette()));
    else if( role == Qt::DisplayRole )
    {
        QSize resolution = img->getImageSize();
        QString condensed = "ID:%1 %2bpp %3x%4";
        condensed = condensed
                .arg(img->nodeIndex())
                .arg(static_cast<int>(img->getImageOriginalDepth()))
                .arg(resolution.width())
                .arg(resolution.height());
        res = condensed;
    }
    else if(role == Qt::EditRole)
        res.setValue(img->nodeIndex());
    return res;
}
