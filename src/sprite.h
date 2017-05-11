#ifndef SPRITE_H
#define SPRITE_H
#include <QByteArray>
#include <QStack>
#include <QHash>
#include <QPixmap>
#include <QRgb>
#include <QImage>
#include <QVector>
#include <QAbstractItemModel>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <cassert>
#include <cstdint>
#include <list>
#include <src/treeelem.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>


extern const char * ElemName_EffectOffset ;
extern const char * ElemName_Palette      ;
extern const char * ElemName_Images       ;
extern const char * ElemName_Image        ;
extern const char * ElemName_FrameCnt     ;
extern const char * ElemName_Frame        ;
extern const char * ElemName_AnimSequence ;
extern const char * ElemName_AnimSequences;
extern const char * ElemName_AnimTable    ;
extern const char * ElemName_AnimGroup    ;

//============================================================================================
//
//============================================================================================

/*******************************************************************
 * BaseTreeTerminalChild
 *  Base class for implementing terminal tree nodes elements!
*******************************************************************/
template<const char** _STRELEMNAME>
    class BaseTreeTerminalChild : public TreeElement
{
protected:
    constexpr QString ElemName()const
    {
        return QString(*_STRELEMNAME);
    }

public:
    BaseTreeTerminalChild(TreeElement * parent)
        :TreeElement(parent)
    {}

    virtual ~BaseTreeTerminalChild() {}


    TreeElement *child(int) override    {return nullptr;}
    int childCount() const override         {return 0;}
    int childNumber() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOf(const_cast<BaseTreeTerminalChild*>(this));

        return 0;
    }

    int indexOf( TreeElement* )const override  {return 0;}

    //Thos can be re-implemented!
    virtual int columnCount() const                 {return 1;}

    virtual QVariant data(int column) const
    {
        if( column != 0 )
            return QVariant();
        QString sprname = QString("%1#%2").arg(ElemName()).arg(childNumber());
        return QVariant(sprname);
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

};


/*******************************************************************
 * BaseListContainerChild
*******************************************************************/
    template<const char** _STRELEMNAME, class _CHILD_TY>
        class BaseListContainerChild : public TreeElement
    {
    protected:
        constexpr QString ElemName()const
        {
            return QString(*_STRELEMNAME);
        }
        typedef _CHILD_TY               child_t;
        typedef QList<child_t> container_t;

        typedef BaseListContainerChild<_STRELEMNAME, _CHILD_TY> my_t;

    public:
        BaseListContainerChild(TreeElement * parent)
            :TreeElement(parent)
        {}

        BaseListContainerChild(const my_t & cp)
            :TreeElement(cp),m_container(cp.m_container)
        {}

        BaseListContainerChild(my_t && mv)
            :TreeElement(std::forward(mv)),m_container(std::move(mv.m_container))
        {}

        my_t & operator=(const my_t & cp)
        {
            m_container = cp.m_container;
            return *this;
        }

        my_t & operator=(my_t && mv)
        {
            m_container = std::move(mv.m_container);
            return *this;
        }

        virtual ~BaseListContainerChild() {}


        TreeElement *child(int row) override    {return &m_container[row];}
        int childCount() const override         {return m_container.size();}
        int childNumber() const override
        {
            if (m_parentItem)
                return m_parentItem->indexOf(const_cast<BaseListContainerChild*>(this));

            return 0;
        }

        int indexOf( TreeElement * ptr )const override
        {
            child_t * ptras = static_cast<child_t *>(ptr);
            //Search a matching child in the list!
            if( ptras )
                return m_container.indexOf(*ptras);
            return 0;
        }

        //Thos can be re-implemented!
        virtual int columnCount() const                 {return 1;}

        virtual QVariant data(int column) const
        {
            if( column != 0 )
                return QVariant();
            QString sprname = QString("%1#%2").arg(ElemName()).arg(childNumber());
            return QVariant(sprname);
        }

        bool insertChildren(int position, int count) override
        {
            int i = 0;
            for( ; i < count; ++i )
                m_container.insert(position, child_t(this) );
            return true;
        }

        bool removeChildren(int position, int count) override
        {
            if( (position + count) >= m_container.size() )
                return false;

            int i = 0;
            for( ; i < count; ++i )
                m_container.removeAt(position);
            return true;
        }

    protected:
        container_t m_container;

    };

//============================================================================================
//
//============================================================================================

