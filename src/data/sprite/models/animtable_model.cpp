#include "animtable_model.hpp"
#include <src/data/sprite/animtable.hpp>
#include <src/data/sprite/animgroup.hpp>


AnimTableModel::AnimTableModel(AnimTable *ptable, Sprite *powner)
    :TreeNodeModel(nullptr)
{
    m_root = ptable;
    m_sprite = powner;
}

AnimTableModel::~AnimTableModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

void AnimTableModel::setSlotNames(QVector<QString> &&names)
{
    m_slotNames = names;
}

QVector<QString> &AnimTableModel::getSlotNames()
{
    return m_slotNames;
}

const QVector<QString> &AnimTableModel::getSlotNames() const
{
    return const_cast<AnimTableModel*>(this)->getSlotNames();
}

int AnimTableModel::columnCount(const QModelIndex &parent) const
{
    return static_cast<int>(AnimGroup::eColumns::NbColumns);
}

QVariant AnimTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    //    if (role != Qt::DisplayRole &&
    //        role != Qt::DecorationRole &&
    //        role != Qt::SizeHintRole &&
//        role != Qt::EditRole)
//        return QVariant();

//    const AnimGroup *grp = static_cast<const AnimGroup*>(getItem(index));
//    return grp->nodeData(index.column(), role);

    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole)
        return QVariant();

    const AnimGroup * grp = static_cast<const AnimGroup*>(getItem(index));
    Q_ASSERT(grp);

    switch(static_cast<AnimGroup::eColumns>(index.column()))
    {
    case AnimGroup::eColumns::GroupID:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return grp->getGroupUID();
            break;
        }
    case AnimGroup::eColumns::GroupName:
        {
            if(role == Qt::DisplayRole)
            {
                if(index.row() < m_slotNames.size())
                    return m_slotNames[index.row()];
                return QString();
            }
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI",9));
                if(index.row() < m_slotNames.size())
                    return QSize(fm.horizontalAdvance(m_slotNames[index.row()])+4, fm.height()+4);
            }
            break;
        }
    case AnimGroup::eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return grp->nodeChildCount();
            else if(role == Qt::SizeHintRole)
                return QSize(64, 32);
            break;
        }
    case AnimGroup::eColumns::Preview:
        {
            if(role == Qt::DecorationRole)
            {
                QPixmap img = grp->MakeGroupPreview(getOwnerSprite(), 256, 32, 8);
                return img;
            }
            else if(role == Qt::SizeHintRole)
            {
                return QSize(256, 32);
            }
            break;
        }
    default:
        {
            break;
        }
    };
    return QVariant();
}

bool AnimTableModel::setData(const QModelIndex&, const QVariant&, int)
{
    return false;
}

QVariant AnimTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::SizeHintRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        if(role == Qt::SizeHintRole)
            return QSize(32,32);
        return QVariant(QString("%1").arg(section));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             (section >= 0) && (section < AnimGroup::ColumnNames.size()) )
    {
        if(role == Qt::SizeHintRole)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.horizontalAdvance(AnimGroup::ColumnNames[section])+16, fm.height()+16);
        }
        return AnimGroup::ColumnNames[section];
    }
    return QVariant();
}

TreeNodeModel::node_t *AnimTableModel::getRootNode()
{
    return static_cast<TreeNode*>(m_root);
}

Sprite *AnimTableModel::getOwnerSprite()
{
    return m_sprite;
}
