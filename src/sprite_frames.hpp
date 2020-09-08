#ifndef SPRITE_FRAMES_HPP
#define SPRITE_FRAMES_HPP
#include <QVector>
#include <QList>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QTableWidget>
#include <QGraphicsScene>
#include <QPointer>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>

#include <algorithm>

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

extern const char * ElemName_FrameCnt;
extern const char * ElemName_Frame;
extern const char * ElemName_FramePart;

enum struct eFramesColumnsType : int
{
    Preview     = 0,
    ImgID,
    TileNum,
    PaletteID,
    Unk0,
    Offset,
    Flip,
    RotNScaling,
    Mosaic,
    Mode,
    Priority,
    HeaderNBColumns,


    direct_XOffset = HeaderNBColumns, //Extra column for accessing directly the x offset
    direct_YOffset, //Extra column for accessing directly the y offset
    direct_VFlip,   //Extra column for accessing directly the vflip boolean
    direct_HFlip,   //Extra column for accessing directly the hflip boolean
    NBColumns,
    INVALID,
};

extern const size_t               FramesHeaderNBColumns;
extern const std::vector<QString> FramesHeaderColumnNames;



//*******************************************************************
//  MFrameDelegate
//*******************************************************************
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

//*******************************************************************
//  MFramePart
//*******************************************************************
class MFramePart : public BaseTreeTerminalChild<&ElemName_FramePart>
{
    typedef BaseTreeTerminalChild<&ElemName_FramePart> partparent_t;
public:
    MFramePart(TreeElement * parent);
    MFramePart(TreeElement * parent, const fmt::step_t & part);
    virtual ~MFramePart();

    bool operator==( const MFramePart & other)const;
    bool operator!=( const MFramePart & other)const;

    void clone(const TreeElement *other)
    {
        const MFramePart * ptr = static_cast<const MFramePart*>(other);
        if(!ptr)
            throw std::runtime_error("MFramePart::clone(): other is not a MFramePart!");
        (*this) = *ptr;
    }

    Sprite          * parentSprite();
    const Sprite    * parentSprite()const;

public:
    virtual int             nodeColumnCount() const;
    virtual Qt::ItemFlags   nodeFlags(int column = 0)const override;
    virtual QVariant        nodeData(int column, int role) const override;


    /*
     *  drawPart
     *      Draws an image from the part's data.
     *      Optimized for displaying the part, return an image in the ARGB32_premultiplied format.
     *      If the part is a -1 frame, returns a null image!
    */
    QImage drawPart(bool transparencyenabled = false)const;

public:
    void        importPart(const fmt::step_t & part);
    fmt::step_t exportPart()const;

    fmt::step_t         & getPartData();
    const fmt::step_t   & getPartData()const;

private:
    static QSize calcTextSize( const QString & str);

    QVariant dataImgPreview     (int role) const;
    QVariant dataImgId          (int role)const;
    QVariant dataUnk0           (int role)const;
    QVariant dataOffset         (int role)const;
    QVariant dataDirectXOffset  (int role)const;
    QVariant dataDirectYOffset  (int role)const;
    QVariant dataFlip           (int role)const;
    QVariant dataDirectVFlip    (int role)const;
    QVariant dataDirectHFlip    (int role)const;
    QVariant dataRotNScaling    (int role)const;
    QVariant dataPaletteID      (int role)const;
    QVariant dataPriority       (int role)const;
    QVariant dataTileNum        (int role)const;
    QVariant dataMosaic         (int role)const;
    QVariant dataMode           (int role)const;

    //Transform the given image according to the parameters stored in this class!
    void applyTransforms(QImage & srcimg)const;

private:
    fmt::step_t  m_data;
    fmt::frmid_t m_imgUID;
};

//*******************************************************************
//  MFrame
//*******************************************************************
class MFrame : public BaseTreeContainerChild<&ElemName_Frame,MFramePart>
{
    friend class MFrameDelegate;
    //Dynamic property for edit controls, so we can keep track of which part they edit!
    static const char * PropPartID;
    typedef BaseTreeContainerChild<&ElemName_Frame,MFramePart> paren_t;

public:
    MFrame( TreeElement * parentNode );
    MFrame(const MFrame & cp);
    MFrame(MFrame      && mv);
    MFrame &  operator=(const MFrame & cp);
    MFrame &  operator=(MFrame      && mv);

    void clone(const TreeElement *other)
    {
        const MFrame * ptr = static_cast<const MFrame*>(other);
        if(!ptr)
            throw std::runtime_error("MFrame::clone(): other is not a MFrame!");
        (*this) = *ptr;
    }

    bool operator==( const MFrame & other)const;
    bool operator!=( const MFrame & other)const;

