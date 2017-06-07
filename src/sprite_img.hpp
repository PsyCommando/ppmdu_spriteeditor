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

    //Fill the widget with the current effect data!
    void fillList()
    {

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

    inline id_t getImageUID()const {return getID();} //Get index indepandent id!

private:
    QImage                  m_img;
    std::vector<uint8_t>    m_raw; //Need this because QImage doesn't own the buffer...
    int                     m_depth;    //Original image depth in bpp
    uint16_t                m_unk2;
    uint16_t                m_unk14;
};

//*******************************************************************
//  ImagesManager
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
//  ImageContainer
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
                return std::move(QVariant( QString("UID") ));
            case 2:
                return std::move(QVariant( QString("Bit Depth") ));
            case 3:
                return std::move(QVariant( QString("Resolution") ));
            };
        }
        return QVariant();
    }

    bool        nodeIsMutable()const override           {return false;}
    virtual int nodeColumnCount() const override        {return 4;}

    inline Image        * getImage(fmt::frmid_t id)     { return static_cast<Image*>(nodeChild(id)); }
    inline const Image  * getImage(fmt::frmid_t id)const { return static_cast<Image*>(const_cast<ImageContainer*>(this)->nodeChild(id)); }

private:
};





#endif // SPRITE_IMGDB_HPP
