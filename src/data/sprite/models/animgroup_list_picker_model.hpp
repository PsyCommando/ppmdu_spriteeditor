#ifndef ANIMGROUPPICKERMODEL_HPP
#define ANIMGROUPPICKERMODEL_HPP
#include <QAbstractItemModel>
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  AnimGroupPickerModel
//*******************************************************************
//Model for the anim group picker list used in the anim table tab
class AnimGroup;
class AnimGroupListPickerModel : public TreeNodeModel
{
    Q_OBJECT
public:
    explicit AnimGroupListPickerModel(Sprite * owner);
    ~AnimGroupListPickerModel();

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    static const int MAX_NB_PREVIEW_IMGS;
    static const int WIDTH_PREVIEW_IMGS;
    static const int HEIGHT_PREVIEW_IMGS;
    Sprite * m_sprite{nullptr};
};

#endif // ANIMGROUPPICKERMODEL_HPP