class Sprite;

//*******************************************************************
//
//*******************************************************************
class EffectOffsetContainer : public BaseTreeTerminalChild<&ElemName_EffectOffset>
{
public:

    EffectOffsetContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

    Sprite * parentSprite();
};

//*******************************************************************
//
//*******************************************************************
class PaletteContainer : public BaseTreeTerminalChild<&ElemName_Palette>
{
public:

    PaletteContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
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
        :BaseTreeTerminalChild(parent), m_depth(0)
    {
        setDataTy(eTreeElemDataType::image);
    }

    inline bool operator==( const Image & other)const  {return this == &other;}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

    void importImage4bpp(const fmt::ImageDB::img_t & img, int w, int h)
    {
        m_depth = 4;
        QVector<QRgb> dummy(16);
        m_raw = std::move( utils::Untile( w, h, utils::Expand4BppTo8Bpp(img) ) );
        m_img = utils::RawToImg( w, h, m_raw, dummy );
    }

    fmt::ImageDB::img_t exportImage4bpp(int & w, int & h)
    {
        fmt::ImageDB::img_t imgtmp(utils::TileFromImg(m_img));
        return std::move(utils::Reduce8bppTo4bpp(imgtmp));
    }

    void importImage8bpp(const fmt::ImageDB::img_t & img, int w, int h)
    {
        m_depth = 8;
        QVector<QRgb> dummy(256);
        m_raw = utils::Untile(w, h, img);
        m_img = utils::RawToImg( w, h, m_raw, dummy);
    }

    fmt::ImageDB::img_t exportImage8bpp(int & w, int & h)
    {
        return std::move(utils::TileFromImg(m_img));
    }

    QPixmap makePixmap( const QVector<QRgb> & palette )
    {
        m_img.setColorTable(palette);
        return QPixmap::fromImage(m_img, Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
    }

    void makeImageTableRow( QTableWidget * tbl, const QVector<QRgb> & pal, int rowid )
    {
        //0. Image Preview
        QLabel * preview = new QLabel("");

        tbl->setCellWidget(rowid, 0, preview);

        preview->setScaledContents(true);
        preview->setAlignment(Qt::AlignCenter);
        preview->setMinimumWidth(m_img.width());
        preview->setMinimumHeight(m_img.height());
        preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        if( tbl->horizontalHeader()->sectionSize(0 < m_img.width() ) )
        {
            tbl->setColumnWidth(0, m_img.width());
            tbl->horizontalHeader()->resizeSection(0, m_img.width());
        }
        if( tbl->verticalHeader()->sectionSize(rowid) < m_img.height() )
        {
            tbl->setRowHeight(rowid, m_img.height());
            tbl->verticalHeader()->resizeSection(rowid, m_img.height());
        }

        preview->setPixmap(makePixmap(pal).scaled(preview->width(), preview->height(), Qt::AspectRatioMode::KeepAspectRatio));

        //1. BPP
        QTableWidgetItem * bpp = new QTableWidgetItem( QString("%1").arg(m_depth) );
        bpp->setFlags( bpp->flags() & (~Qt::ItemFlag::ItemIsEditable) );
        tbl->setItem(rowid, 1, bpp);

        //2. Resolution
        QTableWidgetItem * resolution = new QTableWidgetItem( QString("%1x%2").arg(m_img.width()).arg(m_img.height()) );
        resolution->setFlags( resolution->flags() & (~Qt::ItemFlag::ItemIsEditable) );
        tbl->setItem(rowid, 2, resolution);
    }

    Sprite       * parentSprite();
    const Sprite * parentSprite()const {return const_cast<Image*>(this)->parentSprite();}

    int nbimgcolumns()const
    {
        return 3;
    }

    //Those can be re-implemented!
    QVariant imgData(int column, int role);

private:
//    int         m_width;
//    int         m_height;
    QImage              m_img;
    fmt::ImageDB::img_t m_raw; //Need this because QImage doesn't own the buffer...
    int                 m_depth;    //Original image depth in bpp
};

//*******************************************************************
//
//*******************************************************************
class ImageContainer : public BaseListContainerChild<&ElemName_Images, Image>/*, public QAbstractItemModel*/
{

public:
    class ImagesManager : public QAbstractItemModel
    {
        ImageContainer * m_parentcnt;
        // QAbstractItemModel interface
    public:
        ImagesManager(ImageContainer * parent)
            :QAbstractItemModel(), m_parentcnt(parent)
        {}

