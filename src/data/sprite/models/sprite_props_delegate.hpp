#ifndef SPRITEPROPERTIESDELEGATE_HPP
#define SPRITEPROPERTIESDELEGATE_HPP
#include <QStyledItemDelegate>

//*******************************************************************
//  SpritePropertiesDelegate
//*******************************************************************
//Delegate for handling editing the sprite properties data
class Sprite;
class SpritePropertiesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Sprite          *m_spr{nullptr};
    static const int PropValueColLen = 128;
public:

    SpritePropertiesDelegate(Sprite * parentspr, QObject * parent = nullptr);
    virtual ~SpritePropertiesDelegate();

    // QAbstractItemDelegate interface
public:
    virtual QSize   sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void    setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void    setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void    updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override;
};

#endif // SPRITEPROPERTIESDELEGATE_HPP
