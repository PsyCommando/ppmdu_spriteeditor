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
#include <src/treeelem.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/sprite_img.hpp>
#include <src/sprite_anim.hpp>
#include <src/ppmdu/fmts/compression_handler.hpp>

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
    INVALID,
};
extern const QStringList SpriteColorModes;

//Forward declare Sprite
class Sprite;


//*******************************************************************
//  SpritePropertiesDelegate
//*******************************************************************
class SpritePropertiesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    Sprite * m_spr;

    static const int PropValueColLen = 128;
public:

    SpritePropertiesDelegate(Sprite * parentspr, QObject * parent = nullptr)
        :QStyledItemDelegate(parent), m_spr(parentspr)
    {

    }

    virtual ~SpritePropertiesDelegate(){}

    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize ret = QStyledItemDelegate::sizeHint(option, index);
        if(index.column() == static_cast<int>(eSpritePropColumns::Value))
        {
            ret.setWidth( ret.width() + PropValueColLen);
        }
        return ret;
    }

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        //Qt takes ownership of the widget we create!
        if(index.column() != static_cast<int>(eSpritePropColumns::Value))
            return QStyledItemDelegate::createEditor(parent,option,index);

        switch(static_cast<eSpriteProperties>(index.row()))
        {
        case eSpriteProperties::SpriteType:
            {
                QComboBox * pspritetypes = new QComboBox(parent);
                pspritetypes->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
                pspritetypes->setAutoFillBackground(true);
                pspritetypes->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

                for(const auto & s : fmt::SpriteTypeNames)
                    pspritetypes->addItem(QString::fromStdString(s));
                pspritetypes->removeItem(fmt::SpriteTypeNames.size()-1); //remove the last item since we don't want to allow the user to pick it!
                return pspritetypes;
            }
        case eSpriteProperties::Compression:
            {
                QComboBox * pcompty = new QComboBox(parent);
                pcompty->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
                pcompty->setAutoFillBackground(true);
                pcompty->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

                for(const auto & s : CompressionFmtOptions)
                    pcompty->addItem(s);
                return pcompty;
            }
        case eSpriteProperties::ColorMode:
            {
                QComboBox * pcolmode = new QComboBox(parent);
                pcolmode->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
                pcolmode->setAutoFillBackground(true);
                pcolmode->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
                for(const auto & s : SpriteColorModes)
                    pcolmode->addItem(s);
                return pcolmode;
            }
        case eSpriteProperties::Unk6:
        case eSpriteProperties::Unk7:
        case eSpriteProperties::Unk8:
        case eSpriteProperties::Unk9:
        case eSpriteProperties::Unk10:
        case eSpriteProperties::Unk11:
        case eSpriteProperties::Unk12:
        case eSpriteProperties::Unk13:
        default:
            {
                //nothing here
            }
        };

        return QStyledItemDelegate::createEditor(parent,option,index);
    }

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        if(index.column() != static_cast<int>(eSpritePropColumns::Value))
        {
            QStyledItemDelegate::setModelData(editor,model,index);
            return;
        }

        switch(static_cast<eSpriteProperties>(index.row()))
        {
        case eSpriteProperties::SpriteType:
            {
                QComboBox * pspritetypes = static_cast<QComboBox*>(editor);
                Q_ASSERT(pspritetypes);
                model->setData( index, pspritetypes->currentIndex() );
                return;
            }
        case eSpriteProperties::Compression:
            {
                QComboBox * pcompty = static_cast<QComboBox*>(editor);
                Q_ASSERT(pcompty);
                model->setData( index, pcompty->currentIndex() );
                return;
            }
        case eSpriteProperties::ColorMode:
            {
                QComboBox * pcolmode = static_cast<QComboBox*>(editor);
                Q_ASSERT(pcolmode);
                model->setData(index, pcolmode->currentIndex());
                return;
            }
        case eSpriteProperties::Unk6:
        case eSpriteProperties::Unk7:
        case eSpriteProperties::Unk8:
        case eSpriteProperties::Unk9:
        case eSpriteProperties::Unk10:
        case eSpriteProperties::Unk11:
        case eSpriteProperties::Unk12:
        case eSpriteProperties::Unk13:
        default:
            {
                //nothing here
            }
        };
        QStyledItemDelegate::setModelData(editor,model,index);
    }

    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override
    {
        editor->setGeometry(option.rect);
    }
};

