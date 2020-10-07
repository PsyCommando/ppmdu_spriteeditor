#include "animsequences_list_picker_model.hpp"
#include <src/data/sprite/animsequences.hpp>

//*******************************************************************
//  AnimSequencesPickerModel
//*******************************************************************
//Model meant to display all animation sequences so they can be dragged to an animation slot in the animation table.
AnimSequencesListPickerModel::AnimSequencesListPickerModel(AnimSequences *pseqs, Sprite *owner)
    :AnimSequencesListModel(pseqs, owner)
{

}

AnimSequencesListPickerModel::~AnimSequencesListPickerModel()
{

}

int AnimSequencesListPickerModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return ANIMATION_SEQUENCE_HEADER_COLUMNS.size();
}

QVariant AnimSequencesListPickerModel::data(const QModelIndex &index, int role) const
{
    //Custom data display for the anim sequence picker!
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole )
        return QVariant();

    switch(static_cast<eAnimationSequenceColumns>(index.column()))
    {
    case eAnimationSequenceColumns::Preview:
        {
            if(role == Qt::DecorationRole)
            {
                AnimSequence * pseq = m_root->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QVariant(pseq->makePreview(getOwnerSprite()));
            }
            else if(role == Qt::DisplayRole)
            {
                AnimSequence * pseq = m_root->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QString("Sequence#%1 - %2 frames").arg(index.row()).arg(pseq->nodeChildCount());
            }
            break;
        }
    case eAnimationSequenceColumns::NbFrames:
//        {
//            if(role == Qt::DisplayRole)
//            {
//                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
//                Q_ASSERT(pseq);
//                return QString("%1 frames").arg(pseq->nodeChildCount());
//            }
//            break;
//        }
    default: break;
    };
    return QVariant();
}

QVariant AnimSequencesListPickerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString("%1").arg(section);
    }
    return QVariant();
}

Qt::ItemFlags AnimSequencesListPickerModel::flags(const QModelIndex &index) const
{
    return TreeNodeModel::flags(index) | Qt::ItemIsDragEnabled; //Items we own are draggable
}
