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
#include <QVariant>

#include <algorithm>

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

class Sprite;
extern const char * ElemName_EffectOffset ;
extern const char * ElemName_Images       ;
extern const char * ElemName_Image        ;



//====================================================================
//  EffectOffsetContainer
//====================================================================
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

    bool nodeIsMutable()const override {return false;}

private:
    std::vector<fmt::effectoffset> m_efx;
};

//*******************************************************************
//  Image
//*******************************************************************
//
class Image : public BaseTreeTerminalChild<&ElemName_Image>,
              public utils::BaseSequentialIDGen< BaseTreeTerminalChild<&ElemName_Image>, unsigned int>
{
public:
    Image(TreeElement * parent)
        :BaseTreeTerminalChild(parent), BaseSequentialIDGen(), m_depth(0), m_unk2(0), m_unk14(0)
    {
        setNodeDataTy(eTreeElemDataType::image);
    }

    Image(Image && mv)
        :BaseTreeTerminalChild(std::move(mv)), BaseSequentialIDGen(std::move(mv))
    {
        operator=(std::move(mv));
    }

    Image(const Image & cp)
        :BaseTreeTerminalChild(cp), BaseSequentialIDGen() //make a brand new id on copies
    {
        operator=(cp);
    }

    Image & operator=(Image && mv)
    {
        m_img   = qMove(mv.m_img);
        m_raw   = qMove(mv.m_raw);
        m_depth = mv.m_depth;
        m_unk2  = mv.m_unk2;
        m_unk14 = mv.m_unk14;
        return *this;
    }

    Image & operator=(const Image & cp)
    {
        m_img   = cp.m_img;
        m_raw   = cp.m_raw;
        m_depth = cp.m_depth;
        m_unk2  = cp.m_unk2;
        m_unk14 = cp.m_unk14;
        return *this;
    }

    inline bool operator==( const Image & other)const  {return this == &other;}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

    //
    //Image stuff
    //
public:
    void importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled)
    {
        m_depth = 4;
        QVector<QRgb> dummy(16);
        if(isTiled)
            m_raw = qMove( utils::Untile( w, h, utils::Expand4BppTo8Bpp(img.data) ) );
        else
            m_raw = img.data;
        m_img = qMove( utils::RawToImg(w, h, m_raw, dummy) );
        m_unk2 = img.unk2;
        m_unk14 = img.unk14;
    }

    fmt::ImageDB::img_t exportImage4bpp(int & w, int & h, bool tiled)const
    {
        w = m_img.width();
        h = m_img.height();
        fmt::ImageDB::img_t img;

        if(tiled)
            img.data = qMove(utils::Reduce8bppTo4bpp(utils::TileFromImg(m_img)));
        else
            img.data = std::move(utils::Reduce8bppTo4bpp(m_img));

        img.unk2 = m_unk2;
        img.unk14 = m_unk14;
        return qMove(img);
    }

    void importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h, bool isTiled)
    {
        m_depth = 8;
        QVector<QRgb> dummy(256);

        if(isTiled)
            m_raw = qMove( utils::Untile(w, h, img.data) );
        else
            m_raw = img.data;

        m_img = qMove( utils::RawToImg(w, h, m_raw, dummy) );
        m_unk2 = img.unk2;
        m_unk14 = img.unk14;
    }

    fmt::ImageDB::img_t exportImage8bpp(int & w, int & h, bool tiled)const
    {
        w = m_img.width();
        h = m_img.height();
        fmt::ImageDB::img_t img;

        if(tiled)
            img.data = qMove(utils::TileFromImg(m_img));
        else
            img.data = qMove(utils::ImgToRaw(m_img));
        img.unk2 = m_unk2;
        img.unk14 = m_unk14;
        return qMove(img);
    }

    /*
        Generate a displayable QImage using the specified palette and the image data currently stored.
    */
    QImage makeImage( const QVector<QRgb> & palette )const
    {
        QImage img(m_img);
        img.setColorTable(palette);
        return img;
    }

    //Return bounding rectangle for the image contained
    inline QSize getImageSize()const{return m_img.size();}

    inline int getImageOriginalDepth()const {return m_depth;}

    //Reimplemented nodeData method specifically for displaying images in the image list table!
    virtual QVariant imgData(int column, int role) const;
    virtual QVariant imgDataCondensed(int role) const;

    //Reimplemented nodeColumnCount specifically for displaying images in the image list table!
    virtual int nbimgcolumns()const{return 3;}

    //Returns the session unique id for this image
    inline id_t getImageUID()const {return getID();} //Get index indepandent id!

    //
    //BaseTreeTerminalChild overrides
    //
public:
    Sprite       * parentSprite();
    const Sprite * parentSprite()const {return const_cast<Image*>(this)->parentSprite();}



protected:
    QImage                  m_img;
    std::vector<uint8_t>    m_raw; //Need this because QImage doesn't own the buffer...
    int                     m_depth;    //Original image depth in bpp
    uint16_t                m_unk2;
    uint16_t                m_unk14;
};

