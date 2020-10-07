#ifndef ANIMGROUPDELEGATE_HPP
#define ANIMGROUPDELEGATE_HPP
#include <QStyledItemDelegate>

//*******************************************************************
//  AnimGroupDelegate
//*******************************************************************
//Delegate for editing animation group data
class AnimGroup;
class AnimGroupDelegate : public QStyledItemDelegate
{
public:
    AnimGroupDelegate(AnimGroup * animgroup);
    ~AnimGroupDelegate();

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    AnimGroup * m_animgroup{nullptr};
};

#endif // ANIMGROUPDELEGATE_HPP
