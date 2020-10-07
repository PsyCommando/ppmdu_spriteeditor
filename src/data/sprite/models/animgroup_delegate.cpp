#include "animgroup_delegate.hpp"
#include <src/data/sprite/animgroup.hpp>

AnimGroupDelegate::AnimGroupDelegate(AnimGroup *owner)
    :QStyledItemDelegate(nullptr), m_animgroup(owner)
{
}

AnimGroupDelegate::~AnimGroupDelegate()
{
    m_animgroup = nullptr;
}

QSize AnimGroupDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        TreeNode * pnode = static_cast<TreeNode*>(index.internalPointer());
        const QAbstractItemModel * pmodel = index.model();
        if(pnode)
            return pmodel->data(index, Qt::SizeHintRole).toSize();
    }
    return QStyledItemDelegate::sizeHint(option,index);
}

QWidget *AnimGroupDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    Q_UNUSED(index);
    qWarning("unimplemented!");
    return nullptr;
}

void AnimGroupDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
    qWarning("unimplemented!");
}

void AnimGroupDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}

void AnimGroupDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}