        QModelIndex index(int row, int column, const QModelIndex &parent) const override
        {
            TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
            TreeElement *childItem  = parentItem->child(row);
            if (childItem)
                return createIndex(row, column, childItem);
            else
                return QModelIndex();
        }
        QModelIndex parent(const QModelIndex &child) const override
        {
            TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
            TreeElement *parentItem = childItem->parent();
            assert(parentItem != nullptr);

            if (parentItem == m_parentcnt)
                return QModelIndex();

            return createIndex(parentItem->childNumber(), 0, parentItem);
        }
        int rowCount(const QModelIndex &parent) const override
        {
            TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
            return parentItem->childCount();
        }
        int columnCount(const QModelIndex &parent) const override
        {
            if (parent.isValid())
                return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
            else
                return 3;
        }
        bool hasChildren(const QModelIndex &parent) const override
        {
            TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
            if(parentItem)
                return parentItem->childCount() > 0;
            else
                return false;
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

            Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
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
        bool insertRows(int row, int count, const QModelIndex &parent) override
        {
            TreeElement *parentItem = getItem(parent);
            bool success;

            beginInsertRows(parent, row, row + count - 1);
            success = parentItem->insertChildren(row, count);
            endInsertRows();

            return success;
        }
        bool removeRows(int row, int count, const QModelIndex &parent) override
        {
            TreeElement *parentItem = getItem(parent);
            bool success = true;

            beginRemoveRows(parent, row, row + count - 1);
            success = parentItem->removeChildren(row, count);
            endRemoveRows();

            return success;
        }
        bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override
        {
            assert(false);
            return false;
        }

        TreeElement *getItem(const QModelIndex &index)
        {
            if (index.isValid())
            {
                TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
                if (item)
                    return item;
            }
            return m_parentcnt;
        }
    };

public:
    ImageContainer( TreeElement * parent)
        :BaseListContainerChild(parent),m_manager(new ImagesManager(this))
    {setDataTy(eTreeElemDataType::images);}

    ImageContainer( const ImageContainer & cp)
        :BaseListContainerChild(cp),m_manager(new ImagesManager(this))
    {}

    ImageContainer( ImageContainer && mv)
        :BaseListContainerChild(mv),m_manager(new ImagesManager(this))
    {}

    ImageContainer & operator=( const ImageContainer & cp )
    {
        BaseListContainerChild::operator=(cp);
        m_manager.reset(new ImagesManager(this));
        return *this;
    }

    ImageContainer & operator=( ImageContainer && mv )
    {
        BaseListContainerChild::operator=(mv);
        m_manager.reset(new ImagesManager(this));
        return *this;
    }

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

    void importImages8bpp(const fmt::ImageDB::imgtbl_t & imgs, const fmt::ImageDB::frmtbl_t & frms)
    {
        assert(false);
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

    void fillImgListTable(QTableWidget * tbl, const QVector<QRgb> & pal)
    {
        tbl->setUpdatesEnabled(false);
        tbl->clearContents();
        tbl->setRowCount(childCount());
        //tbl->setColumnCount(3);

        for( size_t cnti = 0; cnti < childCount(); ++cnti )
        {
            Image * pimg = static_cast<Image*>(child(cnti));
            if(pimg)
                pimg->makeImageTableRow(tbl, pal, cnti);
            else
                assert(false);
        }
        tbl->setUpdatesEnabled(true);
    }

    Sprite * parentSprite();


    ImagesManager * getModel(){return m_manager.data();}

private:
    QScopedPointer<ImagesManager> m_manager;

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
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
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

private:

};

//*******************************************************************
//
//*******************************************************************
class AnimSequence : public BaseTreeTerminalChild<&ElemName_AnimSequence>
{
public:
    AnimSequence( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    inline bool operator==( const AnimSequence & other)const  {return this == &other;}
    inline bool operator!=( const AnimSequence & other)const  {return !operator==(other);}

//    inline fmt::AnimDB::animseqid_t getID()const {return m_id;}
//    inline void setID(fmt::AnimDB::animseqid_t id){m_id = id;}

    void importSeq( const fmt::AnimDB::animseq_t & seq/*, fmt::AnimDB::animseqid_t id*/ )
    {
        //m_id = id;
        m_seq = seq;
    }

    fmt::AnimDB::animseq_t exportSeq()const
    {
        return m_seq;
    }

    inline int getSeqLength()const {return m_seq.size();}
    Sprite * parentSprite();

private:
    //fmt::AnimDB::animseqid_t m_id;
    fmt::AnimDB::animseq_t   m_seq;
};

//*******************************************************************
//
// Not an animation group!!!! This is just a list of animation sequences!
//*******************************************************************
class AnimSequences : public BaseListContainerChild<&ElemName_AnimSequences, AnimSequence>
{
public:

