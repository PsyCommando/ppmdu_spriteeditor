#ifndef IMAGESMANAGERMODEL_HPP
#define IMAGESMANAGERMODEL_HPP
#include <src/data/treeelem.hpp>

//*******************************************************************
//  ImagesManager
//*******************************************************************
//Model for displaying the image list on the image list tab!
class ImageContainer;
class ImagesManager : public QAbstractItemModel
{
    Q_OBJECT

    // QAbstractItemModel interface
public:
    ImagesManager(ImageContainer * parent);
    virtual ~ImagesManager();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    TreeElement *getItem(const QModelIndex &index);
    const TreeElement *getItem(const QModelIndex &index)const
    {
        return const_cast<ImagesManager*>(this)->getItem(index);
    }

protected:
    ImageContainer * m_parentcnt{nullptr};
};

#endif // IMAGESMANAGERMODEL_HPP
