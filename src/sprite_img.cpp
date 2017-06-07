#include "sprite_img.hpp"
#include <QImage>
#include <QVariant>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPixmap>
#include <QFontMetrics>

#include <src/sprite.h>

//
//
//
const size_t               FramesHeaderNBColumns = static_cast<unsigned int>(eFramesColumnsType::HeaderNBColumns);
const std::vector<QString> FramesHeaderColumnNames
{
    QString(("")),

    //The rest below is for the parts/step to assemble the frame!
    QString(("Img ID")),
    QString(("Unk#0")),
    QString(("Offset")),
    QString(("Flip")),
    QString(("Rotation & Scaling")),
    QString(("Palette")),
    QString(("Priority")),
    QString(("Char Name")),
};

const char * MFrame::PropPartID = "framePartID";


Sprite * EffectOffsetContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

//
//
//


Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parentNode())->parentSprite();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

QVariant Image::imgData(int column, int role)const
{
    QVariant res;
    switch(column)
    {
    case 0: //preview
        if( role == Qt::DecorationRole )
            res.setValue(makeImage(parentSprite()->getPalette()));
        else if( role == Qt::SizeHintRole )
            res.setValue( QSize(m_img.size().width() *2, m_img.size().height() *2) );
        break;
    case 1:
        if( role == Qt::DisplayRole || role == Qt::EditRole )
            res.setValue(getImageUID());
        break;
    case 2: //depth
        res.setValue(QString("%1bpp").arg(m_depth));
        break;
    case 3: //resolution
        res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
        break;
    };
    return std::move(res);
}

Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}







//
//
//
ImagesManager::ImagesManager(ImageContainer *parent)
    :QAbstractItemModel(), m_parentcnt(parent)
{}

ImagesManager::~ImagesManager()
{
    qDebug("ImagesManager::~ImagesManager()\n");
}

QModelIndex ImagesManager::index(int row, int column, const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    TreeElement *childItem  = parentItem->nodeChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ImagesManager::parent(const QModelIndex &child) const
{
    TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
    TreeElement *parentItem = childItem->parentNode();
    Q_ASSERT(parentItem != nullptr);

    if (parentItem == m_parentcnt)
        return QModelIndex();

    return createIndex(parentItem->nodeIndex(), 0, parentItem);
}

int ImagesManager::rowCount(const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    return parentItem->nodeChildCount();
}

int ImagesManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
    else
        return m_parentcnt->nodeColumnCount();
}

bool ImagesManager::hasChildren(const QModelIndex &parent) const
{
    TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    if(parentItem)
        return parentItem->nodeChildCount() > 0;
    else
        return false;
}

QVariant ImagesManager::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(hasChildren(QModelIndex()));
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
    return img->imgData(index.column(), role);
}

QVariant ImagesManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal )
    {
        switch(section)
        {
        case 0:
            return std::move(QVariant( QString("") ));
        case 1:
            return std::move(QVariant( QString("Bit Depth") ));
        case 2:
            return std::move(QVariant( QString("Resolution") ));
        };
    }
    return QVariant();
}

bool ImagesManager::insertRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildrenNodes(row, count);
    endInsertRows();

    return success;
}

bool ImagesManager::removeRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildrenNodes(row, count);
    endRemoveRows();

    return success;
}

bool ImagesManager::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    Q_ASSERT(false);
    return false;
}

TreeElement *ImagesManager::getItem(const QModelIndex &index)
{
    if (index.isValid())
    {
        TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_parentcnt;
}