    AnimSequences( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

    AnimSequence * getSequenceByID( fmt::AnimDB::animseqid_t id )
    {
//        for( size_t cntchild = 0; cntchild < childCount(); ++cntchild )
//        {
//            AnimSequence * pchild = static_cast<AnimSequence*>(child(cntchild));
//            if( pchild && pchild->getID() == id )
//                 return pchild;
//        }
//        return nullptr;
        return static_cast<AnimSequence*>(child(id));
    }

    void removeSequence( fmt::AnimDB::animseqid_t id )
    {
//        for( size_t cntchild = 0; cntchild < childCount(); ++cntchild )
//        {
//            AnimSequence * pchild = static_cast<AnimSequence*>(child(cntchild));
//            if( pchild && pchild->getID() == id )
//                 removeChildren(cntchild,1);
//        }
        removeChildren(id,1);
    }


    void importSequences( const fmt::AnimDB::animseqtbl_t & src )
    {
        removeChildren(0, childCount());
        insertChildren(0, src.size());

        for( fmt::AnimDB::animseqid_t cntid = 0; cntid < src.size(); ++cntid )
            m_container[cntid].importSeq(src.at(cntid));
    }

    fmt::AnimDB::animseqtbl_t exportSequences()
    {
        fmt::AnimDB::animseqtbl_t seqs(childCount());
        for( int cntid = 0; cntid < childCount(); ++cntid )
        {
            seqs[cntid] = std::move(m_container[cntid].exportSeq());
        }
        return std::move(seqs);
    }

    Sprite * parentSprite();

private:
};

//*******************************************************************
//
//*******************************************************************
class AnimGroup : public BaseTreeTerminalChild<&ElemName_AnimGroup>
{
public:
    AnimGroup( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {setDataTy(eTreeElemDataType::animGroup);}

    int columnCount()const override
    {
        return 1;
    }

    QVariant data(int column) const override
    {
        if( column == 0 )
            return QVariant( QString("%1 %2").arg(ElemName()).arg(childNumber()) );
        else if(column == 1)
            return QVariant(QString("%1").arg(unk16));
        else if(column == 2)
            return QVariant(QString("%1").arg(m_seqlist.size()));
        else
            return QVariant();
    }

    void importGroup(const fmt::AnimDB::animgrp_t & grp/*, fmt::AnimDB::animgrpid_t id*/)
    {
//        m_id = id;
        m_seqlist.reserve(grp.seqs.size());
        for( const auto & seq : grp.seqs )
            m_seqlist.push_back(seq);

        unk16 = grp.unk16;
    }

    fmt::AnimDB::animgrp_t exportGroup()
    {
        fmt::AnimDB::animgrp_t dest;
        dest.seqs.resize(m_seqlist.size());
        std::copy(m_seqlist.begin(), m_seqlist.end(), dest.seqs.begin());
        dest.unk16 = unk16;
        return std::move(dest);
    }

//    inline fmt::AnimDB::animgrpid_t getGrpId()const {return m_id;}

    inline bool operator==( const AnimGroup & other)const  {return this == &other;}
    inline bool operator!=( const AnimGroup & other)const  {return !operator==(other);}

    void fillTableWidget( QTableWidget * tbl, AnimSequences & seqs )
    {
        tbl->setRowCount(m_seqlist.size());

        int cntrow = 0;
        for( auto seq : m_seqlist )
        {
            tbl->setItem(cntrow, 0, new QTableWidgetItem(QString("Sequence ID ").arg(seq)) );
            AnimSequence * pseq = static_cast<AnimSequence*>(seqs.child(seq));
            if( pseq )
                tbl->setItem(cntrow, 1, new QTableWidgetItem(QString("%1").arg(pseq->getSeqLength())) );
            ++cntrow;
        }
    }


    void removeSequenceReferences( fmt::AnimDB::animseqid_t id )
    {
        for( auto & seq : m_seqlist )
        {
            if(seq == id)
                seq = -1;
        }
    }

    Sprite * parentSprite();

private:
//    fmt::AnimDB::animgrpid_t        m_id;
    QList<fmt::AnimDB::animseqid_t> m_seqlist;
    uint16_t                        unk16;
};

//*******************************************************************
//
//*******************************************************************
class AnimTable : public BaseListContainerChild<&ElemName_AnimTable, AnimGroup>
{
public:

    AnimTable( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

    //Load the animation table
    void importAnimationTable( const fmt::AnimDB::animtbl_t & orig )
    {
        for(auto id : orig)
            m_animtbl.push_back(id);
    }

    fmt::AnimDB::animtbl_t exportAnimationTable()
    {
        fmt::AnimDB::animtbl_t dest;
        dest.reserve(m_animtbl.size());
        for( auto id : m_animtbl )
            dest.push_back(id);
        return std::move(dest);
    }

    void importAnimationGroups( fmt::AnimDB::animgrptbl_t & animgrps )
    {
        m_container.reserve(animgrps.size());
        removeChildren(0, childCount());
        insertChildren(0, animgrps.size());

        for( fmt::AnimDB::animgrpid_t cntgrp = 0; cntgrp < animgrps.size(); ++cntgrp )
            m_container[cntgrp].importGroup(animgrps[cntgrp]);
    }

    fmt::AnimDB::animgrptbl_t exportAnimationGroups()
    {
        fmt::AnimDB::animgrptbl_t grps(childCount());
        for( int cntgrp = 0; cntgrp < childCount(); ++cntgrp )
        {
            grps[cntgrp] = std::move(m_container[cntgrp].exportGroup());
        }
        return std::move(grps);
    }

    //Clears any references to a group from the animation table!
    void DeleteGroupRefs( fmt::AnimDB::animgrpid_t id )
    {
        int idx = -1;
        do
        {
            idx = m_animtbl.indexOf(id);
            if( idx != -1 )
                m_animtbl[idx] = -1;
        }while( idx != -1 );
    }

    void DeleteGroupChild( fmt::AnimDB::animgrpid_t id )
    {
        DeleteGroupRefs(id);
//        for( size_t cntchild = 0; cntchild < childCount(); ++cntchild )
//        {
//            AnimGroup   * pchild = static_cast<AnimGroup*>(child(cntchild));
//            if( pchild && pchild->getGrpId() == id )
                 removeChildren(id,1);
//        }
    }

    inline int getAnimTableSize()const {return m_animtbl.size();}
    inline fmt::AnimDB::animgrpid_t & getAnimTableEntry(int entry) {return m_animtbl[entry];}

    Sprite * parentSprite();

private:
    QList<fmt::AnimDB::animgrpid_t> m_animtbl;
};

//============================================================================================
//
//============================================================================================

//*******************************************************************
//
//*******************************************************************
class Sprite : public TreeElement, public utils::BaseSequentialIDGen<Sprite>
{
public:
    Sprite( TreeElement * parent )
        :TreeElement(parent),
          BaseSequentialIDGen(),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false)
    {
        //AddSprite(this);
        InitElemTypes();
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
    }

    Sprite( TreeElement * parent, QByteArray && raw )
        :TreeElement(parent),
          BaseSequentialIDGen(),
          m_raw(raw),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false)
    {
        //AddSprite(this);
        InitElemTypes();
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
    }

    Sprite( const Sprite & cp )
        :TreeElement(cp),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false)
    {
        operator=(cp);
    }

    Sprite & operator=(const Sprite & cp)
    {
        //
        m_sprhndl= cp.m_sprhndl;
        m_efxcnt = cp.m_efxcnt;
        m_palcnt = cp.m_palcnt;
        m_imgcnt = cp.m_imgcnt;
        m_frmcnt = cp.m_frmcnt;
        m_seqcnt = cp.m_seqcnt;
        m_anmtbl = cp.m_anmtbl;
        m_bparsed = cp.m_bparsed;
        //Update the pointer to our instance
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
        //
        m_raw = cp.m_raw;
        InitElemTypes();
        return *this;
    }

    Sprite( Sprite && mv )
        :TreeElement(mv),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false)
    {
        operator=(mv);
    }

