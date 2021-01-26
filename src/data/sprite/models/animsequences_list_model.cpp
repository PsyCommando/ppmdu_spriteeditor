#include "animsequences_list_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/animsequence.hpp>
#include <src/data/sprite/animgroup.hpp>

const std::map<AnimSequencesListModel::eColumns, QString> AnimSequencesListModel::ColumnNames
{
    {AnimSequencesListModel::eColumns::Preview,  "Preview"},
    {AnimSequencesListModel::eColumns::NbFrames, "Nb frames"},
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

int AnimSequencesListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnNames.size();
}

QVariant AnimSequencesListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
        return QVariant(QString("%1").arg(section));
    else if( orientation == Qt::Orientation::Horizontal)
    {
        eColumns col = static_cast<eColumns>(section);
        auto itcol = ColumnNames.find(col);
        if(itcol != ColumnNames.end())
            return itcol->second;
    }
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

    switch(static_cast<eColumns>(index.column()))
    {
    case eColumns::Preview:
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
    case eColumns::NbFrames:
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
