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

//*******************************************************************
//
//*******************************************************************
class EffectOffsetContainer : public BaseTreeTerminalChild<&ElemName_EffectOffset>
{
public:

    EffectOffsetContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {
        setElemTy(eTreeElemType::Fixed);
        setDataTy(eTreeElemDataType::effectOffsets);
    }

    ~EffectOffsetContainer()
    {
        qDebug("EffectOffsetContainer::~EffectOffsetContainer()\n");
    }

    QVariant data(int column, int role) const override
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
        setElemTy(eTreeElemType::Fixed);
        setDataTy(eTreeElemDataType::palette);
    }

    ~PaletteContainer()
    {
        qDebug("PaletteContainer::~PaletteContainer()\n");
    }

    QVariant data(int column, int role) const override
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
        setDataTy(eTreeElemDataType::image);
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

//    void makeImageTableRow( QTableWidget * tbl, const QVector<QRgb> & pal, int rowid )
//    {
//        //0. Image Preview
//        QLabel * preview = new QLabel("");

//        tbl->setCellWidget(rowid, 0, preview);

//        preview->setScaledContents(true);
//        preview->setAlignment(Qt::AlignCenter);
//        preview->setMinimumWidth(m_img.width());
//        preview->setMinimumHeight(m_img.height());
//        preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

//        if( tbl->horizontalHeader()->sectionSize(0 < m_img.width() ) )
//        {
//            tbl->setColumnWidth(0, m_img.width());
//            tbl->horizontalHeader()->resizeSection(0, m_img.width());
//        }
//        if( tbl->verticalHeader()->sectionSize(rowid) < m_img.height() )
//        {
//            tbl->setRowHeight(rowid, m_img.height());
//            tbl->verticalHeader()->resizeSection(rowid, m_img.height());
//        }

//        preview->setPixmap(makePixmap(pal).scaled(preview->width(), preview->height(), Qt::AspectRatioMode::KeepAspectRatio));

//        //1. BPP
//        QTableWidgetItem * bpp = new QTableWidgetItem( QString("%1").arg(m_depth) );
//        bpp->setFlags( bpp->flags() & (~Qt::ItemFlag::ItemIsEditable) );
//        tbl->setItem(rowid, 1, bpp);

//        //2. Resolution
//        QTableWidgetItem * resolution = new QTableWidgetItem( QString("%1x%2").arg(m_img.width()).arg(m_img.height()) );
//        resolution->setFlags( resolution->flags() & (~Qt::ItemFlag::ItemIsEditable) );
//        tbl->setItem(rowid, 2, resolution);
//    }

    Sprite       * parentSprite();
    const Sprite * parentSprite()const {return const_cast<Image*>(this)->parentSprite();}

    int nbimgcolumns()const
    {
        return 3;
    }

    //Those can be re-implemented!
    QVariant imgData(int column, int role) const;

private:
//    int         m_width;
//    int         m_height;
    QImage              m_img;
    std::vector<uint8_t> m_raw; //Need this because QImage doesn't own the buffer...
    int                 m_depth;    //Original image depth in bpp
    uint16_t            m_unk2;
    uint16_t            m_unk14;
};


/*
*/
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
class ImageContainer : public BaseListContainerChild<&ElemName_Images, Image>
{
public:
    ImageContainer( TreeElement * parent)
        :BaseListContainerChild(parent)
    {
        setElemTy(eTreeElemType::Fixed);
        setDataTy(eTreeElemDataType::images);
    }

    ImageContainer( const ImageContainer & cp)
        :BaseListContainerChild(cp)
    {}

    ImageContainer( ImageContainer && mv)
        :BaseListContainerChild(mv)
    {}

    ~ImageContainer()
    {
        qDebug("ImageContainer::~ImageContainer()\n");
    }

    ImageContainer & operator=( const ImageContainer & cp )
    {
        BaseListContainerChild::operator=(cp);
        return *this;
    }

    ImageContainer & operator=( ImageContainer && mv )
    {
        BaseListContainerChild::operator=(mv);
        return *this;
    }

    QVariant data(int column, int role) const override
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
        removeChildren(0, childCount());
        insertChildren(0, imgs.size());
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
        fmt::ImageDB::imgtbl_t images(childCount());
        for( int cntid = 0; cntid < childCount(); ++cntid )
        {
            images[cntid] = std::move(m_container[cntid].exportImage4bpp(w,h));
        }
        return std::move(images);
    }

    fmt::ImageDB::imgtbl_t exportImages8bpp()
    {
        int w = 0;
        int h = 0;
        fmt::ImageDB::imgtbl_t images(childCount());
        for( int cntid = 0; cntid < childCount(); ++cntid )
        {
            images[cntid] = std::move(m_container[cntid].exportImage8bpp(w,h));
        }
        return std::move(images);
    }

