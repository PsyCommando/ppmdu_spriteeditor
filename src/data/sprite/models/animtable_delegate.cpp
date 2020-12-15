#include "animtable_delegate.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animgroup.hpp>
#include <src/data/sprite/animgroups_container.hpp>
#include <src/data/sprite/animtable.hpp>
#include <src/data/treenodemodel.hpp>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>

AnimTableDelegate::AnimTableDelegate(AnimTable *parent, QObject *pobj)
    :QStyledItemDelegate(pobj), m_animtable(parent)
{
}

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

QWidget *AnimTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    QWidget * pedit = nullptr;
    if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupId)
        pedit = makeGroupSelect(parent, index);
    else if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupName)
        pedit = makeNameEdit(parent, index);

    //Common properties
    if(pedit != nullptr)
    {
        //pedit->setAutoFillBackground(true);
        //pedit->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        //pedit->setProperty(UProp_OffsetIdx, index.row());
    }
    return pedit;
}

void AnimTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(!index.isValid())
        return;
    const AnimTableSlot * pgrp  = static_cast<const AnimTableSlot*>(index.internalPointer());

    if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupId)
    {
        QComboBox *cmb = static_cast<QComboBox*>(editor);
        Q_ASSERT(cmb);
        fmt::animgrpid_t ref = pgrp->getGroupRef();
        if(ref == fmt::NullGrpIndex)
            cmb->setCurrentIndex(0);
        else
            cmb->setCurrentIndex(ref + 1); //Since 0 is reserved increment always by one
    }
    else if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupName)
    {
        QLineEdit *txtname = dynamic_cast<QLineEdit*>(editor);
        Q_ASSERT(txtname);
        txtname->setText(pgrp->getSlotName());
    }
    else
        return QStyledItemDelegate::setEditorData(editor,index);
}

void AnimTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(!index.isValid())
    {
        Q_ASSERT(false);
        throw std::runtime_error("AnimTableDelegate::setModelData(): Index is inavalid!\n");
    }

    if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupId)
    {
        QComboBox *cmb = dynamic_cast<QComboBox*>(editor);
        Q_ASSERT(cmb);
        model->setData(index, cmb->currentData(), Qt::EditRole);
    }
    else if(static_cast<AnimTableSlot::eColumns>(index.column()) == AnimTableSlot::eColumns::GroupName)
    {
        QLineEdit *txtname = dynamic_cast<QLineEdit*>(editor);
        Q_ASSERT(txtname);
        model->setData(index, txtname->text(), Qt::EditRole);
    }
    else
        return QStyledItemDelegate::setModelData(editor, model,index);
}

void AnimTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

void AnimTableDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

QComboBox *AnimTableDelegate::makeGroupSelect(QWidget *parent, const QModelIndex & index) const
{
    const TreeNodeModel     *model      = static_cast<const TreeNodeModel*>(index.model());
    QComboBox               *grplstb    = new QComboBox(parent);
    const AnimGroups        *pcnt       = &(model->getOwnerSprite()->getAnimGroups());
    const Sprite            *spr        = model->getOwnerSprite();
    grplstb->setIconSize( QSize(256,32) );
    grplstb->setStyleSheet(m_animtable->ComboBoxStyleSheet());

    //Fill the combobox!

    //Add nodraw frame
    grplstb->addItem( QIcon(), tr("None"), QVariant(-1)); //Set user data to -1
    //Add actual images!
    for( int cntgrp = 0; cntgrp < pcnt->nodeChildCount(); ++cntgrp )
    {
        const AnimGroup* pgrp = pcnt->getGroup(cntgrp);
        QPixmap pmap = pgrp->MakeGroupPreview(spr, 256, 32, 8);
        grplstb->addItem( QIcon(pmap), QString("ID %1").arg(pgrp->getGroupUID()), QVariant(pgrp->getGroupUID()) ); //Set user data to group UID
    }
    grplstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    connect(grplstb, qOverload<int>(&QComboBox::activated), this, &AnimTableDelegate::commitAndCloseEditor);
    return grplstb;
}

QLineEdit *AnimTableDelegate::makeNameEdit(QWidget *parent, const QModelIndex &/*index*/) const
{
    QLineEdit *txtname = new QLineEdit(parent);
    txtname->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    return txtname;
}
