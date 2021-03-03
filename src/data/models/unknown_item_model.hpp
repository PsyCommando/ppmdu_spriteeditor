#ifndef UNKNOWN_ITEM_MODEL_HPP
#define UNKNOWN_ITEM_MODEL_HPP
/*
 * Model for displaying properties for an unknown item on the unknown item tab
*/
#include <QAbstractItemModel>
#include <QPair>
#include <QString>

class UnknownItemNode;
class UnknownItemModel : public QAbstractItemModel
{
    typedef QPair<QString,std::function<QVariant(UnknownItemNode*)>> stats_t;
    static const QVector<stats_t>   DetailsEntries;
    UnknownItemNode*                m_item{nullptr};
    Q_OBJECT
public:
    UnknownItemModel(UnknownItemNode* item);
    virtual ~UnknownItemModel();

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // UNKNOWN_ITEM_MODEL_HPP
