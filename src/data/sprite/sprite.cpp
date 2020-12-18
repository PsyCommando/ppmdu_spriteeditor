#include "sprite.hpp"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/data/sprite/sprite_container.hpp>
#include <src/data/sprite/spritemanager.hpp>
#include <src/ui/errorhelper.hpp>

const QString ElemName_Sprite = "Sprite";

//=================================================================================================================
//  Sprite
//=================================================================================================================
void Sprite::_ctor()
{
    m_bparsed             = false;
    m_bhasimagedata       = false;
    m_targetgompression   = filetypes::eCompressionFormats::INVALID;
    m_efxcnt.setParentNode(this);
    m_palcnt.setParentNode(this);
    m_imgcnt.setParentNode(this);
    m_frmcnt.setParentNode(this);
    m_seqcnt.setParentNode(this);
    m_anmtbl.setParentNode(this);
    m_anmgrp.setParentNode(this);
    if(is256Colors())
        m_palcnt.setPalette(QVector<QRgb>(256));
    else
        m_palcnt.setPalette(QVector<QRgb>(16));
}

Sprite::Sprite(TreeNode *parent)
    :TreeNode(parent),
      m_efxcnt(this),
      m_palcnt(this),
      m_imgcnt(this),
      m_frmcnt(this),
      m_seqcnt(this),
      m_anmtbl(this),
      m_anmgrp(this)
{
    _ctor();
    //m_bparsed = true; //Basically a hack to have empty sprites be processed
}

Sprite::Sprite(TreeNode *parent, Sprite::rawdat_t &&raw)
    :TreeNode(parent),
      m_raw(raw),
      m_efxcnt(this),
      m_palcnt(this),
      m_imgcnt(this),
      m_frmcnt(this),
      m_seqcnt(this),
      m_anmtbl(this),
      m_anmgrp(this)
{
    _ctor();
}

Sprite::Sprite(const Sprite &cp)
    :TreeNode(cp),
      m_efxcnt(this),
      m_palcnt(this),
      m_imgcnt(this),
      m_frmcnt(this),
      m_seqcnt(this),
      m_anmtbl(this),
      m_anmgrp(this)
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
    m_efxcnt.setParentNode(this);
    m_palcnt.setParentNode(this);
    m_imgcnt.setParentNode(this);
    m_frmcnt.setParentNode(this);
    m_seqcnt.setParentNode(this);
    m_anmtbl.setParentNode(this);
    return *this;
}

Sprite::Sprite(Sprite && mv)
    :TreeNode(mv),
      m_efxcnt(this),
      m_palcnt(this),
      m_imgcnt(this),
      m_frmcnt(this),
      m_seqcnt(this),
      m_anmtbl(this),
      m_anmgrp(this)
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
    m_efxcnt.setParentNode(this);
    m_palcnt.setParentNode(this);
    m_imgcnt.setParentNode(this);
    m_frmcnt.setParentNode(this);
    m_seqcnt.setParentNode(this);
    m_anmtbl.setParentNode(this);
    return *this;
}

Sprite::~Sprite()
{
    //qDebug("Sprite::~Sprite(): Sprite ID: %d\n", nodeIndex());
}

TreeNode* Sprite::clone()const
{
    return new Sprite(*this);
}

TreeNode *Sprite::nodeChild(int row)
{
    return ElemPtr(row);
}

eTreeElemDataType Sprite::nodeDataTy()const
{
    return eTreeElemDataType::sprite;
}
const QString& Sprite::nodeDataTypeName()const
{
    return ElemName_Sprite;
}

QString Sprite::nodeDisplayName()const
{
    return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());
}

int Sprite::indexOfChild(const TreeNode * ptr)const
{
    if(ptr)
    {
        for(int idx = 0; idx < nbChildCat(); ++idx)
        {
            if(ElemPtr(idx) == ptr)
                return idx;
        }
    }
    throw std::runtime_error("Invalid child!");
    return -1;
}

int Sprite::nodeChildCount() const
{
    return nbChildCat();
}