//*******************************************************************
//  ImagesManager
//*******************************************************************
//Model for displaying the image list on the image list tab!
class ImageContainer;
class ImagesManager : public QAbstractItemModel
{
    Q_OBJECT
protected:
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
    virtual QVariant data(const QModelIndex &index, int role) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    TreeElement *getItem(const QModelIndex &index);
    const TreeElement *getItem(const QModelIndex &index)const
    {
        return const_cast<ImagesManager*>(this)->getItem(index);
    }
};

//============================================================================================
//  ImageSelectorModel
//============================================================================================
// Model for selecting an image in a popup dialog. Mainly intended for the frames tab
//class ImageSelectorModel : public ImagesManager
//{
//    Q_OBJECT
//    /*
//        "Virtual" image entry for allowing users to set a frame so its a -1 frame!
//    */
//    class NullFirstEntry : public Image
//    {
//    public:
//        using Image::Image;

//        // Image interface
//    public:
//        virtual QVariant imgDataCondensed(int role) const override;
//    };

//    QScopedPointer<NullFirstEntry> m_minusoneimg;

//public:
//    ImageSelectorModel(ImageContainer * pcnt);


//    // QAbstractItemModel interface
//public:
//    virtual int columnCount(const QModelIndex &/*parent*/) const override {return 1;}

//    virtual QVariant data(const QModelIndex &index, int role) const override;

//    virtual bool insertRows(int /*row*/, int /*count*/, const QModelIndex &/*parent*/) override {return false;}
//    virtual bool removeRows(int /*row*/, int /*count*/, const QModelIndex &/*parent*/) override {return false;}
//    virtual bool moveRows(const QModelIndex &/*sourceParent*/,
//                          int /*sourceRow*/,
//                          int /*count*/,
//                          const QModelIndex &/*destinationParent*/,
//                          int /*destinationChild*/) override {return false;}

//    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;

//    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

//    virtual int rowCount(const QModelIndex &parent) const override;
//    virtual bool hasChildren(const QModelIndex &parent) const override;

//    TreeElement *getItem(const QModelIndex &index);
//    const TreeElement *getItem(const QModelIndex &index)const
//    {
//        return const_cast<ImageSelectorModel*>(this)->getItem(index);
//    }
//};

//*******************************************************************
//  ImageContainer
//*******************************************************************
//Contains all the loaded images, and displays them in the appropriate view
class ImageContainer : public BaseTreeContainerChild<&ElemName_Images, Image>
{
public:
    ImageContainer( TreeElement * parent)
        :BaseTreeContainerChild(parent)//, m_pimgselmodel(new ImageSelectorModel(this))
    {
        setNodeDataTy(eTreeElemDataType::images);
    }

    ImageContainer( const ImageContainer & cp)
        :BaseTreeContainerChild(cp)//, m_pimgselmodel(new ImageSelectorModel(this))
    {}

    ImageContainer( ImageContainer && mv)
        :BaseTreeContainerChild(mv)//, m_pimgselmodel(new ImageSelectorModel(this))
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

//
//Image container stuff
//
public:
    static const QString & ComboBoxStyleSheet()
    {
        static const QString SSheet("QComboBox QAbstractItemView::item {margin-top: 2px;}");
        return SSheet;
    }

    /*
        importImages
            Imports a table of raw images into the container, and convert them to a displayable format!
    */
    void importImages(const fmt::ImageDB::imgtbl_t & imgs,
                      const fmt::ImageDB::frmtbl_t & frms);

    inline void importImages8bpp(const fmt::ImageDB::imgtbl_t & imgs, const fmt::ImageDB::frmtbl_t & frms)
    {
        importImages(imgs, frms);
    }

    inline void importImages4bpp(const fmt::ImageDB::imgtbl_t & imgs, const fmt::ImageDB::frmtbl_t & frms)
    {
        importImages(imgs, frms);
    }

    /*
        exportImages
            Exports the child images nodes back into their raw format!
    */
    fmt::ImageDB::imgtbl_t exportImages();
    fmt::ImageDB::imgtbl_t exportImages4bpp();
    fmt::ImageDB::imgtbl_t exportImages8bpp();

    /*
        getImage
            Helper method that casts the childs directly to the appropriate type.
    */
    inline Image        * getImage(fmt::frmid_t id)     { return static_cast<Image*>(nodeChild(id)); }
    inline const Image  * getImage(fmt::frmid_t id)const { return static_cast<Image*>(const_cast<ImageContainer*>(this)->nodeChild(id)); }


    //ImageSelectorModel * getImageSelectModel() {return m_pimgselmodel.data();}

//
//BaseContainerChild overrides
//
public:
    QVariant nodeData(int column, int role) const override
    {
        if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
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
                return std::move(QVariant( QString("UID") ));
            case 2:
                return std::move(QVariant( QString("Bit Depth") ));
            case 3:
                return std::move(QVariant( QString("Resolution") ));
            };
        }
        return QVariant();
    }

    //Whether the node should be movable
    bool        nodeIsMutable()const override           {return false;}
    virtual int nodeColumnCount() const override        {return 4;}

private:
    //QScopedPointer<ImageSelectorModel> m_pimgselmodel;
};





#endif // SPRITE_IMGDB_HPP
