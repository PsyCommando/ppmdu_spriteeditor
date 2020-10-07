#ifndef ANIMTABLEDELEGATE_HPP
#define ANIMTABLEDELEGATE_HPP
#include <QStyledItemDelegate>

//**********************************************************************************
//  AnimTableDelegate
//**********************************************************************************
class AnimTable;
class AnimTableDelegate : public QStyledItemDelegate
{
public:
    AnimTableDelegate(AnimTable * parent, QObject * pobj = nullptr);
    ~AnimTableDelegate();

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    AnimTable * m_animtable {nullptr};
};
#endif // ANIMTABLEDELEGATE_HPP
