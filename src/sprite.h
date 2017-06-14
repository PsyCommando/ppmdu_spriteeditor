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
#include <QPointer>

#include <cstdint>
#include <list>
#include <functional>

#include <src/treeelem.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/utils/imgutils.hpp>

//Backend
#include <src/ppmdu/fmts/compression_handler.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

//Other parts of the sprite
#include <src/sprite_img.hpp>
#include <src/sprite_frames.hpp>
#include <src/sprite_palette.hpp>
#include <src/sprite_anim.hpp>

extern const char * ElemName_SpriteProperty;
enum struct eSpritePropColumns : int
{
    Value= 0,
    Description,
    NbColumns,
};
extern const int         SpritePropertiesNbCols;
extern const QStringList SpritePropertiesColNames;

enum struct eSpriteProperties : int
{
    SpriteType = 0,
    Compression,

    ColorMode,
    Unk6,
    Unk7,
    Unk8,
    Unk9,
    Unk10,
    Unk11,
    Unk12,
    Unk13,

    NbProperties,
};
extern const QStringList SpritePropertiesNames;
extern const QStringList SpritePropertiesDescriptions;
extern const int         SpriteNbProperties;

//Since we have our own compression settings that differs
// from those in the compression_handler.hpp file, put them here!
enum struct eCompressionFmtOptions : int
{
    PKDPX = 0,
    AT4PX,
    NONE,
};
extern const QStringList CompressionFmtOptions; //names for the above options
filetypes::eCompressionFormats  CompOptionToCompFmt( eCompressionFmtOptions opt );
eCompressionFmtOptions          CompFmtToCompOption( filetypes::eCompressionFormats fmt );


enum struct eSpriteColorModes : int
{
    _16Colors = 0,
    _256Colors,
    _Bitmap,
    INVALID,
};
extern const QStringList SpriteColorModes;

//Forward declare Sprite
class Sprite;

//*******************************************************************
//  SpriteOverviewModel
//*******************************************************************
class SpriteOverviewModel : public QAbstractItemModel
{
    Q_OBJECT
    typedef QPair<QString,std::function<QVariant(Sprite*)>> stats_t;
    static const QVector<stats_t>                           StatEntries;
    Sprite                                                  *m_spr;
public:
    SpriteOverviewModel(Sprite * spr);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int /*column*/, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};


//*******************************************************************
//  SpritePropertiesDelegate
//*******************************************************************
class SpritePropertiesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Sprite          *m_spr;
    static const int PropValueColLen = 128;
public:

    SpritePropertiesDelegate(Sprite * parentspr, QObject * parent = nullptr);
    virtual ~SpritePropertiesDelegate();

    // QAbstractItemDelegate interface
public:
    virtual QSize   sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void    setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void    setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void    updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override;
};

//*******************************************************************
//  SpritePropertiesModel
//*******************************************************************
class SpritePropertiesModel : public QAbstractItemModel
{
    Q_OBJECT
    Sprite * m_spr;
public:

    SpritePropertiesModel(Sprite * parentspr, QObject * parent = nullptr);
    virtual ~SpritePropertiesModel();

    int getNbProperties()const;

    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QModelIndex     index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex     parent(const QModelIndex &/*child*/) const override;

    virtual int     rowCount(const QModelIndex &parent) const override;
    virtual int     columnCount(const QModelIndex &/*parent*/) const override;
    virtual bool    hasChildren(const QModelIndex &parent) const override;

    virtual QVariant    data(const QModelIndex &index, int role) const override;
    virtual bool        setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant    headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QVariant dataDisplay(int propid, int column)const;
    QVariant getNameForProperty(int propid)const;
    QVariant getDataForProperty(int propid, int role)const;
    QVariant getDescForProperty(int propid)const;
    void     setDataForProperty( eSpriteProperties propid, const QVariant & data );

signals:
    void spriteTypeSet(fmt::eSpriteType);
};

//*******************************************************************
//  SpritePropertiesHandler
//      Links the Sprite's properties and its Model and Delegate!
//*******************************************************************
class SpritePropertiesHandler : public QObject
{
    Q_OBJECT
    Sprite * m_powner;
    QPointer<SpritePropertiesDelegate> m_pDelegate; //QPointer because the hierachy handles deleting those!
    QPointer<SpritePropertiesModel>    m_pModel;

public:
    SpritePropertiesHandler( Sprite * owner, QObject * parent = nullptr )
        :QObject(parent),
          m_powner(owner),
          m_pDelegate(new SpritePropertiesDelegate(m_powner, this)),
          m_pModel(new SpritePropertiesModel(m_powner, this))
    {}

