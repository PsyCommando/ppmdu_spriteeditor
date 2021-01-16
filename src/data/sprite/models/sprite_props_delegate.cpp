#include "sprite_props_delegate.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/sprite_props_model.hpp>
#include <QComboBox>

//=================================================================================================================
//  SpritePropertiesDelegate
//=================================================================================================================
SpritePropertiesDelegate::SpritePropertiesDelegate(Sprite *parentspr, QObject *parent)
    :QStyledItemDelegate(parent), m_spr(parentspr)
{
}

SpritePropertiesDelegate::~SpritePropertiesDelegate()
{
}

QSize SpritePropertiesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize ret = QStyledItemDelegate::sizeHint(option, index);
    if(index.column() == static_cast<int>(eSpritePropColumns::Value))
    {
        ret.setWidth( ret.width() + PropValueColLen);
    }
    return ret;
}

QWidget *SpritePropertiesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //Qt takes ownership of the widget we create!
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
        return QStyledItemDelegate::createEditor(parent,option,index);

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = new QComboBox(parent);
            pspritetypes->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pspritetypes->setAutoFillBackground(true);
            pspritetypes->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

            for(const auto & s : fmt::SpriteTypeNames)
                pspritetypes->addItem(QString::fromStdString(s));
            pspritetypes->removeItem(fmt::SpriteTypeNames.size()-1); //remove the last item since we don't want to allow the user to pick it!
            return pspritetypes;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = new QComboBox(parent);
            pcompty->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pcompty->setAutoFillBackground(true);
            pcompty->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

            for(const auto & s : CompressionFmtOptions)
                pcompty->addItem(s);
            return pcompty;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = new QComboBox(parent);
            pcolmode->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
            pcolmode->setAutoFillBackground(true);
            pcolmode->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
            for(const auto & s : SpriteColorModes)
                pcolmode->addItem(s);
            return pcolmode;
        }
    case eSpriteProperties::Unk7:   [[fallthrough]];
    case eSpriteProperties::Unk8:   [[fallthrough]];
    case eSpriteProperties::Unk9:   [[fallthrough]];
    case eSpriteProperties::Unk10:  [[fallthrough]];
    case eSpriteProperties::Unk11:  [[fallthrough]];
    case eSpriteProperties::Unk12:  [[fallthrough]];
    case eSpriteProperties::Unk13:  [[fallthrough]];
    default:
        break; //Avoid warnings remove when attributes properly supported
    };

    return QStyledItemDelegate::createEditor(parent,option,index);
}

void SpritePropertiesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //Qt takes ownership of the widget we create!
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = static_cast<QComboBox*>(editor);
            Q_ASSERT(pspritetypes);
            pspritetypes->setCurrentIndex(static_cast<int>(m_spr->type()));
            return;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcompty);
            pcompty->setCurrentIndex( static_cast<int>(CompFmtToCompOption(m_spr->getTargetCompression())) );
            return;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcolmode);
            pcolmode->setCurrentIndex((m_spr->is256Colors())? 1 : 0);
            return;
        }
    case eSpriteProperties::Unk7:   [[fallthrough]];
    case eSpriteProperties::Unk8:   [[fallthrough]];
    case eSpriteProperties::Unk9:   [[fallthrough]];
    case eSpriteProperties::Unk10:  [[fallthrough]];
    case eSpriteProperties::Unk11:  [[fallthrough]];
    case eSpriteProperties::Unk12:  [[fallthrough]];
    case eSpriteProperties::Unk13:  [[fallthrough]];
    default:
        break; //Avoid warnings remove when attributes properly supported
    };
    QStyledItemDelegate::setEditorData(editor, index);
}

void SpritePropertiesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column() != static_cast<int>(eSpritePropColumns::Value))
    {
        QStyledItemDelegate::setModelData(editor,model,index);
        return;
    }

    switch(static_cast<eSpriteProperties>(index.row()))
    {
    case eSpriteProperties::SpriteType:
        {
            QComboBox * pspritetypes = static_cast<QComboBox*>(editor);
            Q_ASSERT(pspritetypes);
            model->setData( index, pspritetypes->currentIndex() );
            return;
        }
    case eSpriteProperties::Compression:
        {
            QComboBox * pcompty = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcompty);
            model->setData( index, pcompty->currentIndex() );
            return;
        }
    case eSpriteProperties::ColorMode:
        {
            QComboBox * pcolmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pcolmode);
            model->setData(index, pcolmode->currentIndex());
            return;
        }
    case eSpriteProperties::Unk7:   [[fallthrough]];
    case eSpriteProperties::Unk8:   [[fallthrough]];
    case eSpriteProperties::Unk9:   [[fallthrough]];
    case eSpriteProperties::Unk10:  [[fallthrough]];
    case eSpriteProperties::Unk11:  [[fallthrough]];
    case eSpriteProperties::Unk12:  [[fallthrough]];
    case eSpriteProperties::Unk13:  [[fallthrough]];
    default:
        break; //Avoid warnings remove when attributes properly supported
    };
    QStyledItemDelegate::setModelData(editor,model,index);
}

void SpritePropertiesDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