bool Sprite::canParse()const
{
    return m_raw.size() != 0;
}

void Sprite::ParseSpriteData()
{
    if(!canParse())
        throw ExBadSpriteData(QString("Sprite::ParseSpriteData(): Tried to parse invalid raw data!"));
    if(IsRawDataCompressed(&m_targetgompression))
        DecompressRawData();

    //Run the low level parser
    m_sprhndl.Parse(m_raw.begin(), m_raw.end());

    //Fill up the nodes in our model
    m_anmgrp.importAnimationGroups(m_sprhndl.getAnimGroups());
    m_anmtbl.importAnimationTable(m_sprhndl.getAnimationTable(), m_anmgrp);

    m_palcnt.setPalette(utils::ConvertSpritePalette(m_sprhndl.getPalette())); //convert the palette once, so we don't do it constantly

    m_seqcnt.importSequences(m_sprhndl.getAnimSeqs());
    m_frmcnt.importFrames(m_sprhndl.getFrames());

    if( m_sprhndl.getImageFmtInfo().is256Colors() )
        m_imgcnt.importImages8bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());
    else
        m_imgcnt.importImages4bpp(m_sprhndl.getImages(), m_sprhndl.getFrames());

    if(m_sprhndl.getSpriteType() == fmt::eSpriteType::Character && m_frmcnt.nodeChildCount() > 0)
        m_efxcnt.importEffects(m_sprhndl.getEffectOffset());

    m_bhasimagedata = m_imgcnt.nodeChildCount() != 0;
    m_bparsed = true;
}

void Sprite::CommitSpriteData()
{
    rawdat_t   buffer;
    auto       itback = std::back_inserter(buffer);

    //First convert the data from the data model
    m_sprhndl.setPalette(utils::ConvertSpritePaletteFromQt(m_palcnt.getPalette()));

    if( m_sprhndl.getImageFmtInfo().is256Colors() ) //#FIXME : FIgure out something better!!!!
        m_sprhndl.setImages(m_imgcnt.exportImages8bpp());
    else
        m_sprhndl.setImages(m_imgcnt.exportImages4bpp());

    m_sprhndl.setFrames         (m_frmcnt.exportFrames());
    m_sprhndl.setEffectOffset   (m_efxcnt.exportEffects());

    m_sprhndl.setAnimSeqs       (m_seqcnt.exportSequences());
    m_sprhndl.setAnimGroups     (m_anmgrp.exportAnimationGroups());
    m_sprhndl.setAnimationTable (m_anmtbl.exportAnimationTable(m_anmgrp));

    //Write the data
    itback = m_sprhndl.Write(itback);
    utils::AppendPaddingBytes( itback, buffer.size(), 16, 0xAA ); //Align the whole thing on 16 bytes
    m_raw  = qMove(buffer);

    //Compress if needed at the end!
    if(m_targetgompression != filetypes::eCompressionFormats::INVALID)
        CompressRawData(m_targetgompression);
}


void Sprite::DumpSpriteToStream(QDataStream & outstr)
{
    CommitSpriteData();
    outstr.writeRawData( (const char *)(getRawData().data()), getRawData().size() );
}

void Sprite::DumpSpriteToFile(const QString & fpath)
{
    QSaveFile   sf(fpath);
    QDataStream outstr(&sf);

    if(sf.open(QIODevice::WriteOnly) && sf.error() == QFileDevice::NoError)
    {
        DumpSpriteToStream(outstr);
        sf.commit();
    }
    else
    {
        throw BaseException(sf.errorString());
    }
}


QPixmap &Sprite::MakePreviewPalette()
{
    return (m_previewPal = utils::PaintPaletteToPixmap(getPalette()));
}

QPixmap Sprite::MakePreviewPalette()const
{
    return utils::PaintPaletteToPixmap(getPalette());
}

const QPixmap & Sprite::getCachedPreviewPalette()const
{
    return m_previewPal;
}

