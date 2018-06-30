#include "sprite.h"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
#include <src/ppmdu/fmts/wa_sprite.hpp>

//=================================================================================================================
//  Sprite
//=================================================================================================================
void Sprite::_ctor()
{
    setNodeDataTy(eTreeElemDataType::sprite);
    m_bparsed             = false;
    m_bhasimagedata       = false;
    m_targetgompression   = filetypes::eCompressionFormats::INVALID;
    m_efxcnt.m_parentItem = this;
    m_palcnt.m_parentItem = this;
    m_imgcnt.m_parentItem = this;
    m_frmcnt.m_parentItem = this;
    m_seqcnt.m_parentItem = this;
    m_anmtbl.m_parentItem = this;
    m_propshndlr.reset(new SpritePropertiesHandler(this));
    m_overmodel .reset(new SpriteOverviewModel(this));
}

Sprite::Sprite(TreeElement *parent)
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

Sprite::Sprite(TreeElement *parent, Sprite::rawdat_t &&raw)
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

Sprite::Sprite(const Sprite &cp)
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

Sprite &Sprite::operator=(const Sprite &cp)
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

Sprite::Sprite(Sprite && mv)
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

Sprite &Sprite::operator=(Sprite && mv)
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

Sprite::~Sprite()
{
    qDebug("Sprite::~Sprite(): Sprite ID: %d\n", nodeIndex());
}

void Sprite::clone(const TreeElement *other)
{
    const Sprite * spr = static_cast<const Sprite*>(other);
    if(!spr)
        throw std::runtime_error("Sprite::clone(): other is not a Sprite!");
    (*this) = *spr;
}

TreeElement *Sprite::nodeChild(int row)
{
    return ElemPtr(row);
}

int Sprite::nodeChildCount() const
{
//    switch(type())
//    {
//    case fmt::eSpriteType::Prop:
//        return 5; // No effects offsets, no animation groups
//    case fmt::eSpriteType::Character:
//        return 6;
//    case fmt::eSpriteType::Effect:
//        return 5; // No effects offsets, no animation groups
//    case fmt::eSpriteType::WAT:
//        return 5; // No effects offsets
//    default:
//        Q_ASSERT(false);
//    };
    return nbChildCat();
}

int Sprite::nodeIndex() const
{
    Q_ASSERT(m_parentItem);
    return m_parentItem->indexOfNode(const_cast<Sprite*>(this));
}

int Sprite::indexOfNode(TreeElement *ptr) const
{
    //Search a matching child in the list!
    for( int idx = 0; idx < nbChildCat(); ++idx )
    {
        if(ElemPtr(idx) == ptr)
            return idx;
    }
    qWarning("Sprite::indexOfNode(): Couldn't find node!!\n");
    Q_ASSERT(false);
    return -1;
}

int Sprite::nodeColumnCount() const
{
    return 1; //Always just 1 column
}

TreeElement *Sprite::parentNode()
{
    return m_parentItem;
}

QVariant Sprite::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(QString("Sprite#%1").arg(nodeIndex()));
    return QVariant();
}

Sprite *Sprite::parentSprite(){return this;}

void Sprite::OnClicked()
{
    //Only parse sprites that were loaded from file! Not newly created ones, or already parsed ones!
    if( m_raw.size() != 0 && !m_bparsed )
        ParseSpriteData();
}

void Sprite::OnExpanded()
{
    OnClicked();
}

void Sprite::ParseSpriteData()
{
    if(IsRawDataCompressed(&m_targetgompression))
        DecompressRawData();

    m_sprhndl.Parse( m_raw.begin(), m_raw.end() );
    m_anmtbl.importAnimationTable(m_sprhndl.getAnimationTable());
    m_anmtbl.importAnimationGroups( m_sprhndl.getAnimGroups() );

    m_palcnt.setPalette(std::move(utils::ConvertSpritePalette(m_sprhndl.getPalette()))); //convert the palette once, so we don't do it constantly

    m_seqcnt.importSequences( m_sprhndl.getAnimSeqs());
    m_frmcnt.importFrames(m_sprhndl.getFrames());

    if( m_sprhndl.getImageFmtInfo().is256Colors() )
        m_imgcnt.importImages8bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());
    else
        m_imgcnt.importImages4bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());

    m_bhasimagedata = m_imgcnt.nodeChildCount() != 0;
    m_bparsed = true;
}

