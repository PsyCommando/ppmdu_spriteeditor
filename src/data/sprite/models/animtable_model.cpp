#include "animtable_model.hpp"
#include <src/data/sprite/animtable.hpp>
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/sprite.hpp>

const QString ANIMTABLE_NO_SLOTS = "--";
const QString ANIMTABLE_NO_GRP = "None";

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

//void AnimTableModel::setSlotNames(QVector<QString> &&names)
//{
//    m_slotNames = names;
//}

//QVector<QString> &AnimTableModel::getSlotNames()
//{
//    return m_slotNames;
//}

//const QVector<QString> &AnimTableModel::getSlotNames() const
//{
//    return const_cast<AnimTableModel*>(this)->getSlotNames();
//}

int AnimTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return static_cast<int>(AnimTableSlot::eColumns::NbColumns);
}

QVariant AnimTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole)
        return parent_t::data(index, role);

    const AnimTableSlot * grpref = static_cast<const AnimTableSlot*>(getItem(index)); //The Reference to the group
    Q_ASSERT(grpref);
    const AnimGroup * pgrp = m_sprite->getAnimGroup(grpref->getGroupRef()); //The group being referenced

    switch(static_cast<AnimTableSlot::eColumns>(index.column()))
    {
    case AnimTableSlot::eColumns::GroupId:
        {
            if(role == Qt::DecorationRole)
            {
                if(pgrp)
                    return pgrp->MakeGroupPreview(getOwnerSprite(), 256, 32, 8);
            }
            else if(role == Qt::DisplayRole)
            {
                if(!grpref->isNull())
                    return QString("ID:%1").arg(grpref->getGroupRef());
                else
                    return ANIMTABLE_NO_GRP;
            }
            else if(role == Qt::EditRole)
                return grpref->getGroupRef();
            else if(role == Qt::SizeHintRole)
            {
                return QSize(288, 32);
            }
            break;
        }
    case AnimTableSlot::eColumns::GroupName:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                return grpref->getSlotName();
            }
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI",9));
                return QSize(fm.horizontalAdvance(data(index, Qt::DisplayRole).toString())+4, fm.height()+4);
            }
            break;
        }
    case AnimTableSlot::eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole)
            {
                if(pgrp)
                    return pgrp->nodeChildCount();
                else
                    return ANIMTABLE_NO_SLOTS;
            }
            else if(role == Qt::SizeHintRole)
                return QSize(64, 32);
            break;
        }
    default:
        break;
    };
    return parent_t::data(index, role);
}

bool AnimTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || role != Qt::EditRole)
        return false;

    AnimTableSlot * grpref = static_cast<AnimTableSlot*>(getItem(index)); //The Reference to the group
    Q_ASSERT(grpref);

    const AnimTableSlot::eColumns columnid = static_cast<AnimTableSlot::eColumns>(index.column());
    if(columnid == AnimTableSlot::eColumns::GroupId)
    {
        bool bok = false;
        fmt::animgrpid_t newref = value.toInt(&bok);
        if(bok)
        {
            grpref->setGroupRef(newref);
            emitDataChanged(index, index, {role});
        }
        return bok;
    }
    else if(columnid == AnimTableSlot::eColumns::GroupName)
    {
        grpref->setSlotName(value.toString());
        emitDataChanged(index, index, {role});
        return true;
    }
    return false;
}

QVariant AnimTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::SizeHintRole)
        return parent_t::headerData(section, orientation, role);

    if( orientation == Qt::Orientation::Vertical )
    {
        if(role == Qt::SizeHintRole)
            return QSize(32,32);
        return QVariant(QString("%1").arg(section));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             (section >= 0) && (section < AnimTableSlot::ColumnNames.size()) )
    {
        AnimTableSlot::eColumns column = static_cast<AnimTableSlot::eColumns>(section);
        if(role == Qt::SizeHintRole)
        {
            switch(column)
            {
            case AnimTableSlot::eColumns::GroupId:
                return QSize(320, 32);
            case AnimTableSlot::eColumns::GroupName:
                return QSize(256, 32);
            case AnimTableSlot::eColumns::NbSlots:
                return QSize(32, 32);
            default:
                break;
            }
        }
        else if(role == Qt::DisplayRole)
            return AnimTableSlot::ColumnNames.value(column);
    }
    return parent_t::headerData(section, orientation, role);
}

TreeNodeModel::node_t *AnimTableModel::getRootNode()
{
    return static_cast<TreeNode*>(m_root);
}

Sprite *AnimTableModel::getOwnerSprite()
{
    return m_sprite;
}
