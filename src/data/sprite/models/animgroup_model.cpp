#include "animgroup_model.hpp"
#include <src/data/sprite/animsequence.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/animsequences_list_model.hpp>
#include <src/data/sprite/animgroup.hpp>

const QStringList AnimGroupModel::ColumnNames
{
    {"Sequence ID"},
    {"Slot Name"},
    {"Nb Frames"},
};

//******************************************************************************
//  AnimGroupModel
//******************************************************************************
AnimGroupModel::AnimGroupModel(AnimGroup *pgrp, Sprite *owner)
    :TreeNodeModel(nullptr)
{
    m_root = pgrp;
    m_sprite = owner;
}

AnimGroupModel::~AnimGroupModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

QVariant AnimGroupModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole )
        return QVariant();

    if(index.row() > m_root->nodeChildCount())
    {
        Q_ASSERT(false);
        return QVariant();
    }

    const AnimSequenceReference * ref = static_cast<const AnimSequenceReference *>(m_root->nodeChild(index.row()));
    Q_ASSERT(ref);
    fmt::animseqid_t    id = ref->getSeqRefID();
    const AnimSequence *        pseq = getOwnerSprite()->getAnimSequence(id);

    switch(static_cast<eColumns>(index.column()))
    {
    case eColumns::Preview:
        {
            if(role == Qt::DisplayRole)
            {
                if(id == -1)
                    return QString("INVALID");
                if(!pseq)
                    return QString("MissingID:%1").arg(id);
                return QString("ID:%1").arg(id);
            }
            else if(role == Qt::DecorationRole)
            {
                const AnimSequence * pseq = getOwnerSprite()->getAnimSequence(id);
                if(!pseq || id == -1)
                    return QVariant();
                return QVariant(pseq->makePreview(getOwnerSprite()));
            }
            else if(role == Qt::EditRole)
                return id;
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI", 9));
                QString str = data(index, Qt::DisplayRole).toString();
                QSize   szimg = data(index, Qt::DecorationRole).value<QImage>().size();
                QSize   sztxt(fm.horizontalAdvance(str), fm.height());

                QSize outsz = sztxt;
                if( szimg.height() > sztxt.height() )
                    outsz.setHeight(szimg.height());
                outsz.setWidth( szimg.width() + sztxt.width());
                return outsz;
            }
            break;
        }
    case eColumns::SlotName:
        {
            if(role == Qt::DisplayRole)
                return getSlotName(index.row());
            break;
        }
    case eColumns::NbFrames:
        {
            if(role == Qt::DisplayRole)
                return QString("%2 frames").arg(pseq->nodeChildCount());
            break;
        }
    }

    return QVariant();
}

QVariant AnimGroupModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(orientation == Qt::Orientation::Vertical)
        {
            return section;
        }
        else if(orientation == Qt::Orientation::Horizontal &&
                section >= 0 && section < ColumnNames.size())
        {
            return ColumnNames[section];
        }
    }
    return TreeNodeModel::headerData(section, orientation, role);
}

bool AnimGroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return false;

    if(index.row() > m_root->nodeChildCount())
    {
        Q_ASSERT(false);
        return false;
    }

    bool bok = false;
    if(index.column() == 0)
    {
        AnimSequenceReference * ref = static_cast<AnimSequenceReference *>(m_root->nodeChild(index.row()));
         ref->setSeqRefID(value.toInt(&bok));
    }

    if(bok)
        emit dataChanged(index, index, QVector<int>{role});
    return bok;
}

Qt::ItemFlags AnimGroupModel::flags(const QModelIndex &index) const
{
    return TreeNodeModel::flags(index) | Qt::ItemIsDropEnabled;
}

QMap<int, QVariant> AnimGroupModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> res;
    if( index.isValid() )
    {
        res.insert( Qt::DisplayRole,    QVariant(data(index, Qt::DisplayRole)) );
        res.insert( Qt::SizeHintRole,   QVariant(data(index, Qt::SizeHintRole)) );
        res.insert( Qt::EditRole,       QVariant(data(index, Qt::EditRole)) );
        res.insert( Qt::DecorationRole, QVariant(data(index, Qt::DecorationRole)) );
    }
    return res;
}

bool AnimGroupModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    auto itf = roles.find(Qt::EditRole);
    if( !index.isValid() && itf == roles.end() )
        return false;
    return setData(index, *itf, Qt::EditRole);
}

Qt::DropActions AnimGroupModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions AnimGroupModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

bool AnimGroupModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return QAbstractItemModel::dropMimeData(data,action,row,column,parent);
}

TreeNodeModel::node_t *AnimGroupModel::getRootNode()
{
    return m_root;
}

Sprite *AnimGroupModel::getOwnerSprite()
{
    return m_sprite;
}

void AnimGroupModel::setSlotSequenceID(int slot, fmt::animseqid_t id)
{
    if(slot > m_root->nodeChildCount() || slot < 0)
        throw std::out_of_range("AnimGroupModel::setSlotSequenceID(): Slot out of range!");
    AnimSequenceReference * ref = static_cast<AnimSequenceReference *>(m_root->nodeChild(slot));
    ref->setSeqRefID(id);
}

fmt::animseqid_t AnimGroupModel::getSlotSequenceID(int slot) const
{
    if(slot > m_root->nodeChildCount() || slot < 0)
        throw std::out_of_range("AnimGroupModel::getSlotSequenceID(): Slot out of range!");
    const AnimSequenceReference * ref = static_cast<const AnimSequenceReference *>(m_root->nodeChild(slot));
    return ref->getSeqRefID();
}

int AnimGroupModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnNames.size();
}

QString AnimGroupModel::getSlotName(int index)const
{
    if(index > m_slotNames.size())
        return QString();
    return m_slotNames[index];
}

void AnimGroupModel::setSlotName(const QList<QString> & slotnamesref)
{
    m_slotNames = slotnamesref;
}