//    void fillImgListTable(QTableWidget * tbl, const QVector<QRgb> & pal)
//    {
//        tbl->setUpdatesEnabled(false);
//        tbl->clearContents();
//        tbl->setRowCount(childCount());
//        //tbl->setColumnCount(3);

//        for( size_t cnti = 0; cnti < childCount(); ++cnti )
//        {
//            Image * pimg = static_cast<Image*>(child(cnti));
//            if(pimg)
//                pimg->makeImageTableRow(tbl, pal, cnti);
//            else
//                Q_ASSERT(false);
//        }
//        tbl->setUpdatesEnabled(true);
//    }

    Sprite * parentSprite();


    //ImagesManager * getModel(){return m_manager.data();}

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
                return std::move(QVariant( QString("Preview") ));
            case 1:
                return std::move(QVariant( QString("Bit Depth") ));
            case 2:
                return std::move(QVariant( QString("Resolution") ));
            };
        }
        return QVariant();
    }

    inline Image * getImage(fmt::frmid_t id) { return static_cast<Image*>(child(id)); }
    inline const Image * getImage(fmt::frmid_t id)const { return static_cast<Image*>(const_cast<ImageContainer*>(this)->child(id)); }

private:
    //QScopedPointer<ImagesManager> m_manager;

};

//*******************************************************************
//
//*******************************************************************
class MFrame : public BaseTreeTerminalChild<&ElemName_Frame>
{
public:
    MFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {
        setDataTy(eTreeElemDataType::frame);
    }

    inline bool operator==( const MFrame & other)const  {return this == &other;}
    inline bool operator!=( const MFrame & other)const  {return !operator==(other);}

//    inline fmt::frmid_t getID()const {return m_id;}
//    inline void setID(fmt::frmid_t id) {m_id = id;}

    void importFrame(const fmt::ImageDB::frm_t & frm/*, fmt::frmid_t id*/)
    {
//        m_id    = id;
        m_parts = frm;
    }

    fmt::ImageDB::frm_t exportFrame()
    {
        return m_parts;
    }

    Sprite * parentSprite();

    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect * out_area = nullptr) const
    {
        return qMove( QPixmap::fromImage(AssembleFrame(xoffset, yoffset, out_area)) );
    }

    QImage AssembleFrame(int xoffset, int yoffset, QRect *out_area = nullptr)const;
    QRect calcFrameBounds()const;


    fmt::ImageDB::frm_t & getParts(){return m_parts;}
    const fmt::ImageDB::frm_t & getParts()const {return m_parts;}


private:
    //fmt::frmid_t        m_id;
    fmt::ImageDB::frm_t m_parts;
};


//*******************************************************************
//
//*******************************************************************
class FramesContainer : public BaseListContainerChild<&ElemName_FrameCnt, MFrame>
{
public:

    FramesContainer( TreeElement * parent )
        :BaseListContainerChild(parent)
    {
        setElemTy(eTreeElemType::Fixed);
        setDataTy(eTreeElemDataType::frames);
    }

    ~FramesContainer()
    {
        qDebug("FrameContainer::~FrameContainer()\n");
    }

    QVariant data(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }


    void importFrames( const fmt::ImageDB::frmtbl_t & frms )
    {
        removeChildren(0, childCount());
        insertChildren(0, frms.size());

        for( fmt::frmid_t cntid = 0; cntid < frms.size(); ++cntid )
            m_container[cntid].importFrame(frms[cntid]);
    }

    fmt::ImageDB::frmtbl_t exportFrames()
    {
        fmt::ImageDB::frmtbl_t frms(childCount());
        for( int cntid = 0; cntid < childCount(); ++cntid )
        {
            frms[cntid] = std::move(m_container[cntid].exportFrame());
        }
        return std::move(frms);
    }

    Sprite * parentSprite();

    MFrame * getFrame(fmt::frmid_t id)
    {
        return static_cast<MFrame*>(child(id));
    }

    const MFrame * getFrame(fmt::frmid_t id)const
    {
        return const_cast<FramesContainer*>(this)->getFrame(id);
    }

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
        return frm->data(index.column(), role);
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
//        if( role != Qt::DisplayRole )
//            return QVariant();

//        if( orientation == Qt::Orientation::Vertical )
//        {
//            return std::move(QVariant( QString("%1").arg(section) ));
//        }
//        else if( orientation == Qt::Orientation::Horizontal )
//        {
//            switch(section)
//            {
//            case 0:
//                return std::move(QVariant( QString("Preview") ));
//            case 1:
//                return std::move(QVariant( QString("Bit Depth") ));
//            case 2:
//                return std::move(QVariant( QString("Resolution") ));
//            };
//        }
        Q_ASSERT(false);
        return QVariant();
    }

private:

};



#endif // SPRITE_IMGDB_HPP