    void importFrame(const fmt::ImageDB::frm_t & frms)
    {
        getModel()->removeRows(0, nodeChildCount());
        getModel()->insertRows(0, frms.size());

        auto itparts = frms.begin();
        for( size_t cntid = 0; cntid < frms.size(); ++cntid, ++itparts )
        {
            m_container[cntid].importPart(*itparts);
        }
    }

    fmt::ImageDB::frm_t exportFrame()const
    {
        fmt::ImageDB::frm_t lst;
        for(const MFramePart & part : m_container)
            lst.push_back(part.exportPart());
        return qMove(lst);
    }

    fmt::step_t         *getPart(int id);
    const fmt::step_t   *getPart(int id)const;
    inline int          getNbParts()const {return nodeChildCount();}

    Sprite * parentSprite();
    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect * out_area = nullptr) const;
    //cropto: A rectangle whose width and height will be used to crop the assembled frame. Mainly useful to get even-sized frames.
    //        A default QRect is ignored and the image will be cropped to our discretion.
    QImage  AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect *out_area = nullptr, bool makebgtransparent = true)const;
    QRect   calcFrameBounds()const;

    //Thos can be re-implemented!
    virtual int         nodeColumnCount() const override;
    virtual QVariant    nodeData(int column, int role) const override;

    //data method of the model and childs!!!
    //We need this so that we can virtually display entries for each steps of the frame!
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QVariant frameDataCondensed(int role)const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //setData method of the model and childs!!!
    virtual bool setData(const QModelIndex  &index,
                         const QVariant     &value,
                         int                role = Qt::EditRole)override;

    virtual int columnCount(const QModelIndex &parentNode) const;

    QVariant DataForAPart(int row, int column, int role) const;

    virtual int rowCount(const QModelIndex &parent)const override
    {
        if(parent.isValid())
            return static_cast<TreeElement*>(parent.internalPointer())->nodeChildCount();
        else
            return nodeChildCount();
    }

    inline MFrameDelegate & itemDelegate()
    {
        return m_delegate;
    }

    //For now UID is index!
    int getFrameUID()const
    {
        return nodeIndex();
    }
private:
    MFrameDelegate m_delegate;
};


//*******************************************************************
//  FramesContainer
//*******************************************************************
class FramesContainer : public BaseTreeContainerChild<&ElemName_FrameCnt, MFrame>
{

public:

    FramesContainer( TreeElement * parent )
        :BaseTreeContainerChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::frames);
    }

    ~FramesContainer()
    {
        qDebug("FrameContainer::~FrameContainer()\n");
    }

    void clone(const TreeElement *other)
    {
        const FramesContainer * ptr = static_cast<const FramesContainer*>(other);
        if(!ptr)
            throw std::runtime_error("FramesContainer::clone(): other is not a FramesContainer!");
        (*this) = *ptr;
    }

    //Elem data
    QVariant nodeData(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    void importFrames( const fmt::ImageDB::frmtbl_t & frms )
    {
        getModel()->removeRows(0, nodeChildCount());
        getModel()->insertRows(0, frms.size());

        for( size_t cntid = 0; cntid < frms.size(); ++cntid )
            m_container[cntid].importFrame(frms[cntid]);
    }

    fmt::ImageDB::frmtbl_t exportFrames()
    {
        fmt::ImageDB::frmtbl_t frms(nodeChildCount());
        for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        {
            frms[cntid] = std::move(m_container[cntid].exportFrame());
        }
        return std::move(frms);
    }

    Sprite * parentSprite();

    MFrame * getFrame(fmt::frmid_t id)
    {
        return static_cast<MFrame*>(nodeChild(id));
    }

    const MFrame * getFrame(fmt::frmid_t id)const
    {
        return const_cast<FramesContainer*>(this)->getFrame(id);
    }

    //Model data
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole &&
                role != Qt::DecorationRole &&
                role != Qt::SizeHintRole &&
                role != Qt::EditRole)
            return QVariant();

        const MFrame *frm = static_cast<const MFrame*>(getItem(index));
        return frm->nodeData(index.column(), role);
    }

    //Model headerdata
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if( role != Qt::DisplayRole )
            return QVariant();

       if( orientation == Qt::Orientation::Vertical )
        {
            return std::move(QVariant( QString("%1").arg(section) ));
        }
        else if( orientation == Qt::Orientation::Horizontal &&
                 static_cast<size_t>(section) < FramesHeaderNBColumns )
        {
            return FramesHeaderColumnNames[section];
        }
        return QVariant();
    }

    bool        nodeIsMutable()const override    {return false;}
    virtual int nodeColumnCount() const override {return FramesHeaderNBColumns;}

private:

};

#endif // SPRITE_FRAMES_HPP
