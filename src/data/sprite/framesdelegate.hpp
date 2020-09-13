#ifndef FRAMESDELEGATE_HPP
#define FRAMESDELEGATE_HPP
#include <src/data/treeelem.hpp>
#include <QStyledItemDelegate>

//*******************************************************************
//  MFrameDelegate
//*******************************************************************
// Delegate for handling editing data from the frame in the frame editor!
class MFrame;
class MFrameDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    //Can't define in cpp, because of "tr", so to make things simpler, I just put it in a static function..
    static const QStringList & prioritiesNames()
    {
        static const QStringList PRIO
        {
            QString(tr("0- Highest")),
            QString(tr("1- High")),
            QString(tr("2- Low")),
            QString(tr("3- Lowest")),
        };
        return PRIO;
    }

    static const QStringList & modeNames()
    {
        static const QStringList Modes
        {
            QString(tr("Normal")),
            QString(tr("Blended")),
            QString(tr("Windowed")),
            QString(tr("Bitmap")),
        };
        return Modes;
    }

public:
    MFrameDelegate(MFrame * frm, QObject *parent = nullptr);
    MFrameDelegate(const MFrameDelegate & cp);
    MFrameDelegate(MFrameDelegate       && mv);
    MFrameDelegate & operator=(const MFrameDelegate & cp);
    MFrameDelegate & operator=(MFrameDelegate       && mv);
    virtual ~MFrameDelegate();

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

private:

    static constexpr const char * ImgSelCmbBoxName(){return "cmbImgSelect";}
    static constexpr const char * ImgSelBtnName()      {return "btnImgSelect";}
    //Make the selector for picking the image id for a given row!
    QWidget * makeImgSelect(QWidget *parent, int row)const;

    static constexpr const char * VFlipChkBoxName() {return "chkVflip";}
    static constexpr const char * HFlipChkBoxName() {return "chkHflip";}
    QWidget * makeFlipSelect(QWidget *parent, int row)const;

    static constexpr const char * OffsetXSpinBoxName() {return "spbXOff";}
    static constexpr const char * OffsetYSpinBoxName() {return "spbYOff";}
    QWidget * makeOffsetSelect(QWidget *parent, int row)const;

    static constexpr const char * RotNScaleChkBoxName() {return "chkRnS";}
    static constexpr const char * RotNScaleBtnName()   {return "btnRnS";}
    QWidget * makeRotNScalingSelect(QWidget *parent, int row)const;


    QWidget * makePaletteIDSelect(QWidget * parent, int row)const;
    QWidget * makePrioritySelect (QWidget * parent, int row)const;
    QWidget * makeTileIdSelect   (QWidget * parent, int row)const;
    QWidget * makeModeSelect     (QWidget * parent, int row)const;

private:
    MFrame *m_pfrm{nullptr};
    QImage  m_minusone;
};

#endif // FRAMESDELEGATE_HPP
