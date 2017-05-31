#ifndef SPRITE_IMGDB_HPP
#define SPRITE_IMGDB_HPP
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

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>


class Sprite;
extern const char * ElemName_EffectOffset ;
extern const char * ElemName_Palette      ;
extern const char * ElemName_Images       ;
extern const char * ElemName_Image        ;
extern const char * ElemName_FrameCnt     ;
extern const char * ElemName_Frame        ;
extern const char * ElemName_FramePart    ;

enum struct eFramesColumnsType : int
{
    Preview     = 0,
    ImgID,
    Unk0,
    Offset,
    Flip,
    RotNScaling,
    PaletteID,
    Priority,
    CharName,
    NBColumns,
    INVALID,
};

extern const size_t               FramesHeaderNBColumns;
extern const std::vector<QString> FramesHeaderColumnNames;

//*******************************************************************
//
//*******************************************************************
class EffectOffsetContainer : public BaseTreeTerminalChild<&ElemName_EffectOffset>
{
public:

    EffectOffsetContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::effectOffsets);
    }

    ~EffectOffsetContainer()
    {
        qDebug("EffectOffsetContainer::~EffectOffsetContainer()\n");
    }

    QVariant nodeData(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    Sprite * parentSprite();

    std::vector<fmt::effectoffset> exportEffects()const
    {
        return m_efx;
    }

    void importEffects( const std::vector<fmt::effectoffset> & efx )
    {
        m_efx = efx;
    }

    //Fill the widget with the current effect data!
    void fillList()
    {

    }

private:
    std::vector<fmt::effectoffset> m_efx;
};

//*******************************************************************
//
//*******************************************************************
class PaletteContainer : public BaseTreeTerminalChild<&ElemName_Palette>
{
public:

    PaletteContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::palette);
    }

    ~PaletteContainer()
    {
        qDebug("PaletteContainer::~PaletteContainer()\n");
    }

    QVariant nodeData(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    Sprite * parentSprite();

    QVector<QRgb> m_pal;
};

//*******************************************************************
//
//*******************************************************************

class Image : public BaseTreeTerminalChild<&ElemName_Image>
{
public:
    Image(TreeElement * parent)
        :BaseTreeTerminalChild(parent), m_depth(0), m_unk2(0), m_unk14(0)
    {
        setNodeDataTy(eTreeElemDataType::image);
    }

    inline bool operator==( const Image & other)const  {return this == &other;}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

    void importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h)
    {
        m_depth = 4;
        QVector<QRgb> dummy(16);
        m_raw = qMove( utils::Untile( w, h, utils::Expand4BppTo8Bpp(img.data) ) );
        m_img = utils::RawToImg( w, h, m_raw, dummy );
        m_unk2 = img.unk2;
        m_unk14 = img.unk14;
    }

    fmt::ImageDB::img_t exportImage4bpp(int & w, int & h)const
    {
        w = m_img.width();
        h = m_img.height();
        fmt::ImageDB::img_t img;
        img.data = std::move(utils::Reduce8bppTo4bpp(utils::TileFromImg(m_img)));
        img.unk2 = m_unk2;
        img.unk14 = m_unk14;
        return qMove(img);
    }

    void importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h)
    {
        m_depth = 8;
        QVector<QRgb> dummy(256);
        m_raw = utils::Untile(w, h, img.data);
        m_img = utils::RawToImg( w, h, m_raw, dummy);
        m_unk2 = img.unk2;
        m_unk14 = img.unk14;
    }

    fmt::ImageDB::img_t exportImage8bpp(int & w, int & h)const
    {
        w = m_img.width();
        h = m_img.height();
        fmt::ImageDB::img_t img;
        img.data = qMove(utils::TileFromImg(m_img));
        img.unk2 = m_unk2;
        img.unk14 = m_unk14;
        return qMove(img);
    }

    QImage makeImage( const QVector<QRgb> & palette )const
    {
        QImage img(m_img);
        img.setColorTable(palette);
        return qMove(img);
    }

    QSize getImageSize()const
    {
        return m_img.size();
    }

    Sprite       * parentSprite();
    const Sprite * parentSprite()const {return const_cast<Image*>(this)->parentSprite();}

    int nbimgcolumns()const
    {
        return 3;
    }

    //Those can be re-implemented!
    QVariant imgData(int column, int role) const;

