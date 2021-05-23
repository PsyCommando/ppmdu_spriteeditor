#ifndef IMAGES_LIST_MODEL_HPP
#define IMAGES_LIST_MODEL_HPP
#include <src/data/treenodemodel.hpp>
#include <map>

//*******************************************************************
//  ImageListModel
//*******************************************************************
//Model for displaying the image list on the image list tab!
class ImageContainer;
class ImageListModel : public TreeNodeModel
{
    Q_OBJECT;
public:
    enum struct eColumns : int
    {
        Preview = 0,
        //UID,
        Depth,
        Resolution,
        Block,
        BlockLen,
        NbColumns,

        direct_Unk2,    //Extra columns that don't count in the NbColumns
        direct_Unk14,
    };
    static const std::map<eColumns, QString> ColumnNames;

public:
    explicit ImageListModel(ImageContainer * imgcnt, Sprite* psprite);
    ~ImageListModel();

    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;
    const node_t * getRootNode()const override      {return const_cast<ImageListModel*>(this)->getRootNode();}
    const Sprite* getOwnerSprite()const override    {return const_cast<ImageListModel*>(this)->getOwnerSprite();}

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

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
