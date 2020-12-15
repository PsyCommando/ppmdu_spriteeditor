#include "effect_set_delegate.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/effectoffsetset.hpp>
#include <src/data/sprite/models/effect_set_model.hpp>
#include <QSpinBox>

const char * EffectSetDelegate::UProp_OffsetIdx = "OffsetIdx";
const char * EffectSetDelegate::XOffSpinBoxName = "XOffset";
const char * EffectSetDelegate::YOffSpinBoxName = "YOffset";

EffectSetDelegate::EffectSetDelegate(EffectOffsetSet * offsetset, QObject *parent)
    :parent_t(parent)
{
    m_offsetSet = offsetset;
    Q_ASSERT(m_offsetSet);
}

EffectSetDelegate::~EffectSetDelegate()
{
}

EffectSetDelegate::EffectSetDelegate(const EffectSetDelegate &cp)
    :parent_t(parent())
{
    operator=(cp);
}

EffectSetDelegate::EffectSetDelegate(EffectSetDelegate &&mv)
    :parent_t(parent())
{
    operator=(mv);
}

EffectSetDelegate &EffectSetDelegate::operator=(const EffectSetDelegate &cp)
{
    m_offsetSet = cp.m_offsetSet;
    return *this;
}

EffectSetDelegate &EffectSetDelegate::operator=(EffectSetDelegate &&mv)
{
    m_offsetSet = mv.m_offsetSet;
    return *this;
}

QSize EffectSetDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if(!index.isValid())
        return QSize();
    const EffectSetModel* pmod = static_cast<const EffectSetModel*>(index.model());
    Q_ASSERT(pmod);
    return pmod->data(index, Qt::SizeHintRole).toSize();
}

QWidget *EffectSetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *               pedit = nullptr;
    //const EffectSetModel *  pmod  = static_cast<const EffectSetModel*>(index.model());
    //const Sprite *          owner = pmod->getOwnerSprite();

    switch(static_cast<EffectSetModel::eColumns>(index.column()))
    {
    case EffectSetModel::eColumns::XOffset:
        {
            pedit = makeOffsetWidget(parent, XOffSpinBoxName);
            break;
        }
    case EffectSetModel::eColumns::YOffset:
        {
            pedit = makeOffsetWidget(parent, YOffSpinBoxName);
            break;
        }
    case EffectSetModel::eColumns::Name:
    default:
        {
            return QStyledItemDelegate::createEditor(parent,option,index);
            break;
        }
    };

    //Common properties
    if(pedit != nullptr)
    {
        pedit->setAutoFillBackground(true);
        pedit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        pedit->setProperty(UProp_OffsetIdx, index.row());
    }
    return pedit;
}

void EffectSetDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    EffectSetModel * pmod = static_cast<EffectSetModel*>(const_cast<QAbstractItemModel*>(index.model()));
    Q_ASSERT(pmod);
    EffectOffset * poff = static_cast<EffectOffset*>(pmod->getItem(index));
    if(!poff)
    {
        Q_ASSERT(false);
        throw std::runtime_error("EffectOffsetDelegate::setEditorData(): Index is inavalid!\n");
    }

    switch(static_cast<EffectSetModel::eColumns>(index.column()))
    {
    case EffectSetModel::eColumns::XOffset:
        {
            QSpinBox *px = static_cast<QSpinBox*>(editor);
            Q_ASSERT(px);
            px->setValue(poff->getX());
            break;
        }
    case EffectSetModel::eColumns::YOffset:
        {
            QSpinBox *py = static_cast<QSpinBox*>(editor);
            Q_ASSERT(py);
            py->setValue(poff->getY());
            break;
        }
    case EffectSetModel::eColumns::Name:
    default:
        {
            QStyledItemDelegate::setEditorData(editor,index);
            break;
        }
    };
}

void EffectSetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    EffectSetModel * pmod = static_cast<EffectSetModel*>(const_cast<QAbstractItemModel*>(index.model()));
    Q_ASSERT(pmod);
    EffectOffset * poff = static_cast<EffectOffset*>(pmod->getItem(index));
    if(!poff)
    {
        Q_ASSERT(false);
        throw std::runtime_error("EffectOffsetDelegate::setModelData(): Index is inavalid!\n");
    }

    switch(static_cast<EffectSetModel::eColumns>(index.column()))
    {
    case EffectSetModel::eColumns::XOffset:
        {
            QSpinBox *px = static_cast<QSpinBox*>(editor);
            Q_ASSERT(px);
            px->interpretText();
            QVariant val;
            val.setValue(px->value());
            model->setData(index, val, Qt::EditRole);
            break;
        }
    case EffectSetModel::eColumns::YOffset:
        {
            QSpinBox *py = static_cast<QSpinBox*>(editor);
            Q_ASSERT(py);
            py->interpretText();
            QVariant val;
            val.setValue(py->value());
            model->setData(index, val, Qt::EditRole);
            break;
        }
    case EffectSetModel::eColumns::Name:
    default:
        {
            QStyledItemDelegate::setModelData(editor,model,index);
            break;
        }
    };
    emit offsetChanged(index);
}

void EffectSetDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

QWidget *EffectSetDelegate::makeOffsetWidget(QWidget *parent, const char * name)const
{
    QSpinBox * pxoff = new QSpinBox(parent);
    pxoff->setObjectName(name);
    pxoff->setRange(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
    parent->adjustSize();
    return pxoff;
}