private:
    QImage              m_img;
    std::vector<uint8_t> m_raw; //Need this because QImage doesn't own the buffer...
    int                 m_depth;    //Original image depth in bpp
    uint16_t            m_unk2;
    uint16_t            m_unk14;
};

//*******************************************************************
//
//*******************************************************************
class ImageContainer;
class ImagesManager : public QAbstractItemModel
{
    Q_OBJECT
    ImageContainer * m_parentcnt;
    // QAbstractItemModel interface
public:
    ImagesManager(ImageContainer * parent);

    virtual ~ImagesManager();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    bool hasChildren(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    TreeElement *getItem(const QModelIndex &index);
};

//*******************************************************************
//
//*******************************************************************
class ImageContainer : public BaseTreeContainerChild<&ElemName_Images, Image>
{
public:
    ImageContainer( TreeElement * parent)
        :BaseTreeContainerChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::images);
    }

    ImageContainer( const ImageContainer & cp)
        :BaseTreeContainerChild(cp)
    {}

    ImageContainer( ImageContainer && mv)
        :BaseTreeContainerChild(mv)
    {}

    ~ImageContainer()
    {
        qDebug("ImageContainer::~ImageContainer()\n");
    }

    ImageContainer & operator=( const ImageContainer & cp )
    {
        BaseTreeContainerChild::operator=(cp);
        return *this;
    }

    ImageContainer & operator=( ImageContainer && mv )
    {
        BaseTreeContainerChild::operator=(mv);
        return *this;
    }

    QVariant nodeData(int column, int role) const override
    {
        if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    void importImages8bpp(const fmt::ImageDB::imgtbl_t & imgs, const fmt::ImageDB::frmtbl_t & frms)
    {
        Q_ASSERT(false);
    }

    void importImages4bpp(const fmt::ImageDB::imgtbl_t & imgs, const fmt::ImageDB::frmtbl_t & frms)
    {
        removeChildrenNodes(0, nodeChildCount());
        insertChildrenNodes(0, imgs.size());
        int w = 256;
        int h = 256;

        for( size_t cntid = 0; cntid < imgs.size(); ++cntid )
        {
            for( size_t frmid = 0; frmid < frms.size(); ++frmid )
            {
                auto itstep = frms[frmid].begin();
                for( size_t stepid= 0; stepid < frms[frmid].size(); ++stepid, ++itstep )
                {
                    if( itstep->frmidx == cntid)
                    {
                        auto res = itstep->GetResolution();
                        w = res.first;
                        h = res.second;
                    }

                }
            }
            m_container[cntid].importImage4bpp(imgs[cntid], w, h );
        }
    }

    fmt::ImageDB::imgtbl_t exportImages4bpp()
    {
        int w = 0;
        int h = 0;
        fmt::ImageDB::imgtbl_t images(nodeChildCount());
        for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        {
            images[cntid] = std::move(m_container[cntid].exportImage4bpp(w,h));
        }
        return std::move(images);
    }

    fmt::ImageDB::imgtbl_t exportImages8bpp()
    {
        int w = 0;
        int h = 0;
        fmt::ImageDB::imgtbl_t images(nodeChildCount());
        for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        {
            images[cntid] = std::move(m_container[cntid].exportImage8bpp(w,h));
        }
        return std::move(images);
    }


    Sprite * parentSprite();

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole &&
                role != Qt::DecorationRole &&
                role != Qt::SizeHintRole &&
                role != Qt::EditRole)
            return QVariant();

        const Image *img = static_cast<const Image*>(getItem(index));
        return img->imgData(index.column(), role);
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if( role != Qt::DisplayRole )
            return QVariant();

        if( orientation == Qt::Orientation::Vertical )
        {
            return std::move(QVariant( QString("%1").arg(section) ));
        }
        else if( orientation == Qt::Orientation::Horizontal )
        {
            switch(section)
            {
            case 0:
                return std::move(QVariant( QString("") ));
            case 1:
                return std::move(QVariant( QString("Bit Depth") ));
            case 2:
                return std::move(QVariant( QString("Resolution") ));
            };
        }
        return QVariant();
    }

    virtual int nodeColumnCount() const                 {return 3;}

    inline Image        * getImage(fmt::frmid_t id)     { return static_cast<Image*>(nodeChild(id)); }
    inline const Image  * getImage(fmt::frmid_t id)const { return static_cast<Image*>(const_cast<ImageContainer*>(this)->nodeChild(id)); }

