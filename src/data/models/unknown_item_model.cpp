#include "unknown_item_model.hpp"
#include <src/data/sprite/unknown_item.hpp>
#include <string>

const QVector<UnknownItemModel::stats_t> UnknownItemModel::DetailsEntries
{
    {"Header",  [](UnknownItemNode* item){
            if(item->raw().size() < 4)
                return QString();
            std::string txt(item->raw().begin(), std::next(item->raw().begin(), 4));
            bool strprintable = true;
            for(char c : txt)
            {
                if(!std::isprint(c))
                {
                    strprintable = false;
                    break;
                }
            }
            QString result;
            if(strprintable)
            {
                result = QString("%1%2%3%4")
                        .arg(static_cast<char>(item->raw()[0]))
                        .arg(static_cast<char>(item->raw()[1]))
                        .arg(static_cast<char>(item->raw()[2]))
                        .arg(static_cast<char>(item->raw()[3]));
            }
            else
            {
                result = QString("0x%1 0x%2 0x%3 0x%4")
                        .arg(item->raw()[0], 2, 16, QLatin1Char('0'))
                        .arg(item->raw()[1], 2, 16, QLatin1Char('0'))
                        .arg(item->raw()[2], 2, 16, QLatin1Char('0'))
                        .arg(item->raw()[3], 2, 16, QLatin1Char('0'));
            }
            return result;
        }
    },
    {"Size",    [](UnknownItemNode* item){return QString("%L1 bytes").arg(item->raw().size());} },
};

UnknownItemModel::UnknownItemModel(UnknownItemNode* item)
    :QAbstractItemModel(nullptr), m_item(item)
{}

UnknownItemModel::~UnknownItemModel()
{}

QModelIndex UnknownItemModel::index(int row, int /*column*/, const QModelIndex &/*parent*/) const
{
    return createIndex(row, 0);
}

QModelIndex UnknownItemModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

int UnknownItemModel::rowCount(const QModelIndex &parent) const
{
    if( !parent.isValid() )
        return DetailsEntries.size();
    else
        return 0;
}

int UnknownItemModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant UnknownItemModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if(index.row() >= 0 && index.row() < DetailsEntries.size())
        return DetailsEntries.at(index.row()).second(m_item);

    return QVariant();
}

QVariant UnknownItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole || orientation != Qt::Orientation::Vertical )
        return QVariant();

    if(section >= 0 && section < DetailsEntries.size())
        return DetailsEntries.at(section).first;

    return QVariant();
}
