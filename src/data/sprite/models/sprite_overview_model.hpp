#ifndef SPRITEOVERVIEWMODEL_HPP
#define SPRITEOVERVIEWMODEL_HPP
#include <QAbstractItemModel>

class Sprite;
//*******************************************************************
//  SpriteOverviewModel
//*******************************************************************
//Model for displaying general statistics on the sprite in the property tab!
class SpriteOverviewModel : public QAbstractItemModel
{
    Q_OBJECT
    typedef QPair<QString,std::function<QVariant(Sprite*)>> stats_t;
    static const QVector<stats_t>                           StatEntries;
    Sprite                                                  *m_spr{nullptr};
public:
    SpriteOverviewModel(Sprite * spr);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int /*column*/, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // SPRITEOVERVIEWMODEL_HPP