    Sprite & operator=(Sprite && mv)
    {
        //
        m_efxcnt = std::move(mv.m_efxcnt);
        m_palcnt = std::move(mv.m_palcnt);
        m_imgcnt = std::move(mv.m_imgcnt);
        m_frmcnt = std::move(mv.m_frmcnt);
        m_seqcnt = std::move(mv.m_seqcnt);
        m_anmtbl = std::move(mv.m_anmtbl);
        m_bparsed = mv.m_bparsed;
        //Update the pointer to our instance
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
        //
        m_raw = std::move(mv.m_raw);
        InitElemTypes();
        return *this;
    }

    ~Sprite()
    {
    }

    void InitElemTypes()
    {
        setDataTy(eTreeElemDataType::sprite);

        m_efxcnt.setElemTy(eTreeElemType::Fixed);
        m_efxcnt.setDataTy(eTreeElemDataType::effectOffsets);

        m_palcnt.setElemTy(eTreeElemType::Fixed);
        m_palcnt.setDataTy(eTreeElemDataType::palette);

        m_imgcnt.setElemTy(eTreeElemType::Fixed);
        m_imgcnt.setDataTy(eTreeElemDataType::images);

        m_frmcnt.setElemTy(eTreeElemType::Fixed);
        m_frmcnt.setDataTy(eTreeElemDataType::frames);

        m_seqcnt.setElemTy(eTreeElemType::Fixed);
        m_seqcnt.setDataTy(eTreeElemDataType::animSequences);

        m_anmtbl.setElemTy(eTreeElemType::Fixed);
        m_anmtbl.setDataTy(eTreeElemDataType::animTable);
    }




public:

    TreeElement *child(int row) override
    {
        return ElemPtr(row);
    }

    int childCount() const override
    {
        return NBChilds;
    }

    int childNumber() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOf(const_cast<Sprite*>(this));

        return -1;
    }

    int indexOf( TreeElement * ptr )const override
    {
        //Search a matching child in the list!
        for( int idx = 0; idx < NBChilds; ++idx )
        {
            if(ElemPtr(idx) == ptr)
                return idx;
        }
        return -1;
    }

    int columnCount() const override
    {
        return 1; //Always just 1 column
    }

    TreeElement *parent() override
    {
        return m_parentItem;
    }

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        QString sprname = QString("Sprite#%1").arg(childNumber());
        return QVariant(sprname);
    }

    void OnClicked() override
    {
        if( m_raw.size() != 0 && !m_bparsed )
            ParseSpriteData();
    }

    void OnExpanded() override
    {
        if( m_raw.size() != 0 && !m_bparsed )
            ParseSpriteData();
    }

    /**/
    void ParseSpriteData()
    {
        m_sprhndl.Parse( m_raw.begin(), m_raw.end() );
        m_anmtbl.importAnimationTable(m_sprhndl.getAnimationTable());
        m_anmtbl.importAnimationGroups( m_sprhndl.getAnimGroups() );

        m_palcnt.m_pal = std::move(utils::ConvertSpritePalette(m_sprhndl.getPalette())); //conver the palette once, so we don't do it constantly

        m_seqcnt.importSequences( m_sprhndl.getAnimSeqs());
        m_frmcnt.importFrames(m_sprhndl.getFrames());

        if( m_sprhndl.getImageFmtInfo().is256Colors() )
            m_imgcnt.importImages8bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());
        else
            m_imgcnt.importImages4bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());


        m_bparsed = true;
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

