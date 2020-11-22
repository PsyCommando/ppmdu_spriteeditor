#include "animsequences_list_picker_model.hpp"
#include <src/data/sprite/animsequences.hpp>

QSize CalcTextSize(const QString & text )
{
    static const QFontMetrics fm(QFont("Sergoe UI",9));
    return QSize(fm.horizontalAdvance(text), fm.height());
}

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

int AnimSequencesListPickerModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ANIMATION_SEQUENCE_HEADER_COLUMNS.size();
}

QVariant AnimSequencesListPickerModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole )
        return QVariant();

    AnimSequence * pseq = m_root->getSequenceByID(index.row());
    Q_ASSERT(pseq);

    switch(static_cast<eAnimationSequenceColumns>(index.column()))
    {
    case eAnimationSequenceColumns::Preview:
        {
            if(role == Qt::DecorationRole)
                return QVariant(pseq->makePreview(getOwnerSprite()));
            else if(role == Qt::DisplayRole)
                return QString("Sequence#%1").arg(index.row());
            else if(role == Qt::SizeHintRole)
                return CalcTextSize(data(index, Qt::DisplayRole).toString());
            break;
        }
    case eAnimationSequenceColumns::NbFrames:
        {
            if(role == Qt::DisplayRole)
                return QString("%1 frames").arg(pseq->nodeChildCount());
            else if(role == Qt::SizeHintRole)
                return CalcTextSize(data(index, Qt::DisplayRole).toString());
            break;
        }
    default:
        break;
    };
    return QVariant();
}

QVariant AnimSequencesListPickerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString("%1").arg(section);
    }
    else if(orientation == Qt::Horizontal && role == Qt::DisplayRole && section < ANIMATION_SEQUENCE_HEADER_COLUMNS.size())
    {
        return {ANIMATION_SEQUENCE_HEADER_COLUMNS[section]};
    }
    return TreeNodeModel::headerData(section, orientation, role);
}

Qt::ItemFlags AnimSequencesListPickerModel::flags(const QModelIndex &index) const
{
    return TreeNodeModel::flags(index) | Qt::ItemIsDragEnabled; //Items we own are draggable
}
