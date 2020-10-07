#ifndef IMAGES_LIST_MODEL_HPP
#define IMAGES_LIST_MODEL_HPP
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  ImagesManager
//*******************************************************************
//Model for displaying the image list on the image list tab!
class ImageContainer;
class ImageListModel : public TreeNodeModel
{
    Q_OBJECT;
public:
    explicit ImageListModel(ImageContainer * imgcnt, Sprite* psprite);
    ~ImageListModel();

    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;
    const node_t * getRootNode()const override      {return const_cast<ImageListModel*>(this)->getRootNode();}
    const Sprite* getOwnerSprite()const override    {return const_cast<ImageListModel*>(this)->getOwnerSprite();}

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//    bool insertRows(int row, int count, const QModelIndex &parent) override;
//    bool removeRows(int row, int count, const QModelIndex &parent) override;
//    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
//    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    ImageContainer * m_root  {nullptr};
    Sprite *         m_sprite{nullptr};
};

//*******************************************************************
//  ImageListModelCondensed
//*******************************************************************
//Specialized model for displaying the images with a condensed description
class ImageListModelCondensed : public ImageListModel
{
    Q_OBJECT
    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // IMAGES_LIST_MODEL_HPP