    inline bool operator==( const Sprite & other)const  {return getID() == other.getID();}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}


    QPixmap & MakePreviewPalette()
    {
        m_previewPal = utils::PaintPaletteToPixmap(getPalette()); // utils::ConvertSpritePalette(m_sprhndl.getPalette()) );
        return m_previewPal;
    }

    QPixmap & MakePreviewFrame()
    {
        if(m_bparsed)
        {
            return m_previewImg = std::move(AssembleFrame(0));
        }
        return m_previewImg;
    }


    QPixmap AssembleFrame(size_t frameid)
    {
        if(!m_bparsed || (frameid >= m_sprhndl.getFrames().size()))
            return QPixmap();

        QPixmap  resultimg(256,512);
        QPainter qpaint(&resultimg);
        QTransform deftrans = qpaint.transform();


        //#1 - Grab all our images and assemble them!
        size_t lowestX = 256;
        size_t lowestY = 512;
        size_t highestX = 0;
        size_t highestY = 0;
        int    lastimage = -1;
        size_t cntstep = 0;
        for( const fmt::step_t & step : m_sprhndl.getFrame(frameid))
        {
            qpaint.setTransform(deftrans);
            auto imgres = step.GetResolution();
            size_t offsetx = step.getXOffset()-128;
            size_t offsety = step.getYOffset();

            if( offsetx < lowestX )
                lowestX = offsetx;
            if( offsety < lowestY )
                lowestY = offsety;

            if( (offsetx + imgres.first) > highestX )
                highestX = offsetx + imgres.first;
            if( (offsety + imgres.second) > highestY )
                highestY = offsety + imgres.second;

            if( step.frmidx != 0xFFFF || (step.frmidx == 0xFFFF && lastimage != -1) )
            {
                int imgidx = (step.frmidx != 0xFFFF)? step.frmidx : lastimage;

                lastimage = step.frmidx;
                QPixmap curpixmap;
                const std::vector<uint8_t> & curimg = m_sprhndl.getImage(imgidx);
                if( step.isColorPal256() )
                {
                    curpixmap = std::move( utils::UntileIntoImg( step.GetResolution().first,
                                                                 step.GetResolution().second,
                                                                 QByteArray::fromRawData( (char *)curimg.data(), curimg.size() ),
                                                                 getPalette() ) );
                }
                else
                {
                    //Turn 4bb pixels into 8bpp pixels
                    QByteArray expanded(utils::Expand4BppTo8Bpp( QByteArray::fromRawData((char *)curimg.data(), curimg.size()) ));
                    curpixmap = std::move(utils::UntileIntoImg( step.GetResolution().first,
                                                                step.GetResolution().second,
                                                                expanded,
                                                                getPalette() ) );

                    //imgstrips.push_back(  QImage( m_sprhndl.m_images.m_images[step.frmidx].data(), step.GetResolution().first, step.GetResolution().second, QImage::Format_Indexed8) );
                }

                //Transform
                if(step.isHFlip())
                    curpixmap = std::move(curpixmap.transformed( QTransform().scale(-1, 1)));
                if(step.isVFlip())
                    curpixmap = std::move(curpixmap.transformed( QTransform().scale(1, -1)));

                qpaint.drawPixmap( offsetx, offsety, imgres.first, imgres.second, curpixmap);

                //DEBUG!!!
                curpixmap.save(QString("./step%1.png").arg(cntstep),"png");
                resultimg.save(QString("./step%1_res.png").arg(cntstep),"png");
            }
            ++cntstep;
        }
        //m_sprhndl.m_images.m_images;
        //m_sprhndl.m_images.m_frames;
        //m_sprhndl.m_images.m_pal;

        //Crop
        return std::move(resultimg.copy( lowestX, lowestY, (highestX - lowestX), (highestY - lowestY) ));
    }


    static Sprite * ParentSprite( TreeElement * parentspr ) {return static_cast<Sprite*>(parentspr); }

    const QVector<QRgb> & getPalette()const { return m_palcnt.m_pal; }
    QVector<QRgb>       & getPalette() { return m_palcnt.m_pal; }

private:

    TreeElement * ElemPtr( int idx )
    {
        switch(idx)
        {
        case 0:
            return &m_efxcnt;
        case 1:
            return &m_palcnt;
        case 2:
            return &m_imgcnt;
        case 3:
            return &m_frmcnt;
        case 4:
            return &m_seqcnt;
        case 5:
            return &m_anmtbl;
        };
        assert(false);
        return nullptr;
    }

    const TreeElement * ElemPtr( int idx )const
    {
        return const_cast<Sprite*>(this)->ElemPtr(idx);
    }

    static const int        NBChilds = 6;
    EffectOffsetContainer   m_efxcnt;
    PaletteContainer        m_palcnt;
    ImageContainer          m_imgcnt;
    FramesContainer         m_frmcnt;
    AnimSequences           m_seqcnt;
    AnimTable               m_anmtbl;

    bool                    m_bparsed;
public:
    bool wasParsed()const
    {
        return m_bparsed;
    }

    //Raw data buffer
    QByteArray              m_raw;
    QPixmap                 m_previewImg;
    QPixmap                 m_previewPal;
    fmt::WA_SpriteHandler   m_sprhndl;
    //QVector<QRgb>           m_palette;
};

#endif // SPRITE_H
