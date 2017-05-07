#ifndef SPRITE_H
#define SPRITE_H
#include <QByteArray>
#include <QStack>
#include <QHash>
#include <QPixmap>
#include <QRgb>
#include <QImage>
#include <QVector>
#include <cassert>
#include <cstdint>
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

//============================================================================================
//
//============================================================================================
/*
 * BaseTreeTerminalChild
 *  Base class for implementing terminal tree nodes elements!
*/
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
        QString sprname = ElemName() + "#" + QString(childNumber());
        return QVariant(sprname);
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

};


/*
 * BaseListContainerChild
*/
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

    public:
        BaseListContainerChild(TreeElement * parent)
            :TreeElement(parent)
        {}

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
            QString sprname = ElemName() + "#" + QString(childNumber());
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

//
//
//
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
};

//
//
//
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
};

//
//
//

class Image : public BaseTreeTerminalChild<&ElemName_Image>, public utils::BaseSequentialIDGen<Image>
{
public:
    Image(TreeElement * parent)
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {
    }

    inline bool operator==( const Image & other)const  {return getID() == other.getID();}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

};

class ImageContainer : public BaseListContainerChild<&ElemName_Images, Image>
{
public:

    ImageContainer( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

};

//
//
//
class MFrame : public BaseTreeTerminalChild<&ElemName_Frame>, public utils::BaseSequentialIDGen<MFrame>
{
public:
    MFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {}

    inline bool operator==( const MFrame & other)const  {return getID() == other.getID();}
    inline bool operator!=( const MFrame & other)const  {return !operator==(other);}

private:
};


//
//
//

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

private:
};

//
//
//

class AnimSequence : public BaseTreeTerminalChild<&ElemName_AnimSequence>, public utils::BaseSequentialIDGen<AnimSequence>
{
public:
    AnimSequence( TreeElement * parent )
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {}

    inline bool operator==( const AnimSequence & other)const  {return getID() == other.getID();}
    inline bool operator!=( const AnimSequence & other)const  {return !operator==(other);}
};

//
//
//
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

private:
};

//
//
//
class AnimTable : public BaseTreeTerminalChild<&ElemName_AnimTable>
{
public:

    AnimTable( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }
};

//============================================================================================
//
//============================================================================================

//
//
//
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

    /**/
    void ParseSpriteData()
    {
        m_sprhndl.Parse( m_raw.begin(), m_raw.end() );
        m_bparsed = true;
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

    inline bool operator==( const Sprite & other)const  {return getID() == other.getID();}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}


    QPixmap & MakePreviewPalette()
    {
        m_previewPal = utils::PaintPaletteToPixmap( utils::ConvertSpritePalette(m_sprhndl.m_images.m_pal.colors) );
        return m_previewPal;
    }

    QPixmap & MakePreviewFrame()
    {
        if(m_bparsed)
        {
//            QByteArray indexed8;
//            for(auto pixpair : m_sprhndl.m_images.m_images.front())
//            {
//                indexed8.push_back((pixpair >> 4) & 0x0F );
//                indexed8.push_back(pixpair & 0x0F);
//            }
//            m_previewImg = QImage( (unsigned char *) indexed8.data(), 32, 32, QImage::Format_Indexed8);
//            QVector<QRgb> colortbl;

//            for( size_t cntcol = 0; cntcol < m_sprhndl.m_images.m_pal.colors.size(); ++cntcol )
//            {
//                uint32_t colval = m_sprhndl.m_images.m_pal.colors[cntcol];
//                QColor tmpcol;
//                tmpcol.setRed( (colval >> 24) & 0xFF );
//                tmpcol.setGreen( (colval >> 16) & 0xFF );
//                tmpcol.setBlue( (colval >> 8) & 0xFF );
//                tmpcol.setAlpha(255);
//                colortbl.push_back(tmpcol.rgba()); //shift by 8 right to align with the format QT uses
//            }
//            m_previewImg.setColorTable(colortbl);

            return m_previewImg = std::move(AssembleFrame(0));
        }
        return m_previewImg;
    }


    QPixmap AssembleFrame(size_t frameid)
    {
        if(!m_bparsed || (frameid >= m_sprhndl.m_images.m_frames.size()))
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
        for( const fmt::ImageDB::step_t & step : m_sprhndl.m_images.m_frames[frameid] )
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
                const std::vector<uint8_t> & curimg = m_sprhndl.m_images.m_images[imgidx];
                if( step.isColorPal256() )
                {
                    curpixmap = std::move( utils::UntileIntoImg( step.GetResolution().first,
                                                                 step.GetResolution().second,
                                                                 QByteArray::fromRawData( (char *)curimg.data(), curimg.size() ),
                                                                 utils::ConvertSpritePalette(m_sprhndl.m_images.m_pal.colors) ) );
                }
                else
                {
                    //Turn 4bb pixels into 8bpp pixels
                    QByteArray expanded(utils::Expand4BppTo8Bpp( QByteArray::fromRawData((char *)curimg.data(), curimg.size()) ));
                    curpixmap = std::move(utils::UntileIntoImg( step.GetResolution().first,
                                                                step.GetResolution().second,
                                                                expanded,
                                                                utils::ConvertSpritePalette(m_sprhndl.m_images.m_pal.colors) ) );

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
};

#endif // SPRITE_H