//*******************************************************************
//  SpritePropertiesModel
//*******************************************************************
class SpritePropertiesModel : public QAbstractItemModel
{
    Q_OBJECT
    Sprite * m_spr;

//    //PropNode
//    // Dummy entry for a unique Sprite property
//    class PropNode : public BaseTreeTerminalChild<&ElemName_SpriteProperty>
//    {
//    public:
//        //Nothing here
//    };
//    QList<PropNode> m_dummyProps; //Since we need pointer to an entity in an index, we use these
public:

    SpritePropertiesModel(Sprite * parentspr, QObject * parent = nullptr);

    virtual ~SpritePropertiesModel();

    int getNbProperties()const;

    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    virtual QModelIndex parent(const QModelIndex &/*child*/) const override;

    virtual int rowCount(const QModelIndex &parent) const override;

    virtual int columnCount(const QModelIndex &/*parent*/) const override;

    virtual bool hasChildren(const QModelIndex &parent) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;


    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QVariant dataDisplay(int propid, int column)const;

    QVariant getNameForProperty(int propid)const;

    QVariant getDataForProperty(int propid, int role)const;

    QVariant getDescForProperty(int propid)const;

    void setDataForProperty( eSpriteProperties propid, const QVariant & data );

signals:
    void spriteTypeSet(fmt::eSpriteType);

public:


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

    void _ctor()
    {
        m_bparsed = false;
        m_bhasimagedata = false;
        m_targetgompression = filetypes::eCompressionFormats::INVALID;
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
        setNodeDataTy(eTreeElemDataType::sprite);
        m_propshndlr.reset( new SpritePropertiesHandler(this) );
    }

public:
    friend class SpritePropertiesHandler;

    typedef std::vector<uint8_t> rawdat_t;

    Sprite( TreeElement * parent )
        :TreeElement(parent),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)

    {
        _ctor();
    }

    Sprite( TreeElement * parent, rawdat_t && raw )
        :TreeElement(parent),
          m_raw(raw),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)
    {
        _ctor();
    }


    Sprite( const Sprite & cp )
        :TreeElement(cp),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)
    {
        _ctor();
        operator=(cp);
    }

    Sprite & operator=(const Sprite & cp)
    {
        m_sprhndl               = cp.m_sprhndl;
        m_efxcnt                = cp.m_efxcnt;
        m_palcnt                = cp.m_palcnt;
        m_imgcnt                = cp.m_imgcnt;
        m_frmcnt                = cp.m_frmcnt;
        m_seqcnt                = cp.m_seqcnt;
        m_anmtbl                = cp.m_anmtbl;
        m_bparsed               = cp.m_bparsed;
        m_bhasimagedata         = cp.m_bhasimagedata;
        m_targetgompression     = cp.m_targetgompression;
        m_raw                   = cp.m_raw;

        //Update the pointer to our instance
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
        return *this;
    }

    Sprite( Sprite && mv )
        :TreeElement(mv),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)
    {
        _ctor();
        operator=(mv);
    }

    Sprite & operator=(Sprite && mv)
    {
        m_sprhndl               = std::move(mv.m_sprhndl);
        m_efxcnt                = std::move(mv.m_efxcnt);
        m_palcnt                = std::move(mv.m_palcnt);
        m_imgcnt                = std::move(mv.m_imgcnt);
        m_frmcnt                = std::move(mv.m_frmcnt);
        m_seqcnt                = std::move(mv.m_seqcnt);
        m_anmtbl                = std::move(mv.m_anmtbl);
        m_bparsed               = mv.m_bparsed;
        m_bhasimagedata         = mv.m_bhasimagedata;
        m_targetgompression     = mv.m_targetgompression;
        m_raw                   = std::move(mv.m_raw);

        //Update the pointer to our instance
        m_efxcnt.m_parentItem = this;
        m_palcnt.m_parentItem = this;
        m_imgcnt.m_parentItem = this;
        m_frmcnt.m_parentItem = this;
        m_seqcnt.m_parentItem = this;
        m_anmtbl.m_parentItem = this;
        return *this;
    }

    ~Sprite()
    {
        qDebug("Sprite::~Sprite(): Sprite ID: %d\n", nodeIndex());
    }


    void FillSpriteProperties(QTableWidget * tbl)
    {
        tbl->clearContents();
        tbl->setCellWidget(0,0, new QLabel(QString( "Raw Size" )));
        tbl->setCellWidget(0,1, new QLabel( QString( "%1b" ).arg(m_raw.size()) ));
    }

