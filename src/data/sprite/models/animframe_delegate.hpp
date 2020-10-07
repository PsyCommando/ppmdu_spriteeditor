#ifndef ANIMFRAME_DELEGATE_HPP
#define ANIMFRAME_DELEGATE_HPP
#include <QStyledItemDelegate>
#include <QPointer>
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  AnimSequenceDelegate
//*******************************************************************
class Sprite;
class AnimSequence;
class FramesListModel;
class AnimFrameDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(AnimFrameDelegate);
public:
    AnimFrameDelegate(AnimSequence * seq, QObject *parent = nullptr);
    ~AnimFrameDelegate();

    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    //Returns the stylesheet used for comboboxes this delegate makes
    static const QString & ComboBoxStyleSheet();
    static const QString XOffsSpinBoxName;
    static const QString YOffsSpinBoxName;
    static const char*   UProp_AnimFrameID;

    QWidget * makeFrameSelect       (QWidget *parent, Sprite* spr, TreeNodeModel * pmodel)const;
    QWidget * makeOffsetWidget      (QWidget *parent)const;
    QWidget * makeShadowOffsetWidget(QWidget *parent)const;

    //Variable:
private:
    AnimSequence *m_animseq {nullptr};
    QPointer<FramesListModel> m_selectModel;
};

#endif // ANIMFRAME_DELEGATE_HPP
