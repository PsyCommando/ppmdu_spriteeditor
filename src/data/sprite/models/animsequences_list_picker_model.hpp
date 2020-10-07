#ifndef ANIMSEQUENCESLISTPICKERMODEL_HPP
#define ANIMSEQUENCESLISTPICKERMODEL_HPP
#include <src/data/sprite/models/animsequences_list_model.hpp>

//*******************************************************************
//  AnimSequencesPickerModel
//*******************************************************************
//Model meant to display all animation sequences so they can be dragged to an animation slot in the animation table.
class AnimSequencesListPickerModel : public AnimSequencesListModel
{
    Q_OBJECT
public:
    explicit AnimSequencesListPickerModel(AnimSequences * pseqs, Sprite * owner);
    ~AnimSequencesListPickerModel();

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
};

#endif // ANIMSEQUENCESLISTPICKERMODEL_HPP
