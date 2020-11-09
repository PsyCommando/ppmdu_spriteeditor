#ifndef EFFECTSETDELEGATE_HPP
#define EFFECTSETDELEGATE_HPP
/*
 * Delegate to allow editing effect marker points position
*/
#include <QStyledItemDelegate>

class EffectOffsetSet;
class EffectSetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    using parent_t = QStyledItemDelegate;
public:
    static const char * UProp_OffsetIdx; //Custom property for keeping track of the offset a control is refering to.
    static const char * XOffSpinBoxName;
    static const char * YOffSpinBoxName;

    EffectSetDelegate(EffectOffsetSet * offsetset, QObject *parent = nullptr);
    ~EffectSetDelegate();
    EffectSetDelegate(const EffectSetDelegate & cp);
    EffectSetDelegate(EffectSetDelegate       && mv);
    EffectSetDelegate &operator=(const EffectSetDelegate & cp);
    EffectSetDelegate & operator=(EffectSetDelegate       && mv);

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void offsetChanged(const QModelIndex & changed)const;
private:
    QWidget *makeOffsetWidget(QWidget * parent, const char * name)const;

private:
    EffectOffsetSet * m_offsetSet {nullptr};
};

#endif // EFFECTSETDELEGATE_HPP