private:
};

//*******************************************************************
//
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

    MFrameDelegate(MFrame * frm, QObject *parent = nullptr);
    MFrameDelegate(const MFrameDelegate & cp)
        :QStyledItemDelegate()
    {
        operator=(cp);
    }

    MFrameDelegate(MFrameDelegate && mv)
        :QStyledItemDelegate()
    {
        operator=(mv);
    }

    MFrameDelegate & operator=(const MFrameDelegate & cp)
    {
        m_pfrm    = cp.m_pfrm;
        return *this;
    }

    MFrameDelegate & operator=(MFrameDelegate && mv)
    {
        m_pfrm    = mv.m_pfrm;
        mv.m_pfrm = nullptr;
        return *this;
    }

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

    QSize sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const override
    {
        TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
        if(pnode)
            return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
        else
            return QSize();
    }

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

    QWidget * makePrioritySelect(QWidget * parent, int row)const;

    QWidget * makeTileIdSelect(QWidget * parent, int row)const;

private:
    MFrame      *m_pfrm;
};

//*******************************************************************
//
//*******************************************************************
class MFramePart : public BaseTreeTerminalChild<&ElemName_FramePart>
{
    typedef BaseTreeTerminalChild<&ElemName_FramePart> partparent_t;
public:
    //static const QStringList PRIORITIES;

    MFramePart(TreeElement * parent)
        :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags())
    {
        setNodeDataTy(eTreeElemDataType::framepart);
    }

    MFramePart(TreeElement * parent, const fmt::step_t & part)
        :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags()), m_data(part)
    {
        setNodeDataTy(eTreeElemDataType::framepart);
    }

    virtual ~MFramePart()
    {
    }

    bool operator==( const MFramePart & other)const
    {
        return m_data.attr0 == other.m_data.attr0 &&
               m_data.attr1 == other.m_data.attr1 &&
               m_data.attr2 == other.m_data.attr2;
    }
    bool operator!=( const MFramePart & other)const
    {
        return !operator==(other);
    }

    Sprite * parentSprite();

public:
    virtual int nodeColumnCount() const                 {return FramesHeaderNBColumns;}

    virtual Qt::ItemFlags nodeFlags(int column = 0)const override
    {
        if(column == static_cast<int>(eFramesColumnsType::Preview))
            return Qt::ItemFlags(m_flags).setFlag(Qt::ItemFlag::ItemIsEditable, false); //The preview is never editable!
        return m_flags;
    }

    virtual QVariant nodeData(int column, int role) const override
    {
        if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
                return QVariant();

        switch(static_cast<eFramesColumnsType>(column))
        {
        case eFramesColumnsType::Preview:       return dataImgPreview(role);
        case eFramesColumnsType::ImgID:         return dataImgId(role);
        case eFramesColumnsType::Unk0:          return dataUnk0(role);
        case eFramesColumnsType::Offset:        return dataOffset(role);
        case eFramesColumnsType::Flip:          return dataFlip(role);
        case eFramesColumnsType::RotNScaling:   return dataRotNScaling(role);
        case eFramesColumnsType::PaletteID:     return dataPaletteID(role);
        case eFramesColumnsType::Priority:      return dataPriority(role);
        case eFramesColumnsType::CharName:      return dataCharName(role);

            //Undefined cases
        default:
            break;
        };

        return QVariant();
    }



