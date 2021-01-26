#ifndef FRAME_PART_DELEGATE_HPP
#define FRAME_PART_DELEGATE_HPP
#include <src/data/treenode.hpp>
#include <QStyledItemDelegate>

//*******************************************************************
//  MFramePartDelegate
//*******************************************************************
// Delegate for handling editing data for mframe parts
class MFrame;
class MFramePartDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MFramePartDelegate(MFrame * frm, QObject *parent = nullptr);
    MFramePartDelegate(const MFramePartDelegate & cp);
    MFramePartDelegate(MFramePartDelegate       && mv);
    MFramePartDelegate & operator=(const MFramePartDelegate & cp);
    MFramePartDelegate & operator=(MFramePartDelegate       && mv);
    virtual ~MFramePartDelegate();

    /*
        createEditor
            Makes the control for editing the data in a cell.
    */
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    /*
        setEditorData
            Initialize the editor with the data.
    */
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    QSize sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const override;

    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;


private:

    static constexpr const char * ImgSelCmbBoxName()    {return "cmbImgSelect";}
    static constexpr const char * ImgSelBtnName()       {return "btnImgSelect";}
    //Make the selector for picking the image id for a given row!
    QWidget * makeImgSelect(QWidget *parent, const QModelIndex & index)const;

    QWidget * makeVFlipSelect(QWidget *parent, int row)const;
    QWidget * makeHFlipSelect(QWidget *parent, int row)const;

    QWidget * makeXOffsetSelect(QWidget *parent, int row)const;
    QWidget * makeYOffsetSelect(QWidget *parent, int row)const;

    QWidget * makePaletteIDSelect       (QWidget * parent, const QModelIndex & index)const;
    QWidget * makePrioritySelect        (QWidget * parent, int row)const;
    QWidget * makeTileIdSelect          (QWidget * parent, int row)const;
    QWidget * makeModeSelect            (QWidget * parent, int row)const;
    QWidget * makeRotnScalingParamSelect(QWidget * parent)const;

private:
    MFrame *m_pfrm{nullptr};
    QImage  m_minusone;

};

#endif // FRAME_PART_DELEGATE_HPP