    virtual ~SpritePropertiesHandler()
    {
        qDebug("SpritePropertiesHandler::~SpritePropertiesHandler()\n");
    }

    void setOwner( Sprite * own )
    {
        m_powner = own;
    }

    SpritePropertiesDelegate * delegate() {return m_pDelegate.data();}
    SpritePropertiesModel    * model()    {return m_pModel.data();}

public slots:
    void setSpriteType( fmt::eSpriteType ty )
    {
        //Convert Sprite Type as neccessary!
        qDebug("SpritePropertiesHandler::setSpriteType(): Sprite type changed to %d!", static_cast<int>(ty));
    }

signals:

};


//*******************************************************************
//  Sprite
//*******************************************************************
class Sprite : public TreeElement
{
    friend class SpritePropertiesHandler;
    void _ctor();
public:
    typedef std::vector<uint8_t> rawdat_t;

    Sprite( TreeElement * parent );
    Sprite( TreeElement * parent, rawdat_t && raw );
    Sprite( const Sprite & cp );
    Sprite( Sprite       && mv );
    Sprite & operator=(const Sprite & cp);
    Sprite & operator=(Sprite       && mv);
    ~Sprite();

    inline bool operator==( const Sprite & other)const  {return this == &other;}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}

public:
    TreeElement     *nodeChild(int row) override;
    int             nodeChildCount() const override;
    int             nodeIndex() const override;
    int             indexOfNode( TreeElement * ptr )const override;
    int             nodeColumnCount() const override;
    TreeElement     *parentNode() override;
    QVariant        nodeData(int column, int role) const override;
    virtual Sprite  *parentSprite()override;

    //Don't allow that, since it would change the sprite type..
    inline bool insertChildrenNodes(int, int)override {return false;}
    inline bool removeChildrenNodes(int, int)override {return false;}

public:
    void OnClicked() override;
    void OnExpanded() override;

