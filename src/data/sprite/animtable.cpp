#include "animtable.hpp"
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/sprite.hpp>

const char * ElemName_AnimTable     = "Animation Table";

//**********************************************************************************
//  AnimTableDelegate
//**********************************************************************************
class AnimTableDelegate : public QStyledItemDelegate
{
public:
    AnimTableDelegate(AnimTable * parent, QObject * pobj = nullptr)
        :QStyledItemDelegate(pobj), m_pOwner(parent)
    {}

    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
        if(pnode)
            return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
        return QStyledItemDelegate::sizeHint(option,index);
    }

    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget * pedit = nullptr;
        Q_ASSERT(false); //TODO
        return pedit;
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        Q_ASSERT(false); //TODO
    }

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        if(!index.isValid())
        {
            Q_ASSERT(false);
            qCritical("AnimTableDelegate::setModelData(): Index is inavalid!\n");
            return;
        }

        Sprite          * spr   = m_pOwner->parentSprite();
        const AnimGroup * pfrm  = static_cast<const AnimGroup*>(spr->getAnimTable().nodeChild(index.row()));
        Q_ASSERT(spr && pfrm);

        switch(static_cast<AnimGroup::eColumns>(index.column()))
        {
        case AnimGroup::eColumns::GroupName:
            {
                Q_ASSERT(false); //TODO
                break;
            }
        case AnimGroup::eColumns::GroupID:
            {
                Q_ASSERT(false); //TODO
                break;
            }
        case AnimGroup::eColumns::NbSlots:
        default:
            {
                QStyledItemDelegate::setModelData(editor,model,index);
                break;
            }
        };
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override
    {
        editor->setGeometry(option.rect);
    }

private:
    AnimTable * m_pOwner;
};

//**********************************************************************************
//  AnimTable
//**********************************************************************************
AnimTable::AnimTable(TreeElement *parent)
    :BaseTreeContainerChild(parent),
      m_delegate(new AnimTableDelegate(this))
{
    setNodeDataTy(eTreeElemDataType::animTable);
}

AnimTable::AnimTable(const AnimTable &cp)
    :BaseTreeContainerChild(cp),
      m_delegate(new AnimTableDelegate(this))
{
    operator=(cp);
}

AnimTable::AnimTable(AnimTable &&mv)
    :BaseTreeContainerChild(qMove(mv)),
      m_delegate(new AnimTableDelegate(this))
{
    operator=(qMove(mv));
}

AnimTable &AnimTable::operator=(const AnimTable &cp)
{
    m_slotNames = cp.m_slotNames;
    return *this;
}

AnimTable &AnimTable::operator=(AnimTable &&mv)
{
    m_slotNames = qMove(mv.m_slotNames);
    return *this;
}

void AnimTable::clone(const TreeElement *other)
{
    const AnimTable * ptr = static_cast<const AnimTable*>(other);
    if(!ptr)
        throw std::runtime_error("AnimTable::clone(): other is not a AnimTable!");
    (*this) = *ptr;
}

AnimTable::~AnimTable()
{

}

QVariant AnimTable::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

void AnimTable::importAnimationTable(const fmt::AnimDB::animtbl_t &orig)
{
    //#TODO: See if we can't load strings from a template at the same time for naming slots!
    for(auto id : orig)
        m_slotNames.insert(id,QString());
}

fmt::AnimDB::animtbl_t AnimTable::exportAnimationTable()
{
    //#FIXME:
    //For now just dummy fill it will the ids of all our groups
    //But eventually rewrite this and the import/export code, so we don't care about group ids
    //considering groups are tied to animations, and groups never are null or shared.
    fmt::AnimDB::animtbl_t dest;
    dest.reserve(nodeChildCount());
    for( int i = 0; i < nodeChildCount(); ++i )
    {
        dest.push_back(i);
    }
    return std::move(dest);
}

void AnimTable::importAnimationGroups(fmt::AnimDB::animgrptbl_t &animgrps)
{
    m_container.reserve(animgrps.size());
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, animgrps.size());

    for( fmt::AnimDB::animgrpid_t cntgrp = 0; cntgrp < static_cast<fmt::AnimDB::animgrpid_t>(animgrps.size());
         ++cntgrp )
        m_container[cntgrp].importGroup(animgrps[cntgrp]);
}

fmt::AnimDB::animgrptbl_t AnimTable::exportAnimationGroups()
{
    fmt::AnimDB::animgrptbl_t grps;
    for( int cntgrp = 0; cntgrp < nodeChildCount(); ++cntgrp )
    {
        grps[cntgrp] = std::move(m_container[cntgrp].exportGroup());
    }
    return std::move(grps);
}

void AnimTable::DeleteGroupRefs(fmt::AnimDB::animgrpid_t id)
{
    //Invalidate all references to this group!
//    for(int i = 0; i < m_slotNames.size(); ++i)
//    {
//        if(m_slotNames[i].first == id)
//            m_slotNames[i].first = -1;
//    }
}

void AnimTable::DeleteGroupChild(fmt::AnimDB::animgrpid_t id)
{
    DeleteGroupRefs(id);
    getModel()->removeRow(id);
}

QString AnimTable::getSlotName(fmt::AnimDB::animgrpid_t entry) const
{
    if(entry >= 0 && entry < m_slotNames.size() )
    {
        auto itf = m_slotNames.find(entry);
        if(itf != m_slotNames.end())
            return (*itf);
    }
    return QString();
}

Sprite *AnimTable::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

QVariant AnimTable::data(const QModelIndex &index, int role) const
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

//    if( Qt::SizeHintRole )
//    {
//        QFontMetrics fm(QFont("Sergoe UI", 9));
//        QString str = data(index, Qt::DisplayRole).toString();
//        return QSize(fm.width(str), fm.height());
//    }

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
            if(role == Qt::DisplayRole || role == Qt::EditRole)
            {
                auto itf = m_slotNames.find(index.row());
                if(itf != m_slotNames.end())
                    return (*itf);
                return QString("--");
            }
            break;
        }
    case AnimGroup::eColumns::NbSlots:
        {
            if(role == Qt::DisplayRole || role == Qt::EditRole)
                return grp->seqSlots().size();
            break;
        }
    default:
        {
            break;
        }
    };
    return QVariant();
}

QVariant AnimTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             (section >= 0) && (section < AnimGroup::ColumnNames.size()) )
    {
        return AnimGroup::ColumnNames[section];
    }
    return QVariant();
}

void AnimTable::setSlotName(fmt::AnimDB::animgrpid_t entry, const QString &name)
{
    if(entry >= 0 && entry < m_slotNames.size())
        m_slotNames[entry] = name;
}
