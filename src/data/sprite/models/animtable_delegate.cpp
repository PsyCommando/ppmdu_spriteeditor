#include "animtable_delegate.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animgroup.hpp>
#include <src/data/treenodemodel.hpp>

AnimTableDelegate::AnimTableDelegate(AnimTable *parent, QObject *pobj)
    :QStyledItemDelegate(pobj), m_animtable(parent)
{}

AnimTableDelegate::~AnimTableDelegate()
{
    m_animtable = nullptr;
}

QSize AnimTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        const QAbstractItemModel * pmod = index.model();
        return pmod->data(index, Qt::SizeHintRole).toSize();
    }
    return QStyledItemDelegate::sizeHint(option,index);
}

QWidget *AnimTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    Q_UNUSED(index);
    QWidget * pedit = nullptr;
    Q_ASSERT(false); //TODO
    return pedit;
}

void AnimTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
    Q_ASSERT(false); //TODO
}

void AnimTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(!index.isValid())
    {
        Q_ASSERT(false);
        qCritical("AnimTableDelegate::setModelData(): Index is inavalid!\n");
        return;
    }

    TreeNodeModel   * pmod  = static_cast<TreeNodeModel*>(model);
    Q_ASSERT(pmod);
    Sprite          * spr   = pmod->getOwnerSprite();
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

void AnimTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