public:

    TreeElement *nodeChild(int row) override
    {
        return ElemPtr(row);
    }

    int nodeChildCount() const override
    {

//        switch(getSpriteType())
//        {
//        case fmt::eSpriteType::Prop:
//            return 5; // No effects offsets
//        case fmt::eSpriteType::Character:
//            return 6;
//        case fmt::eSpriteType::Effect:
//            return 5; // No effects offsets
//        case fmt::eSpriteType::WAT:
//            return 5; // No effects offsets
//        default:
//            break;
//        };

        return NBChilds;
    }

    int nodeIndex() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOfNode(const_cast<Sprite*>(this));

        return -1;
    }

    int indexOfNode( TreeElement * ptr )const override
    {
        //Search a matching child in the list!
        for( int idx = 0; idx < NBChilds; ++idx )
        {
            if(ElemPtr(idx) == ptr)
                return idx;
        }
        return -1;
    }

    int nodeColumnCount() const override
    {
        return 1; //Always just 1 column
    }

    TreeElement *parentNode() override
    {
        return m_parentItem;
    }

    QVariant nodeData(int column, int role) const override
    {
        if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(QString("Sprite#%1").arg(nodeIndex()));
        return QVariant();
    }

    virtual Sprite *parentSprite()override{return this;}

    inline void OnClicked() override
    {
        //Only parse sprites that were loaded from file! Not newly created ones, or already parsed ones!
        if( m_raw.size() != 0 && !m_bparsed )
            ParseSpriteData();
    }

    inline void OnExpanded() override
    {
        OnClicked();
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

        m_bhasimagedata = m_imgcnt.nodeChildCount() != 0;
        m_bparsed = true;
    }

    /*
     * Turns the data from the data structure to its native format into the m_raw container!
    */
    void CommitSpriteData()
    {
        rawdat_t   buffer;
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

    //Don't allow that, since it would change the sprite type..
    bool insertChildrenNodes(int, int) override {return false;}
    bool removeChildrenNodes(int, int) override {return false;}

    inline bool operator==( const Sprite & other)const  {return this == &other;}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}


    QPixmap & MakePreviewPalette()
    {
        m_previewPal = utils::PaintPaletteToPixmap(getPalette()); // utils::ConvertSpritePalette(m_sprhndl.getPalette()) );
        return m_previewPal;
    }

    QPixmap & MakePreviewFrame(bool transparency = false)
    {
        if(wasParsed() && hasImageData())
        {
            if(m_frmcnt.hasChildren())
                return m_previewImg = std::move(QPixmap::fromImage(m_frmcnt.getFrame(0)->AssembleFrame(0,0, nullptr, transparency)) );
            else
                return m_previewImg = std::move(QPixmap::fromImage(m_imgcnt.getImage(0)->makeImage(getPalette())) );
        }
        return m_previewImg;
    }
    static Sprite * ParentSprite( TreeElement * parentspr ) {return static_cast<Sprite*>(parentspr); }

    const QVector<QRgb> & getPalette()const { return m_palcnt.m_pal; }
    QVector<QRgb>       & getPalette() { return m_palcnt.m_pal; }

    inline void setPalette(QVector<QRgb> && pal) {m_palcnt.m_pal = qMove(pal);}
    inline void setPalette(const QVector<QRgb> & pal) {m_palcnt.m_pal = pal;}

    inline AnimSequences        & getAnimSequences()        {return m_seqcnt;}
    inline const AnimSequences  & getAnimSequences()const   {return m_seqcnt;}

    inline AnimSequence         * getAnimSequence(fmt::AnimDB::animseqid_t id)        {return m_seqcnt.getSequenceByID(id);}
    //inline const AnimSequences  * getAnimSequence(fmt::AnimDB::animseqid_t id)const   {return m_seqcnt.getSequenceByID(id);}

    inline MFrame * getFrame( fmt::frmid_t id ) { return m_frmcnt.getFrame(id); }
    inline const MFrame * getFrame( fmt::frmid_t id )const { return m_frmcnt.getFrame(id); }

    inline Image * getImage(fmt::frmid_t idx)
    {
        if( idx >= 0 && idx < m_imgcnt.nodeChildCount() )
            return m_imgcnt.getImage(idx);
        else
            return nullptr;
    }
    inline const Image * getImage(fmt::frmid_t idx)const
    {
        return const_cast<Sprite*>(this)->m_imgcnt.getImage(idx);
    }

    inline const rawdat_t & getRawData()const
    {
        return m_raw;
    }

    inline ImageContainer & getImages()
    {
        return m_imgcnt;
    }

    inline const ImageContainer & getImages()const
    {
        return m_imgcnt;
    }

    inline fmt::eSpriteType getSpriteType()const
    {
        return m_sprhndl.getSpriteType();
    }

    void convertSpriteToType(fmt::eSpriteType newty)
    {
        //Do nothing if its the same type as the current one!
        if(newty == getSpriteType())
            return;

        switch(newty)
        {
        case fmt::eSpriteType::Prop:

        case fmt::eSpriteType::Character:

        case fmt::eSpriteType::Effect:

        case fmt::eSpriteType::WAT:

        default:
            break;
        };

        //#TODO:!!
        Q_ASSERT(false);
    }

    //SPRITE PROPERTIES
    SpritePropertiesHandler * propHandler()
    {
        return m_propshndlr.data();
    }

    const SpritePropertiesHandler * propHandler()const
    {
        return m_propshndlr.data();
    }

    inline bool is256Colors()const
    {
        return m_sprhndl.getImageFmtInfo().is256Colors();
    }

    inline void setIs256Colors( bool state )
    {
        m_sprhndl.getImageFmtInfo().setIs256Colors(state);
    }

    void setTargetCompression(filetypes::eCompressionFormats fmt)
    {
        m_targetgompression = fmt;
    }
    inline filetypes::eCompressionFormats getTargetCompression()const
    {
        return m_targetgompression;
    }


    inline uint16_t unk6()const  {return m_sprhndl.getAnimFmtInfo().unk6;}
    inline void unk6(uint16_t v) {m_sprhndl.getAnimFmtInfo().unk6 = v;}

    inline uint16_t unk7()const  {return m_sprhndl.getAnimFmtInfo().unk7;}
    inline void unk7(uint16_t v) {m_sprhndl.getAnimFmtInfo().unk7 = v;}

    inline uint16_t unk8()const  {return m_sprhndl.getAnimFmtInfo().unk8;}
    inline void unk8(uint16_t v) {m_sprhndl.getAnimFmtInfo().unk8 = v;}

    inline uint16_t unk9()const  {return m_sprhndl.getAnimFmtInfo().unk9;}
    inline void unk9(uint16_t v) {m_sprhndl.getAnimFmtInfo().unk9 = v;}

    inline uint16_t unk10()const  {return m_sprhndl.getAnimFmtInfo().unk10;}
    inline void unk10(uint16_t v) {m_sprhndl.getAnimFmtInfo().unk10 = v;}

    inline uint16_t unk11()const  {return m_sprhndl.getImageFmtInfo().unk11;}
    inline void unk11(uint16_t v) {m_sprhndl.getImageFmtInfo().unk11 = v;}

    inline uint16_t unk12()const  {return m_sprhndl.getUnk12();}
    inline void unk12(uint16_t v) {m_sprhndl.getUnk12() = v;}

    inline uint16_t unk13()const  {return m_sprhndl.getImageFmtInfo().unk13;}
    inline void unk13(uint16_t v) {m_sprhndl.getImageFmtInfo().unk13 = v;}

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
        filetypes::eCompressionFormats fmt =filetypes::IndentifyCompression(m_raw.begin(), m_raw.end());

        rawdat_t buffer;
        auto                 itback = std::back_inserter(buffer);
        if(fmt == filetypes::eCompressionFormats::PKDPX)
            filetypes::DecompressPKDPX(m_raw.begin(), m_raw.end(), itback);
//        else if(fmt == filetypes::eCompressionFormats::AT4PX)
//            filetypes::DecompressAT4PX(m_raw.begin(), m_raw.end(), itback);
        m_raw = std::move(buffer);
    }

    void CompressRawData(filetypes::eCompressionFormats cpfmt)
    {
        rawdat_t buffer;
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
        default:
            Q_ASSERT(false);
        };
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

    bool                    m_bparsed;          //Whether the sprite's raw has been parsed to be displayed yet or not!
    bool                    m_bhasimagedata;    //Whether the sprite can be displayed or not!
    filetypes::eCompressionFormats m_targetgompression;

public:
    inline bool wasParsed()const    {return m_bparsed;}
    inline bool hasImageData()const {return m_imgcnt.hasChildren();}

    //Raw data buffer
    std::vector<uint8_t>    m_raw;
    QPixmap                 m_previewImg;       //Cached image preview
    QPixmap                 m_previewPal;       //Cached palette preview
    fmt::WA_SpriteHandler   m_sprhndl;
    QScopedPointer<SpritePropertiesHandler> m_propshndlr;
};

#endif // SPRITE_H