public:
    void importPart(const fmt::step_t & part)
    {
        m_data = part;
    }

    fmt::step_t exportPart()const
    {
        return m_data;
    }

    fmt::step_t & getPartData()
    {
        return m_data;
    }

    const fmt::step_t & getPartData()const
    {
        return m_data;
    }



private:
    static QSize calcTextSize( const QString & str)
    {
        static QFontMetrics fm(QFont("Sergoe UI", 9));
        return QSize(fm.width(str), fm.height());
    }


    QVariant dataImgPreview(int role) const;

    QVariant dataImgId(int role)const
    {
        if(role == Qt::DisplayRole)
        {
            return QString("Img#%1").arg(static_cast<int>(m_data.getFrameIndex()));
        }
        else if(role == Qt::EditRole)
        {
            //Just output image id
            return static_cast<int>(m_data.getFrameIndex());
        }
        else if(role == Qt::SizeHintRole)
        {
            QSize sz = calcTextSize(dataImgId(Qt::DisplayRole).toString());
            sz.setWidth( sz.width() + 150 );
            sz.setHeight(sz.height() + 72);
            return sz;
        }
        return QVariant();
    }

    QVariant dataUnk0(int role)const
    {
        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return static_cast<int>(m_data.unk0);
        }
        else if(role == Qt::SizeHintRole)
        {
            return calcTextSize(dataUnk0(Qt::DisplayRole).toString());
        }
        return QVariant();
    }

    QVariant dataOffset(int role)const
    {
        if(role == Qt::DisplayRole)
        {
            return QString("(%1, %2)").arg(m_data.getXOffset()).arg(m_data.getYOffset());
        }
        else if(role == Qt::EditRole)
        {
            QVariant res;
            res.setValue(QPair<int,int>(m_data.getXOffset(), m_data.getYOffset()));
            return res;
        }
        else if(role == Qt::SizeHintRole)
        {
            QSize sz = calcTextSize(dataOffset(Qt::DisplayRole).toString());
            sz.setWidth( sz.width() + 80 );
            return sz;
        }
        return QVariant();
    }

    QVariant dataFlip(int role)const
    {
        if(role == Qt::DisplayRole)
        {
            QString flipval;
            if(m_data.isVFlip())
                flipval += "V ";
            if(m_data.isHFlip())
                flipval += "H ";
            return flipval;
        }
        else if(role == Qt::EditRole)
        {
            QVariant res;
            res.setValue(QPair<bool,bool>(m_data.isVFlip(), m_data.isHFlip()));
            return res;
        }
        else if(role == Qt::SizeHintRole)
        {
            QSize sz = calcTextSize(dataFlip(Qt::DisplayRole).toString());
            sz.setWidth( sz.width() + 80 );
            return sz;
        }
        return QVariant();
    }

    QVariant dataRotNScaling(int role)const
    {
        if(role == Qt::DisplayRole)
        {
            return m_data.isRotAndScalingOn();
        }
        else if(role == Qt::EditRole)
        {
            //#TODO: will need a custom struct here to properly send over RnS data!
            return m_data.isRotAndScalingOn();
        }
        else if(role == Qt::SizeHintRole)
        {
            QSize sz = calcTextSize(dataRotNScaling(Qt::DisplayRole).toString());
            sz.setWidth( sz.width() + 80 );
            return sz;
        }
        return QVariant();
    }

    QVariant dataPaletteID(int role)const
    {
        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return static_cast<int>(m_data.getPalNb());
        }
        else if(role == Qt::SizeHintRole)
        {
            return calcTextSize(dataPaletteID(Qt::DisplayRole).toString());
        }
        return QVariant();
    }

    QVariant dataPriority(int role)const
    {
        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return MFrameDelegate::prioritiesNames().at(m_data.getPriority());
        }
        else if(role == Qt::SizeHintRole)
        {
            QSize sz = calcTextSize(dataPriority(Qt::DisplayRole).toString());
            sz.setWidth( sz.width() + 50 );
            return sz;
        }
        return QVariant();
    }

    QVariant dataCharName(int role)const
    {
        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return static_cast<int>(m_data.getTileNum());
        }
        else if(role == Qt::SizeHintRole)
        {
            return calcTextSize(dataCharName(Qt::DisplayRole).toString());
        }
        return QVariant();
    }

