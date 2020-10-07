#include "animsequences_list_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/animsequence.hpp>
#include <src/data/sprite/animgroup.hpp>

const QStringList ANIMATION_SEQUENCE_HEADER_COLUMNS
{
    "Preview",
    "Nb frames",
};

//*******************************************************************
//  AnimSequencesModel
//*******************************************************************
AnimSequencesListModel::AnimSequencesListModel(AnimSequences * pseqs, Sprite * owner)
    :TreeNodeModel()
{
    m_root = pseqs;
    m_sprite = owner;
}
TreeNodeModel::node_t *AnimSequencesListModel::getRootNode()
{
    return m_root;
}

Sprite *AnimSequencesListModel::getOwnerSprite()
{
    return m_sprite;
}

int AnimSequencesListModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return ANIMATION_SEQUENCE_HEADER_COLUMNS.size();
}

QVariant AnimSequencesListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
        return QVariant(QString("%1").arg(section));
    else if( orientation == Qt::Orientation::Horizontal && section < ANIMATION_SEQUENCE_HEADER_COLUMNS.size() )
        return ANIMATION_SEQUENCE_HEADER_COLUMNS.at(section);
    return QVariant();
}

QVariant AnimSequencesListModel::data(const QModelIndex &index, int role)const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    switch(static_cast<eAnimationSequenceColumns>(index.column()))
    {
    case eAnimationSequenceColumns::Preview:
        {
            if(role == Qt::DecorationRole)
            {
                const AnimSequence * pseq = m_root->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QVariant(pseq->makePreview(getOwnerSprite()));
            }
            else if(role == Qt::DisplayRole)
            {
                const AnimSequence * pseq = m_root->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QString("Sequence#%1 - %2 frames").arg(index.row()).arg(pseq->nodeChildCount());
            }
            break;
        }
    case eAnimationSequenceColumns::NbFrames:
        {
            if(role == Qt::DisplayRole)
            {
                const AnimSequence * pseq = getOwnerSprite()->getAnimSequence(index.row());
                Q_ASSERT(pseq);
                return QString("%1 frames").arg(pseq->nodeChildCount());
            }
            break;
        }
    default: break;
    };
    return QVariant();
}


//QVariant AnimSequences::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant("root");

//    if (role != Qt::DisplayRole &&
//        role != Qt::DecorationRole &&
//        role != Qt::SizeHintRole &&
//        role != Qt::EditRole)
//        return QVariant();

//    return static_cast<TreeNode*>(index.internalPointer())->nodeData(index.column(), role);
//}

//QVariant AnimSequences::nodeData(int column, int role) const
//{
//    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(ElemName());
//    return QVariant();
//}

//QVariant AnimSequences::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//        return qMove(QVariant( QString("%1").arg(section) ));
//    else if( orientation == Qt::Orientation::Horizontal && section < HEADER_COLUMNS.size() )
//        return HEADER_COLUMNS.at(section);
//    return QVariant();
//}

//{
//    if (!index.isValid())
//        return QVariant("root");

//    //    if (role != Qt::DisplayRole &&
//    //        role != Qt::DecorationRole &&
//    //        role != Qt::SizeHintRole &&
////        role != Qt::EditRole)
////        return QVariant();

////    const AnimGroup *grp = static_cast<const AnimGroup*>(getItem(index));
////    return grp->nodeData(index.column(), role);

//    if( role != Qt::DisplayRole  &&
//        role != Qt::EditRole     &&
//        role != Qt::SizeHintRole &&
//        role != Qt::DecorationRole)
//        return QVariant();

//    const AnimGroup * grp = static_cast<const AnimGroup*>(getItem(index));
//    Q_ASSERT(grp);

////    if( Qt::SizeHintRole )
////    {
////        QFontMetrics fm(QFont("Sergoe UI", 9));
////        QString str = data(index, Qt::DisplayRole).toString();
////        return QSize(fm.width(str), fm.height());
////    }

//    switch(static_cast<AnimGroup::eColumns>(index.column()))
//    {
//    case AnimGroup::eColumns::GroupID:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return grp->getGroupUID();
//            break;
//        }
//    case AnimGroup::eColumns::GroupName:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//            {
//                auto itf = m_slotNames.find(index.row());
//                if(itf != m_slotNames.end())
//                    return (*itf);
//                return QString("--");
//            }
//            break;
//        }
//    case AnimGroup::eColumns::NbSlots:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return grp->seqSlots().size();
//            break;
//        }
//    default:
//        {
//            break;
//        }
//    };
//    return QVariant();
//}

//QVariant AnimSequencesModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//    {
//        return QVariant(QString("%1").arg(section));
//    }
//    else if( orientation == Qt::Orientation::Horizontal &&
//             (section >= 0) && (section < AnimGroup::ColumnNames.size()) )
//    {
//        return AnimGroup::ColumnNames[section];
//    }
//    return QVariant();
//}








