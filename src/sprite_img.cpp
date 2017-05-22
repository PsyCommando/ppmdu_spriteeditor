#include "sprite_img.hpp"
#include <QImage>
#include <QVariant>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPixmap>
#include <src/sprite.h>



Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parent())->parentSprite();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
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
    case 1: //depth
        res.setValue(QString("%1bpp").arg(m_depth));
        break;
    case 2: //resolution
        res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
    };
    return std::move(res);
}



Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *MFrame::parentSprite()
{
    return static_cast<FramesContainer*>(parent())->parentSprite();
}

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect * out_area) const
{
    Sprite * pspr = const_cast<MFrame*>(this)->parentSprite();
    //QRect dim;

    //#TODO: Implement checks for the other paramters for a frame, and for mosaic and etc!
    QImage      imgres(512,512, QImage::Format_RGB32);
    QPainter    painter(&imgres);
    QRect       bounds = calcFrameBounds();

    //Set first pal color as bg color!
    painter.setBackground( QColor(pspr->getPalette().front()) );
    painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

    //Make first color transparent
    QVector<QRgb> pal = pspr->getPalette();
    QColor firstcol(pal.front());
    firstcol.setAlpha(0);
    pal.front() = firstcol.rgba();

    //Draw all the parts of the frame
    const fmt::step_t * plast = nullptr; //A reference on the last valid frame, so we can properly copy it when encountering a -1 frame!
    for( const fmt::step_t & part : m_parts )
    {
        auto res = part.GetResolution();
        Image* pimg = pspr->getImage(part.getFrameIndex()); // returns null if -1 frame or out of range!
        QImage pix;
        if(!pimg && plast) //check for -1 frames
        {
            Image* plastimg = pspr->getImage(plast->getFrameIndex());
            pix = qMove(plastimg->makeImage(pal));
        }
        else if(pimg)
        {
            pix = qMove(pimg->makeImage(pal));
            plast = &part;
        }

        //pix.setMask(pix.createMaskFromColor( QColor(pspr->getPalette().front()), Qt::MaskMode::MaskInColor ));

        if(part.isHFlip())
            pix = qMove( pix.transformed( QTransform().scale(-1, 1) ) );
        if(part.isVFlip())
            pix = qMove( pix.transformed( QTransform().scale(1, -1) ) );

        int finalx = (part.getXOffset());
        int finaly = (part.getYOffset());
        painter.drawImage(xoffset + finalx, yoffset + finaly, pix );
    }

    //imgres.save("./mframeassemble.png", "png");

//    dim.setX(xoffset - bounds.x());
//    dim.setY(yoffset - bounds.y());
//    dim.setWidth(bounds.width());
//    dim.setHeight(bounds.height());

    if(out_area)
        *out_area = bounds;

    return imgres.copy( xoffset + bounds.x(),
                        yoffset + bounds.y(),
                        bounds.width(),
                        bounds.height() );
}

QRect MFrame::calcFrameBounds() const
{
    int smallestx = 512;
    int biggestx = 0;
    int smallesty  = 256;
    int biggesty  = 0;

    for( const fmt::step_t & part : m_parts )
    {
        auto imgres = part.GetResolution();
        int xoff = part.getXOffset();
        int yoff = part.getYOffset();

        if( xoff < smallestx)
            smallestx = xoff;
        if( (xoff + imgres.first) >= biggestx )
            biggestx = (xoff + imgres.first);

        if( yoff < smallesty)
            smallesty = yoff;
        if( (yoff + imgres.second) >= biggesty )
            biggesty = (yoff + imgres.second);
    }

    return QRect( smallestx, smallesty, (biggestx - smallestx), (biggesty - smallesty) );
}


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
    TreeElement *childItem  = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ImagesManager::parent(const QModelIndex &child) const
{
    TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
    TreeElement *parentItem = childItem->parent();
    Q_ASSERT(parentItem != nullptr);

    if (parentItem == m_parentcnt)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int ImagesManager::rowCount(const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    return parentItem->childCount();
}

int ImagesManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
    else
        return 3;
}

bool ImagesManager::hasChildren(const QModelIndex &parent) const
{
    TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    if(parentItem)
        return parentItem->childCount() > 0;
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
            return std::move(QVariant( QString("Preview") ));
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
    success = parentItem->insertChildren(row, count);
    endInsertRows();

    return success;
}

bool ImagesManager::removeRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildren(row, count);
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
