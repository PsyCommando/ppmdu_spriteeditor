#include "sprite_overview_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <functional>

//=================================================================================================================
//  SpriteOverviewModel
//=================================================================================================================
const QVector<SpriteOverviewModel::stats_t> SpriteOverviewModel::StatEntries
{
    {"Nb Images",       [](Sprite* spr){return QString("%L1").arg(spr->getImages().nodeChildCount());} },
    {"Nb Frames",       [](Sprite* spr){return QString("%L1").arg(spr->getFrames().nodeChildCount());} },
    {"Nb Sequences",    [](Sprite* spr){return QString("%L1").arg(spr->getAnimSequences().nodeChildCount());} },
    {"Nb Animations",   [](Sprite* spr){return QString("%L1").arg(spr->getAnimTable().nodeChildCount());} },
    {"Palette",         [](Sprite* spr)
        {
            if(!spr->getPalette().empty())
                return QString("%L1 colors").arg( spr->getPalette().size() );
            else
                return QString("none");
        } },
    {"Blocks Used",     [](Sprite* spr){return QString("%L1 blocks").arg(spr->getMaxBlocksUsage());}},
    {"Size",            [](Sprite* spr){return QString("%L1 bytes").arg(spr->getRawData().size());} },
};

SpriteOverviewModel::SpriteOverviewModel(Sprite *spr)
    :QAbstractItemModel(nullptr), m_spr(spr)
{}

QModelIndex SpriteOverviewModel::index(int row, int, const QModelIndex &/*parent*/) const
{
    return createIndex(row, 0);
}

QModelIndex SpriteOverviewModel::parent(const QModelIndex &/*child*/) const
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

int SpriteOverviewModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant SpriteOverviewModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if(index.row() >= 0 && index.row() < StatEntries.size())
        return StatEntries.at(index.row()).second(m_spr);

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