QPixmap &Sprite::MakePreviewFrame(bool transparency)
{
    if(hasImageData())
    {
        if(m_frmcnt.nodeHasChildren())
            return m_previewImg = QPixmap::fromImage(m_frmcnt.getFrame(0)->AssembleFrame(0,0, QRect(), nullptr, transparency, this));
        else
            return m_previewImg = QPixmap::fromImage(m_imgcnt.getImage(0)->makeImage(getPalette()));
    }
    return m_previewImg;
}

QPixmap Sprite::MakePreviewFrame(bool transparency)const
{
    if(hasImageData())
    {
        if(m_frmcnt.nodeHasChildren())
            return QPixmap::fromImage(m_frmcnt.getFrame(0)->AssembleFrame(0,0, QRect(), nullptr, transparency, this));
        else
            return QPixmap::fromImage(m_imgcnt.getImage(0)->makeImage(getPalette()));
    }
    return QPixmap();
}

const QPixmap & Sprite::getCachedPreviewFrame()const
{
    return m_previewImg;
}

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

const EffectOffsetSet* Sprite::getAttachMarkers(fmt::frmid_t frmidx)const
{
    return const_cast<Sprite*>(this)->getAttachMarkers(frmidx);
}

EffectOffsetSet* Sprite::getAttachMarkers(fmt::frmid_t frmidx)
{
    if(frmidx < 0 || frmidx > m_efxcnt.nodeChildCount())
    {
        return nullptr;
    }
    return static_cast<EffectOffsetSet*>(m_efxcnt.nodeChild(frmidx));
}

//#TODO: Implement this
void Sprite::convertSpriteToType(fmt::eSpriteType newty)
{
    //Do nothing if its the same type as the current one!
    if(newty == type())
        return;

    switch(newty)
    {
        case fmt::eSpriteType::Prop:
        {
            //Anim groups have a single slot
            ResizeAnimGroupSlots(1);
            //Delete Attach Points
            ClearAttachPoints();
            break;
        }
        case fmt::eSpriteType::Character:
        {
            //Re-arrange anim table, each groups has 8 sequences
            ResizeAnimGroupSlots(8);
            //Create Attach Points for all frames
            CreateAttachPoints();
            break;
        }
        case fmt::eSpriteType::Effect:
        {
            //Re-arrange anim table
            ResizeAnimGroupSlots(1);
            //Delete Attach Points
            ClearAttachPoints();
            break;
        }
        case fmt::eSpriteType::WAT:
        {
            //Re-arrange anim table
            ResizeAnimGroupSlots(1);
            //Delete Attach Points
            ClearAttachPoints();
            break;
        }
        default:
        {
            //Error
            Q_ASSERT(false);
            throw BaseException("Sprite::convertSpriteToType(): Bad sprite type!!");
        }
    };

    m_sprhndl.setSpriteType(newty);
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
    rawdat_t buffer;
    auto     itback = std::back_inserter(buffer);
    filetypes::Decompress(m_raw.begin(), m_raw.end(), itback);
    m_raw = std::move(buffer);
}

void Sprite::CompressRawData(filetypes::eCompressionFormats cpfmt)
{
    rawdat_t buffer;
    auto     itback = std::back_inserter(buffer);
    filetypes::Compress( cpfmt, m_raw.begin(), m_raw.end(), itback);
    m_raw = std::move(buffer);
}

void Sprite::ResizeAnimGroupSlots(int newsz)
{
    m_anmgrp.setNbGroupSlots(newsz);
}

void Sprite::ClearAttachPoints()
{
    m_efxcnt.Clear();
}
void Sprite::CreateAttachPoints()
{
    m_efxcnt.Resize(m_frmcnt.size());
}

TreeNode *Sprite::ElemPtr(int idx)
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
        return &m_anmgrp;
    case 4:
        return &m_anmtbl;
    default:
        Q_ASSERT(false);
    };
    return nullptr;
}

int Sprite::nbChildCat() const
{
    return 5;
}

const TreeNode *Sprite::ElemPtr(int idx) const
{
    return const_cast<Sprite*>(this)->ElemPtr(idx);
}