void Sprite::CommitSpriteData()
{
    rawdat_t   buffer;
    auto       itback = std::back_inserter(buffer);

    //First convert the data from the UI
    m_sprhndl.getAnimationTable()   = m_anmtbl.exportAnimationTable();
    m_sprhndl.getAnimGroups()       = m_anmtbl.exportAnimationGroups();
    m_sprhndl.getPalette()          = utils::ConvertSpritePaletteFromQt(m_palcnt.getPalette());
    m_sprhndl.getAnimSeqs()         = m_seqcnt.exportSequences();
    m_sprhndl.getFrames()           = m_frmcnt.exportFrames();

    if( m_sprhndl.getImageFmtInfo().is256Colors() ) //#FIXME : FIgure out something better!!!!
        m_sprhndl.getImages() = m_imgcnt.exportImages8bpp();
    else
        m_sprhndl.getImages() = m_imgcnt.exportImages4bpp();

    //Write the data
    itback = m_sprhndl.Write(itback);
    utils::AppendPaddingBytes( itback, buffer.size(), 16, 0xAA ); //Align the whole thing on 16 bytes
    m_raw  = qMove(buffer);

    //Compress if needed at the end!
    if(m_targetgompression != filetypes::eCompressionFormats::INVALID)
        CompressRawData(m_targetgompression);
}

QPixmap &Sprite::MakePreviewPalette()
{
    m_previewPal = utils::PaintPaletteToPixmap(getPalette()); // utils::ConvertSpritePalette(m_sprhndl.getPalette()) );
    return m_previewPal;
}

QPixmap &Sprite::MakePreviewFrame(bool transparency)
{
    if(wasParsed() && hasImageData())
    {
        if(m_frmcnt.hasChildren())
            return m_previewImg = std::move(QPixmap::fromImage(m_frmcnt.getFrame(0)->AssembleFrame(0,0, QRect(), nullptr, transparency)) );
        else
            return m_previewImg = std::move(QPixmap::fromImage(m_imgcnt.getImage(0)->makeImage(getPalette())) );
    }
    return m_previewImg;
}

//Sprite *Sprite::ParentSprite(TreeElement *parentspr)
//{
//    return static_cast<Sprite*>(parentspr);
//}

Image *Sprite::getImage(fmt::frmid_t idx)
{
    if( idx >= 0 && idx < m_imgcnt.nodeChildCount() )
        return m_imgcnt.getImage(idx);
    else
        return nullptr;
}

const Image *Sprite::getImage(fmt::frmid_t idx) const
{
    return const_cast<Sprite*>(this)->m_imgcnt.getImage(idx);
}



void Sprite::convertSpriteToType(fmt::eSpriteType newty)
{
    //Do nothing if its the same type as the current one!
    if(newty == type())
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

SpritePropertiesHandler *Sprite::propHandler()
{
    return m_propshndlr.data();
}

const SpritePropertiesHandler *Sprite::propHandler() const
{
    return m_propshndlr.data();
}

SpriteOverviewModel *Sprite::overviewModel()
{
    return m_overmodel.data();
}

const SpriteOverviewModel *Sprite::overviewModel() const
{
    return m_overmodel.data();
}





bool Sprite::IsRawDataCompressed(filetypes::eCompressionFormats *outfmt) const
{
    filetypes::eCompressionFormats fmt = filetypes::IndentifyCompression( m_raw.begin(), m_raw.end() );
    if(outfmt)
        (*outfmt) = fmt;
    return fmt < filetypes::eCompressionFormats::INVALID;
}

void Sprite::DecompressRawData()
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

void Sprite::CompressRawData(filetypes::eCompressionFormats cpfmt)
{
    rawdat_t buffer;
    auto                 itback = std::back_inserter(buffer);
    filetypes::Compress( cpfmt, m_raw.begin(), m_raw.end(), itback);
    m_raw = std::move(buffer);
}

TreeElement *Sprite::ElemPtr(int idx)
{
    if( !hasEfxOffsets() )
        return ElemPtrNoEfx(idx);

    switch(idx)
    {
    case 0:
        return &m_imgcnt;
    case 1:
        return &m_frmcnt;
    case 2:
        return &m_seqcnt;
    case 3:
        return &m_anmtbl;
    case 4:
        return &m_efxcnt;
    default:
        Q_ASSERT(false);
    };
    return nullptr;
}

TreeElement *Sprite::ElemPtrNoEfx(int idx)
{
    switch(idx)
    {
    case 0:
        return &m_imgcnt;
    case 1:
        return &m_frmcnt;
    case 2:
        return &m_seqcnt;
    case 3:
        return &m_anmtbl;
    default:
        Q_ASSERT(false);
    };
    return nullptr;
}

const TreeElement *Sprite::ElemPtr(int idx) const
{
    return const_cast<Sprite*>(this)->ElemPtr(idx);
}

const TreeElement *Sprite::ElemPtrNoEfx(int idx) const
{
    return const_cast<Sprite*>(this)->ElemPtrNoEfx(idx);
}

int Sprite::nbChildCat() const
{
    if( !hasEfxOffsets() )
        return 4;
    else
        return 5;
}
