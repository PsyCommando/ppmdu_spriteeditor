#include "animgroup.hpp"
#include <src/sprite.hpp>
#include <src/data/animsequences.hpp>

//*******************************************************************
//  AnimGroupDelegate
//*******************************************************************
class AnimGroupDelegate : public QStyledItemDelegate
{
public:
    AnimGroupDelegate(AnimGroup * owner)
        :QStyledItemDelegate(nullptr), m_pOwner(owner)
    {
    }


    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
        if(pnode)
            return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
        return QStyledItemDelegate::sizeHint(option,index);
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        qWarning("unimplemented!");
        return nullptr;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        qWarning("unimplemented!");
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        editor->setGeometry(option.rect);
    }

private:
    AnimGroup * m_pOwner;
};

//**********************************************************************************
//  AnimGroup
//**********************************************************************************
const QStringList AnimGroup::ColumnNames
{
    "Group ID",
    "Group Name",
    "Nb Slots",
};


AnimGroup::AnimGroup(TreeElement *parent)
    :BaseTreeTerminalChild(parent),
      m_model(this)
{
    m_delegate.reset(new AnimGroupDelegate(this));
    setNodeDataTy(eTreeElemDataType::animGroup);
}

AnimGroup::AnimGroup(AnimGroup &&mv)
    :BaseTreeTerminalChild(qMove(mv)),
      m_model(this)
{
    m_delegate.reset(new AnimGroupDelegate(this));
    operator=(qMove(mv));
}

AnimGroup::AnimGroup(const AnimGroup &cp)
    :BaseTreeTerminalChild(cp),
      m_model(this)
{
    m_delegate.reset(new AnimGroupDelegate(this));
    operator=(cp);
}

AnimGroup & AnimGroup::operator=(AnimGroup &&mv)
{
    m_unk16     = mv.m_unk16;
    m_seqlist   = qMove(mv.m_seqlist);
    //We don't touch the delegate
    return *this;
}

AnimGroup & AnimGroup::operator=(const AnimGroup &cp)
{
    m_unk16     = cp.m_unk16;
    m_seqlist   = cp.m_seqlist;
    //We don't touch the delegate
    return *this;
}

AnimGroup::~AnimGroup()
{
    //Need non-default destructor for defining the delegate in the cpp
}

void AnimGroup::clone(const TreeElement *other)
{
    const AnimGroup * ptr = static_cast<const AnimGroup*>(other);
    if(!ptr)
        throw std::runtime_error("AnimGroup::clone(): other is not a AnimGroup!");
    (*this) = *ptr;
}

QVariant AnimGroup::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole)
        return QVariant();

    if(role == Qt::SizeHintRole)
    {
        QFontMetrics fm(QFont("Sergoe UI", 9));
        QString str(nodeData(column,Qt::DisplayRole).toString());
        return QSize(fm.horizontalAdvance(str), fm.height());
    }

    switch(static_cast<eColumns>(column))
    {
    case eColumns::GroupID:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return getGroupUID();
            break;
        }
    case eColumns::GroupName:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                const AnimTable * tbl = static_cast<const AnimTable*>(parentNode());
                Q_ASSERT(tbl);
                return tbl->data( tbl->getModel()->index(nodeIndex(), column, QModelIndex()), role);
            }
            break;
        }
    case eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return m_seqlist.size();
            break;
        }
    default:
        {
            break;
        }
    };
    return QVariant();
}

void AnimGroup::importGroup(const fmt::AnimDB::animgrp_t &grp)
{
    m_seqlist.reserve(grp.seqs.size());
    for( const auto & seq : grp.seqs )
        m_seqlist.push_back(seq);

    m_unk16 = grp.unk16;
}

fmt::AnimDB::animgrp_t AnimGroup::exportGroup()
{
    fmt::AnimDB::animgrp_t dest;
    dest.seqs.resize(m_seqlist.size());
    std::copy(m_seqlist.begin(), m_seqlist.end(), dest.seqs.begin());
    dest.unk16 = m_unk16;
    return dest;
}

void AnimGroup::removeSequenceReferences(fmt::AnimDB::animseqid_t id)
{
    for( auto & seq : m_seqlist )
    {
        if(seq == id)
            seq = -1;
    }
}

Sprite *AnimGroup::parentSprite()
{
    return static_cast<AnimTable*>(parentNode())->parentSprite();
}

//AnimGroupDelegate *AnimGroup::getDelegate()
//{
//    return m_delegate.data();
//}

//const AnimGroupDelegate *AnimGroup::getDelegate() const
//{
//    return m_delegate.data();
//}

//******************************************************************************
//  AnimGroupModel
//******************************************************************************
AnimGroupModel::AnimGroupModel(AnimGroup *pgrp, QObject *parent)
    :QAbstractItemModel(parent), m_pOwner(pgrp)
{
}

