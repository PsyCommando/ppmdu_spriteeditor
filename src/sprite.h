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
#include <QDebug>
#include <QGraphicsScene>
#include <cstdint>
#include <list>
#include <src/treeelem.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/sprite_img.hpp>
#include <src/sprite_anim.hpp>





//============================================================================================
//
//============================================================================================

class Sprite;




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

    QVariant data(int column, int role) const override
    {
        if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(QString("Sprite#%1").arg(childNumber()));
        return QVariant();
    }

//    QVariant headerData(int section, bool bhorizontal, int role) const override
//    {
//        if(bhorizontal)
//        {
//            if(section == 0  && (role == Qt::DisplayRole || role == Qt::EditRole))
//                return qMove(QVariant(QString("Sprite#%1").arg(childNumber())));
//        }
//        else
//        {
//            return qMove(QVariant(QString("%1").arg(section)));
//        }
//        return QVariant();
//    }


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

            if( step.frmidx != -1 || (step.frmidx == -1 && lastimage != -1) )
            {
                fmt::frmid_t imgidx = (step.frmidx != -1)? step.frmidx : lastimage;
                if(step.frmidx != -1)
                    lastimage = step.frmidx;

                if(imgidx == -1)
                    qDebug("Sprite::AssembleFrame():Got a fully -1 frame sequence!!!!\n");


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
                //curpixmap.save(QString("./step%1.png").arg(cntstep),"png");
                //resultimg.save(QString("./step%1_res.png").arg(cntstep),"png");
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

    inline AnimSequences        & getAnimSequences()        {return m_seqcnt;}
    inline const AnimSequences  & getAnimSequences()const   {return m_seqcnt;}

    inline AnimSequence         * getAnimSequence(fmt::AnimDB::animseqid_t id)        {return m_seqcnt.getSequenceByID(id);}
    //inline const AnimSequences  * getAnimSequence(fmt::AnimDB::animseqid_t id)const   {return m_seqcnt.getSequenceByID(id);}

    inline MFrame * getFrame( fmt::frmid_t id ) { return m_frmcnt.getFrame(id); }
    inline const MFrame * getFrame( fmt::frmid_t id )const { return m_frmcnt.getFrame(id); }

    inline Image * getImage(fmt::frmid_t idx)
    {
        if( idx >= 0 && idx < m_imgcnt.childCount() )
            return m_imgcnt.getImage(idx);
        else
            return nullptr;
    }
    inline const Image * getImage(fmt::frmid_t idx)const
    {
        return const_cast<Sprite*>(this)->m_imgcnt.getImage(idx);
    }

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
        Q_ASSERT(false);
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