//bool Sprite::canFetchMore(const QModelIndex & parent)const
//{
//    if(!parent.isValid())
//        return false;
//    Sprite * spr = static_cast<Sprite *>(parent.internalPointer());
//    if(!spr || spr != this)
//        return false;
//    return !wasParsed();
//}

//void Sprite::fetchMore(const QModelIndex & parent)
//{
//    if(!parent.isValid())
//        return;
//    Sprite * spr = static_cast<Sprite *>(parent.internalPointer());
//    if(!spr || spr != this)
//        return;
//    ParseSpriteData();
//}

bool Sprite::nodeCanFetchMore()const
{
    return canParse() && !wasParsed();
}

void Sprite::nodeFetchMore()
{
    ParseSpriteData();
}

bool Sprite::_insertChildrenNode    (TreeNode*, int)                {return false;}
bool Sprite::_insertChildrenNodes   (int, int)                      {return false;}
bool Sprite::_insertChildrenNodes   (const QList<TreeNode*> &, int) {return false;}

bool Sprite::_removeChildrenNode    (TreeNode*)                     {return false;}
bool Sprite::_removeChildrenNodes   (int, int)                      {return false;}
bool Sprite::_removeChildrenNodes   (const QList<TreeNode*> &)      {return false;}

bool Sprite::_deleteChildrenNode    (TreeNode*)                     {return false;}
bool Sprite::_deleteChildrenNodes   (int, int)                      {return false;}
bool Sprite::_deleteChildrenNodes   (const QList<TreeNode*> &)      {return false;}

bool Sprite::_moveChildrenNodes     (int, int, int, TreeNode*)      {return false;}
bool Sprite::_moveChildrenNodes     (QModelIndexList&,int,QModelIndex) {return false;}
bool Sprite::_moveChildrenNodes     (const QList<TreeNode *>&, int, QModelIndex){return false;}

bool Sprite::isImageDoubleSize(const QImage & img)
{
    return img.width() > fmt::MaxFrameResValue.first || img.height() > fmt::MaxFrameResValue.second;
}

fmt::frmid_t Sprite::importImageParts(const imgparts_t & img)
{
    MFrame * newfrm = m_frmcnt.appendNewFrame();
    for(const imgpart_t & part : img)
    {
        Image * newimg = m_imgcnt.appendNewImage();
        //Convert the QImage to raw
        fmt::ImageDB::img_t newimgdata;
        newimgdata.unk2 = 0;
        newimgdata.unk14 = 0;
        //All image data is stored as 8bpp
        newimgdata.data = utils::ImgToRaw(part.second);
        newimg->importImage8bpp(newimgdata, part.second.width(), part.second.height(), false);

        MFramePart * newpart = newfrm->appendNewFramePart();
        newpart->setColorPal256(is256Colors());
        newpart->setDoubleSize(isImageDoubleSize(part.second));
        newpart->setFrameIndex(newimg->getImageUID());
    }
    return newfrm->getFrameUID();
}

fmt::animseqid_t Sprite::importImageSequence(const imgseq_t & seq, uint8_t frmduration)
{
    AnimSequence * newseq = m_seqcnt.appendNewSequence();
    for(const imgparts_t & img : seq)
    {
        fmt::frmid_t curfrmid = importImageParts(img);
        AnimFrame * newafrm = newseq->appendNewAnimFrame();
        newafrm->setFrmidx(curfrmid);
        newafrm->setDuration(frmduration);
    }
    return newseq->nodeIndex();
}

void Sprite::importImageSequences(const imgseqs_t & sequences, uint8_t frmduration)
{
    for(const imgseq_t & seq : sequences)
    {
        importImageSequence(seq, frmduration);
    }
}

//If true the caller will likely force a commit on this sprite!!!
 bool Sprite::hasUnsavedChanges()const
 {
    return !hasRawData(); //#FIXME: Maybe a better system should be implemented since loaded sprites will always have raw data
 }