public:
    //Parse the data from the internal raw container
    void ParseSpriteData();
    //Turns the data from the data structure to its native format into the m_raw container!
    void CommitSpriteData();

    inline bool wasParsed()const    {return m_bparsed;}
    inline bool hasImageData()const {return m_imgcnt.hasChildren();}
    inline bool hasEfxOffsets()const{return type() == fmt::eSpriteType::Character;}
    inline bool hasAnimGrps()const  {return type() == fmt::eSpriteType::Character;}

    QPixmap & MakePreviewPalette();

    QPixmap & MakePreviewFrame(bool transparency = false);
    static Sprite * ParentSprite( TreeElement * parentspr );

    const QVector<QRgb> & getPalette()const     { return m_palcnt.getPalette(); }
    QVector<QRgb>       & getPalette()          { return m_palcnt.getPalette(); }
    const PaletteModel * getPaletteModel()const { return m_palcnt.getModel(); }
    PaletteModel       * getPaletteModel()      { return m_palcnt.getModel(); }

    inline void setPalette(const QVector<QRgb> & pal)   {m_palcnt.setPalette(pal);}

    inline AnimSequences        & getAnimSequences()                            {return m_seqcnt;}
    inline const AnimSequences  & getAnimSequences()const                       {return m_seqcnt;}
    inline AnimSequence         * getAnimSequence(fmt::AnimDB::animseqid_t id)  {return m_seqcnt.getSequenceByID(id);}

    inline AnimTable                & getAnimTable()                    {return m_anmtbl;}
    inline const AnimTable          & getAnimTable()const               {return m_anmtbl;}
    inline ImageContainer           & getImages()                       {return m_imgcnt;}
    inline const ImageContainer     & getImages()const                  {return m_imgcnt;}
    inline MFrame                   * getFrame( fmt::frmid_t id )       {return m_frmcnt.getFrame(id);}
    inline const MFrame             * getFrame( fmt::frmid_t id )const  {return m_frmcnt.getFrame(id);}
    inline FramesContainer          & getFrames()                       {return m_frmcnt;}
    inline const FramesContainer    & getFrames()const                  {return m_frmcnt;}
    Image                           * getImage(fmt::frmid_t idx);
    const Image                     * getImage(fmt::frmid_t idx)const;

    //Return binary version of the sprite
    inline rawdat_t             & getRawData()      {return m_raw;}
    inline const rawdat_t       & getRawData()const {return m_raw;}

    inline fmt::eSpriteType type()const    {return m_sprhndl.getSpriteType();}
    void                    convertSpriteToType(fmt::eSpriteType newty);

    //SPRITE PROPERTIES
    SpritePropertiesHandler         * propHandler();
    const SpritePropertiesHandler   * propHandler()const;

    SpriteOverviewModel             * overviewModel();
    const SpriteOverviewModel       * overviewModel()const;

    inline bool is256Colors()const
    {
        return m_sprhndl.getImageFmtInfo().is256Colors();
    }
    inline void setIs256Colors( bool state )
    {
        m_sprhndl.getImageFmtInfo().setIs256Colors(state);
    }

    //0 == 2D mapping, 1 == 1D mapping
    inline bool     isTiled()const              {return unk13() == 0;}
    inline void     SetIsTiled(bool state)      {return unk13(state);}

    void                                    setTargetCompression(filetypes::eCompressionFormats fmt){ m_targetgompression = fmt;}
    inline filetypes::eCompressionFormats   getTargetCompression()const                             {return m_targetgompression;}

    inline uint16_t unk6()const         {return m_sprhndl.getAnimFmtInfo().unk6;}
    inline void     unk6(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk6 = v;}
    inline uint16_t unk7()const         {return m_sprhndl.getAnimFmtInfo().unk7;}
    inline void     unk7(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk7 = v;}
    inline uint16_t unk8()const         {return m_sprhndl.getAnimFmtInfo().unk8;}
    inline void     unk8(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk8 = v;}
    inline uint16_t unk9()const         {return m_sprhndl.getAnimFmtInfo().unk9;}
    inline void     unk9(uint16_t v)    {m_sprhndl.getAnimFmtInfo().unk9 = v;}
    inline uint16_t unk10()const        {return m_sprhndl.getAnimFmtInfo().unk10;}
    inline void     unk10(uint16_t v)   {m_sprhndl.getAnimFmtInfo().unk10 = v;}
    inline uint16_t unk11()const        {return m_sprhndl.getImageFmtInfo().unk11;}
    inline void     unk11(uint16_t v)   {m_sprhndl.getImageFmtInfo().unk11 = v;}
    inline uint16_t unk12()const        {return m_sprhndl.getUnk12();}
    inline void     unk12(uint16_t v)   {m_sprhndl.getUnk12() = v;}
    inline uint16_t unk13()const        {return m_sprhndl.getImageFmtInfo().unk13;}
    inline void     unk13(uint16_t v)   {m_sprhndl.getImageFmtInfo().unk13 = v;}



private:
    bool IsRawDataCompressed(filetypes::eCompressionFormats * outfmt = nullptr)const;
    void DecompressRawData();
    void CompressRawData(filetypes::eCompressionFormats cpfmt);

    TreeElement       * ElemPtr(int idx);
    TreeElement       * ElemPtrNoEfx(int idx);
    const TreeElement * ElemPtr(int idx)const;
    const TreeElement * ElemPtrNoEfx(int idx)const;
    int                 nbChildCat()const;

private:
    //Raw data buffer & handler
    std::vector<uint8_t>    m_raw;
    fmt::WA_SpriteHandler   m_sprhndl;

    //Converted data
    EffectOffsetContainer                   m_efxcnt;
    PaletteContainer                        m_palcnt;
    ImageContainer                          m_imgcnt;
    FramesContainer                         m_frmcnt;
    AnimSequences                           m_seqcnt;
    AnimTable                               m_anmtbl;
    QScopedPointer<SpritePropertiesHandler> m_propshndlr;

    //Status / statistics
    bool                                    m_bparsed;          //Whether the sprite's raw has been parsed to be displayed yet or not!
    bool                                    m_bhasimagedata;    //Whether the sprite can be displayed or not!
    filetypes::eCompressionFormats          m_targetgompression;
    QScopedPointer<SpriteOverviewModel>     m_overmodel;

    //UI data
    QPixmap m_previewImg;       //Cached image preview
    QPixmap m_previewPal;       //Cached palette preview
};

#endif // SPRITE_H
