#ifndef SPRITE_H
#define SPRITE_H
#include <QObject>
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
#include <src/ppmdu/fmts/compression_handler.hpp>

class Sprite;

/*
*/
class SpritePropertiesHandler : public QObject
{
    Q_OBJECT
    Sprite * m_powner;
public:
    SpritePropertiesHandler( Sprite * owner, QObject * parent = nullptr );
    ~SpritePropertiesHandler();

    void sendSpriteLoaded();

    void setOwner( Sprite * own );

public slots:
    void setSpriteType( fmt::eSpriteType ty );

signals:
    void spriteLoaded();
};



//*******************************************************************
//Sprite
//*******************************************************************
class Sprite : public TreeElement, public utils::BaseSequentialIDGen<Sprite>
{
public:
    friend class SpritePropertiesHandler;


public:
    Sprite( TreeElement * parent )
        :TreeElement(parent),
          BaseSequentialIDGen(),
          m_prophndlr(this),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false),
          m_targetgompression(filetypes::eCompressionFormats::INVALID)
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
          m_prophndlr(this),
          m_raw(raw),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false),
          m_targetgompression(filetypes::eCompressionFormats::INVALID)
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
          m_prophndlr(this),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this),
          m_bparsed(false),
          m_targetgompression(filetypes::eCompressionFormats::INVALID)
    {
        operator=(cp);
    }

    Sprite & operator=(const Sprite & cp)
    {
        m_prophndlr.setOwner(this);
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
          m_prophndlr(this),
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
        m_prophndlr.setOwner(this);
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


    void FillSpriteProperties()
    {

    }

    SpritePropertiesHandler & getEventHandler()
    {
        return m_prophndlr;
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
        if(IsRawDataCompressed(&m_targetgompression))
            DecompressRawData();

        m_sprhndl.Parse( m_raw.begin(), m_raw.end() );
        m_anmtbl.importAnimationTable(m_sprhndl.getAnimationTable());
        m_anmtbl.importAnimationGroups( m_sprhndl.getAnimGroups() );

        m_palcnt.m_pal = std::move(utils::ConvertSpritePalette(m_sprhndl.getPalette())); //convert the palette once, so we don't do it constantly

        m_seqcnt.importSequences( m_sprhndl.getAnimSeqs());
        m_frmcnt.importFrames(m_sprhndl.getFrames());

        if( m_sprhndl.getImageFmtInfo().is256Colors() )
            m_imgcnt.importImages8bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());
        else
            m_imgcnt.importImages4bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());


        m_bparsed = true;
    }

    /*
     * Turns the data from the data structure to its native format into the m_raw container!
    */
    void CommitSpriteData()
    {
        QByteArray buffer;
        auto       itback = std::back_inserter(buffer);

        //First convert the data from the UI
        m_sprhndl.getAnimationTable()   = m_anmtbl.exportAnimationTable();
        m_sprhndl.getAnimGroups()       = m_anmtbl.exportAnimationGroups();
        m_sprhndl.getPalette()          = utils::ConvertSpritePaletteFromQt(m_palcnt.m_pal);
        m_sprhndl.getAnimSeqs()         = m_seqcnt.exportSequences();
        m_sprhndl.getFrames()           = m_frmcnt.exportFrames();

        if( m_sprhndl.getImageFmtInfo().is256Colors() ) //#FIXME : FIgure out something better!!!!
            m_sprhndl.getImages() = m_imgcnt.exportImages8bpp();
        else
            m_sprhndl.getImages() = m_imgcnt.exportImages4bpp();

        //Write the data
        itback = m_sprhndl.Write(itback);
        m_raw  = qMove(buffer);

        //Compress if needed at the end!
        if(m_targetgompression != filetypes::eCompressionFormats::INVALID)
            CompressRawData(m_targetgompression);
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
            return m_previewImg = std::move(QPixmap::fromImage(m_frmcnt.getFrame(0)->AssembleFrame(0,0)) );
        }
        return m_previewImg;
    }

    void setTargetCompression(filetypes::eCompressionFormats fmt)
    {
        m_targetgompression = fmt;
    }
    inline filetypes::eCompressionFormats getTargetCompression()const
    {
        return m_targetgompression;
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

    inline const QByteArray & getRawData()const
    {
        return m_raw;
    }

private:

    inline bool IsRawDataCompressed(filetypes::eCompressionFormats * outfmt = nullptr)const
    {
        filetypes::eCompressionFormats fmt = filetypes::IndentifyCompression( m_raw.begin(), m_raw.end() );
        if(outfmt)
            (*outfmt) = fmt;
        return fmt < filetypes::eCompressionFormats::INVALID;
    }

    void DecompressRawData()
    {
        QByteArray buffer;
        auto                 itback = std::back_inserter(buffer);
        filetypes::Decompress(m_raw.begin(), m_raw.end(), itback);
        m_raw = std::move(buffer);
    }

    void CompressRawData(filetypes::eCompressionFormats cpfmt)
    {
        QByteArray buffer;
        auto                 itback = std::back_inserter(buffer);
        filetypes::Compress( cpfmt, m_raw.begin(), m_raw.end(), itback);
        m_raw = std::move(buffer);
    }

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
    filetypes::eCompressionFormats m_targetgompression;


    SpritePropertiesHandler m_prophndlr;

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
