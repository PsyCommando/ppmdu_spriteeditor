#include "palette_model.hpp"
#include <QImage>
#include <src/data/sprite/palettecontainer.hpp>

//============================================================================================
//  PaletteModel
//============================================================================================
PaletteModel::PaletteModel(PaletteContainer * pal, QObject * parent)
    :QAbstractItemModel(parent), m_root(pal)
{

}

PaletteModel::~PaletteModel()
{

}

QModelIndex PaletteModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        //            if(row < 0)
        //                return createIndex(row, 0, nullptr ); //Always col 0
        //            else
        return createIndex(row, 0, &m_root->getPalette()[row] ); //Always col 0
    }
    else //Access color component 0 to 3, pointer is set to color itself
    {
        return createIndex(0, column, &m_root->getPalette()[row] );
    }
}

QModelIndex PaletteModel::parent(const QModelIndex &child) const
{
    if(!child.isValid())
        return QModelIndex();

    /*        if(!child.parent().isValid())
            return QModelIndex(); //If invalid child is a color, so the palette is its parent
        else*/ //If valid, child is a component of a color, so that color is its parent
    if( child.column() == 0 )
    {
        //            QRgb * colr = &(m_pal->at(child.parent().row()));//getColor2D(child.parent().row(), child.parent().column());
        //            Q_ASSERT(colr);
        return  child;
    }
    else
        return child;
}

int PaletteModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 1; //color entries have only one "row"
    return m_root->getPalette().size()/* / PaletteDisplayNBColorsPerRow*/;
}

int PaletteModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 4; //color entries have 4 components ARGB
    return 1; //one entry for colors
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) //root is never valid!
        return QVariant();

    if(!index.parent().isValid()) //this means we're a color!
    {
        //If the parent is the root, aka the palette we print the color/return the color value
        if(role == Qt::DisplayRole)
        {
            QImage coloricon(8,8, QImage::Format::Format_ARGB32_Premultiplied);
            coloricon.fill( *static_cast<QRgb*>(index.internalPointer()));
            return coloricon;
        }
        else if(role == Qt::EditRole)
        {
            QVariant var;
            var.setValue<QRgb>( *static_cast<QRgb*>(index.internalPointer()) );
            return var;
        }
    }
    else if( (role == Qt::EditRole || role == Qt::DisplayRole) &&
             (index.column() < columnCount(index)) ) //Make sure we're accessing one of the 4 components
    {
        //If the parent is a specific color, we return one component for each column!
        uint32_t component = *static_cast<QRgb*>(index.internalPointer()) >> (24 - (index.column() * 8));
        return (component & 0xFF);
    }
    return QVariant();
}

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid()) //root is never valid!
        return false;

    Q_ASSERT(index.internalPointer());
    if(!index.parent().isValid()) //this means we're a color!
    {
        //If the parent is the root, aka the palette we print the color/return the color value
        if(role == Qt::EditRole)
        {
            (m_root->getPalette()[index.row()]) = value.toUInt();
            return true;
        }
    }
    else if( role == Qt::EditRole &&
             (index.column() < columnCount(index)) ) //Make sure we're accessing one of the 4 components
    {
        //If the parent is a specific color, we return one component for each column!
        QRgb * col = static_cast<QRgb *>(index.internalPointer());
        Q_ASSERT(col);
        uint32_t shiftlen     = (24 - (index.column() * 8));
        uint32_t maskedTarget = (~(0xFF << shiftlen)) & *col; //Keep all bits but the 8 we're editing!
        (*col) = maskedTarget | (value.toUInt() << shiftlen);
        return true;
    }
    return false;
}

QVariant PaletteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
        return QString("%1").arg(section);
    else if( orientation == Qt::Orientation::Horizontal )
    {
        switch(section)
        {
        case 0: return QString("Red");
        case 1: return QString("Green");
        case 2: return QString("Blue");
        case 3: return QString("Alpha");
        };
    }
    return QVariant();
}

bool PaletteModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid())
        return false;

    beginInsertRows(parent, row, row + (count -1) );
    m_root->getPalette().insert(row, count, QRgb());
    endInsertRows();
    return true;
}

bool PaletteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid())
        return false;

    beginRemoveRows(parent, row, (row + (count - 1)));
    m_root->getPalette().remove(row, count);
    endRemoveRows();
    return true;
}

bool PaletteModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    //We only handle moves of colors, in the same palette
    if( (destinationParent != sourceParent) ||
        (sourceParent.isValid() && destinationParent.isValid()) )
        return false;

    beginMoveRows(sourceParent,
                  sourceRow,
                  sourceRow + (count-1),
                  destinationParent,
                  destinationChild);
    for(int cntr = sourceRow; cntr < sourceRow + count; ++cntr)
        m_root->getPalette().move(sourceRow, (destinationChild - 1) );
    endMoveRows();
    return true;
}

Qt::ItemFlags PaletteModel::flags(const QModelIndex &) const
{
    return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable;
}