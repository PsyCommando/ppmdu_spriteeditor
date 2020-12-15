#ifndef ANIMTABLEDELEGATE_HPP
#define ANIMTABLEDELEGATE_HPP
#include <QStyledItemDelegate>
#include <QObject>
#include <QComboBox>

//**********************************************************************************
//  AnimTableDelegate
//**********************************************************************************
// Used to edit anim table entries
class AnimTable;
class AnimTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
    Q_DISABLE_COPY_MOVE(AnimTableDelegate);
public:
    explicit AnimTableDelegate(AnimTable * parent, QObject * pobj = nullptr);
    ~AnimTableDelegate();

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public slots:
    void commitAndCloseEditor();

private:
    QComboBox *makeGroupSelect(QWidget * parent, const QModelIndex & index)const;
    QLineEdit *makeNameEdit(QWidget * parent, const QModelIndex & index)const;

private:
    AnimTable * m_animtable {nullptr};
};
#endif // ANIMTABLEDELEGATE_HPP
