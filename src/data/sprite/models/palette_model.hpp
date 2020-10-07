#ifndef PALETTEMODEL_HPP
#define PALETTEMODEL_HPP
#include <QAbstractItemModel>

//====================================================================
//  PaletteModel
//====================================================================
//Interface for accessing palettes from the palette container
class PaletteContainer;
class PaletteModel : public QAbstractItemModel
{
    Q_OBJECT
 public:
    explicit PaletteModel(PaletteContainer * pal, QObject * parent = nullptr);
    ~PaletteModel();

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool     setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &sourceParent,
                  int sourceRow,
                  int count,
                  const QModelIndex &destinationParent,
                  int destinationChild) override;

    Qt::ItemFlags flags(const QModelIndex &/*index*/) const override;

private:
    PaletteContainer * m_root {nullptr};
};
#endif // PALETTEMODEL_HPP