QModelIndex AnimGroupModel::index(int row, int column, const QModelIndex &parent) const
{
    if(row < rowCount(parent) && row >= 0 &&
       column < columnCount(parent) && column >= 0)
        return createIndex(row, column, row);
    return QModelIndex();
}

QModelIndex AnimGroupModel::parent(const QModelIndex &) const
{
    //        if(child.internalId() >= 0 && child.internalId() < m_pOwner->nodeColumnCount())
    return QModelIndex();
}

int AnimGroupModel::rowCount(const QModelIndex &/*parent*/) const
{
    Q_ASSERT(m_pOwner);
    return m_pOwner->seqSlots().size();
}

int AnimGroupModel::columnCount(const QModelIndex &) const
{
//    if(!parent.isValid())
//        return AnimSequences::HEADER_COLUMNS.size();
    return 1;
}

bool AnimGroupModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

QVariant AnimGroupModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole  &&
        role != Qt::EditRole     &&
        role != Qt::SizeHintRole &&
        role != Qt::DecorationRole )
        return QVariant();

    if(index.row() > m_pOwner->seqSlots().size())
    {
        Q_ASSERT(false);
        return QVariant();
    }

    if(index.column() == 0)
    {
        fmt::AnimDB::animseqid_t id = m_pOwner->seqSlots().at(index.row());
        if(role == Qt::DisplayRole)
        {
            AnimSequence *pseq = m_pOwner->parentSprite()->getAnimSequence(id);

            if(id == -1)
                return QString("INVALID");

            if(!pseq)
                return QString("MissingID:%1").arg(id);
            return QString("SequenceID:%1, %2 frames").arg(id).arg(pseq->nodeChildCount());
        }
        else if(role == Qt::DecorationRole)
        {
            AnimSequence * pseq = m_pOwner->parentSprite()->getAnimSequence(id);
            if(!pseq || id == -1)
                return QVariant();
            return QVariant(pseq->makePreview());
        }
        else if(role == Qt::EditRole)
        {
            return id;
        }
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
    }
    return QVariant();
}

QVariant AnimGroupModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole /*&& role != Qt::SizeHintRole*/)
        return QVariant();

    if(orientation == Qt::Orientation::Vertical)
    {
        return section;
    }
    else if(orientation == Qt::Orientation::Horizontal &&
            section >= 0 && section < AnimSequences::HEADER_COLUMNS.size())
    {
        return AnimSequences::HEADER_COLUMNS[section];
    }
    return QVariant();
}

bool AnimGroupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return false;

    if(index.row() > m_pOwner->seqSlots().size())
    {
        Q_ASSERT(false);
        return false;
    }

    bool bok = false;
    if(index.column() == 0)
        m_pOwner->seqSlots()[index.row()] = value.toInt(&bok);

    if(bok)
        emit dataChanged(index, index, QVector<int>{role});
    return bok;
}

bool AnimGroupModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid() || row < 0 || row > m_pOwner->seqSlots().size() )
        return false;

    beginInsertRows(parent, row, row + (row + count));
    for( int cntins = 0; cntins < count; ++cntins )
        m_pOwner->InsertRow(row,-1);
    endInsertRows();
    return true;
}

bool AnimGroupModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid() ||
       row < 0 ||
       row > m_pOwner->seqSlots().size() ||
       count > m_pOwner->seqSlots().size() )
        return false;

    beginRemoveRows(parent, row, row + (count - 1));
    for( int cnt = 0; cnt < count; ++cnt )
        m_pOwner->RemoveRow(row);
    endRemoveRows();
    return true;
}

bool AnimGroupModel::moveRows(const QModelIndex &sourceParent,
                              int sourceRow,
                              int count,
                              const QModelIndex &destinationParent,
                              int destinationChild)
{
    if(sourceParent.isValid() ||
       sourceParent != destinationParent ||
       sourceRow < 0 ||
       sourceRow > m_pOwner->seqSlots().size() ||
       destinationChild < 0 ||
       destinationChild > m_pOwner->seqSlots().size() ||
       count < 0 ||
       count > m_pOwner->seqSlots().size())
        return false;

    beginMoveRows(sourceParent, sourceRow, sourceRow + (count-1), destinationParent, destinationChild);
    QVector<fmt::AnimDB::animseqid_t> tomove;
    tomove.reserve(count);

    for( int cnt = 0; cnt < count; ++cnt )
        tomove.push_back(m_pOwner->seqSlots().takeAt(sourceRow));

    //Compute the new destination index after removing the elements to move
    int newdest = (destinationChild < sourceRow)? destinationChild : (destinationChild - count);

    for(int i = 0; i < tomove.size(); ++i)
        m_pOwner->seqSlots().insert(i + newdest, tomove[i]);

    endMoveRows();
    return true;
}

Qt::ItemFlags AnimGroupModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled;
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