private:
    fmt::step_t m_data;
};

//*******************************************************************
//
//*******************************************************************
class MFrame : public BaseTreeContainerChild<&ElemName_Frame,MFramePart>
{
    //Q_OBJECT
    friend class MFrameDelegate;
    //Dynamic property for edit controls, so we can keep track of which part they edit!
    static const char * PropPartID;
    typedef BaseTreeContainerChild<&ElemName_Frame,MFramePart> paren_t;

public:
    MFrame( TreeElement * parentNode );

    MFrame(const MFrame & cp);
    MFrame(MFrame && mv);
    MFrame &  operator=(const MFrame & cp);
    MFrame &  operator=(MFrame && mv);

    bool operator==( const MFrame & other)const;
    bool operator!=( const MFrame & other)const;

    void importFrame(const fmt::ImageDB::frm_t & frms)
    {
        removeChildrenNodes(0, nodeChildCount());
        insertChildrenNodes(0, frms.size());

        auto itparts = frms.begin();
        for( fmt::frmid_t cntid = 0; cntid < frms.size(); ++cntid, ++itparts )
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

    fmt::step_t *getPart(int id);
    const fmt::step_t *getPart(int id)const;

    Sprite * parentSprite();

    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect * out_area = nullptr) const;

    QImage AssembleFrame(int xoffset, int yoffset, QRect *out_area = nullptr, bool makebgtransparent = true)const;
    QRect calcFrameBounds()const;


//    fmt::ImageDB::frm_t         & getParts();
//    const fmt::ImageDB::frm_t   & getParts()const;

    //Thos can be re-implemented!
    virtual int nodeColumnCount() const override;

    virtual QVariant nodeData(int column, int role) const override;

    //data method of the model and childs!!!
    //We need this so that we can virtually display entries for each steps of the frame!
    virtual QVariant data(const QModelIndex &index, int role) const override;

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

private:


//    int getPartIdFromSender(QObject * sender)
//    {
//        if(sender)
//        {
//            QVariant val = sender->property(PropPartID);
//            if(val.isNull())
//                return -1;
//            if(val.Int < 0 || val.Int >= m_parts.size())
//                return -1;
//            else
//                return val.Int;
//        }
//        return -1;
//    }

private:
    //fmt::ImageDB::frm_t m_parts;
//    BaseTreeNodeModel   m_model; //Virtual model for displaying steps for this frame!
    MFrameDelegate      m_delegate;
};


//*******************************************************************
//
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

    //Elem data
    QVariant nodeData(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    void importFrames( const fmt::ImageDB::frmtbl_t & frms )
    {
        removeChildrenNodes(0, nodeChildCount());
        insertChildrenNodes(0, frms.size());

        for( fmt::frmid_t cntid = 0; cntid < frms.size(); ++cntid )
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
        else if( orientation == Qt::Orientation::Horizontal && section < FramesHeaderNBColumns )
        {
            return FramesHeaderColumnNames[section];
        }
        return QVariant();
    }

    virtual int nodeColumnCount() const override {return FramesHeaderNBColumns;}

private:

};



#endif // SPRITE_IMGDB_HPP
