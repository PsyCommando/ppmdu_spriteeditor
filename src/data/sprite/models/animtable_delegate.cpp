#include "animtable_delegate.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animgroup.hpp>
#include <src/data/treenodemodel.hpp>
#include <QComboBox>

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
        throw std::runtime_error("AnimTableDelegate::setModelData(): Index is inavalid!\n");
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

//QWidget *AnimTableDelegate::makeGroupSelect(QWidget *parent, const QModelIndex & index) const
//{
//    const TreeNodeModel     *model      = static_cast<const TreeNodeModel*>(index.model());
//    QComboBox               *cmbGroup   = new QComboBox(parent);
//    const ImageContainer    *pcnt       = &(model->getOwnerSprite()->getImages());
//    const Sprite            *spr        = model->getOwnerSprite();
//    imglstb->setIconSize( QSize(32,32) );
//    imglstb->setStyleSheet(pcnt->ComboBoxStyleSheet());

//    //Fill the combobox!

//    //Add nodraw frame
//    imglstb->addItem( QPixmap::fromImage(m_minusone), MFramePart_NoDrawFrame, QVariant(-1)); //Set user data to -1
//    //Add actual images!
//    for( int cntimg = 0; cntimg < pcnt->nodeChildCount(); ++cntimg )
//    {
//        const Image* pimg = pcnt->getImage(cntimg);
//        QPixmap pmap = QPixmap::fromImage(pimg->makeImage(spr->getPalette()));
//        QString text = pimg->getImageDescription();
//        imglstb->addItem( QIcon(pmap), text, QVariant(pimg->getID()) ); //Set user data to image's UID
//    }
//    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
//    return imglstb;
//}
