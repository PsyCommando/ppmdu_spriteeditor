#include "sprite.hpp"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
#include <QComboBox>
#include <QXmlStreamWriter>
#include <src/ppmdu/fmts/wa_sprite.hpp>

//const char * ElemName_EffectOffset  = "Effect Offsets";
//const char * ElemName_Palette       = "Palette";
//const char * ElemName_Images        = "Images";
//const char * ElemName_Image         = "Image";
//const char * ElemName_FrameCnt      = "Frames";
//const char * ElemName_Frame         = "Frame";
//const char * ElemName_AnimSequence  = "Anim Sequence";
//const char * ElemName_AnimSequences = "Anim Sequences";
//const char * ElemName_AnimTable     = "Animation Table";
//const char * ElemName_AnimGroup     = "Anim Group";
//const char * ElemName_AnimFrame     = "Anim Frame";
//const char * ElemName_FramePart     = "Frame Part";
//const char * ElemName_SpriteProperty= "Property";

const QStringList CompressionFmtOptions
{
    "PKDPX",
    "AT4PX",
    "AT4PN",
    "Uncompressed",
};

filetypes::eCompressionFormats CompOptionToCompFmt( eCompressionFmtOptions opt )
{
    switch(opt)
    {
    case eCompressionFmtOptions::PKDPX:
        return filetypes::eCompressionFormats::PKDPX;

    case eCompressionFmtOptions::AT4PX:
        return filetypes::eCompressionFormats::AT4PX;

    case eCompressionFmtOptions::NONE:
    default:
        return filetypes::eCompressionFormats::INVALID;
    };
}

eCompressionFmtOptions CompFmtToCompOption( filetypes::eCompressionFormats fmt )
{
    switch(fmt)
    {
    case filetypes::eCompressionFormats::PKDPX:
        return eCompressionFmtOptions::PKDPX;

    case filetypes::eCompressionFormats::AT4PX:
        return eCompressionFmtOptions::AT4PX;

    case filetypes::eCompressionFormats::INVALID:
    default:
        return eCompressionFmtOptions::NONE;
    };
}

const QStringList SpriteColorModes
{
  "16 colors",
  "256 colors",
  "bitmap",
};



//=================================================================================================================
//  SpritePropertiesHandler
//=================================================================================================================

//=================================================================================================================
//  SpriteXMLSharedConsts
//=================================================================================================================
const QString XML_NODE_SPRITE       = "Sprite";
const QString XML_NODE_PROPERTIES   = "Properties";
const QString XML_NODE_IMAGES       = "Images";
const QString XML_NODE_IMAGE        = "Image";
const QString XML_NODE_IMAGEREF     = "ImageRef";
const QString XML_NODE_FRAMES       = "Frames";
const QString XML_NODE_FRAME        = "Frame";
const QString XML_NODE_FRAMEPART    = "FramePart";
const QString XML_NODE_SEQUENCES    = "AnimSequences";
const QString XML_NODE_SEQUENCE     = "AnimSequence";
const QString XML_NODE_SEQUENCEREF  = "AnimSequenceRef";
const QString XML_NODE_ANIMFRAME    = "AnimFrame";
const QString XML_NODE_GROUPS       = "AnimGroups";
const QString XML_NODE_GROUP        = "AnimGroup";
const QString XML_NODE_GROUPREF     = "AnimGroupRef";
const QString XML_NODE_TABLE        = "AnimTable";
const QString XML_NODE_ATTACH       = "Attachments";

const QString XML_ATTR_SPRTYPE      = "type";

const QString
XML_ELEM_PALIS256 = "Is256Colors",
XML_ELEM_MOSAIC = "IsMosaicModeEnabled",
XML_ELEM_DISABLED = "IsDisabled",
XML_ELEM_ROT_CANVAS = "IsRotCanvas",
XML_ELEM_RNS = "IsRotationAndScalingOn",
XML_ELEM_RNS_PARAM = "RotationAndScalingParam",
XML_ELEM_XOFF = "X",
XML_ELEM_YOFF = "Y",
XML_ELEM_VFLIP = "VFlipped",
XML_ELEM_HFLIP = "HFlipped",
XML_ELEM_PRIO = "Priority",
XML_ELEM_RES = "ResolutionType",
XML_ELEM_IMG = "ImageId",
XML_ELEM_PAL = "PaletteId",
XML_ELEM_TILE = "Tile",
XML_ELEM_OBJM = "ObjectMode",

XML_ELEM_TILE_MAPPING_MODE = "Unk13",
XML_ELEM_UNK11 = "Unk11",
XML_ELEM_UNK6 = "Unk6",
XML_ELEM_UNK7 = "Unk7",
XML_ELEM_UNK8 = "Unk8",
XML_ELEM_UNK9 = "Unk9",
XML_ELEM_UNK10 = "Unk10",
XML_ELEM_COMP  = "Compression",

XML_ELEM_OFFSET = "Offset",
XML_ELEM_NAME = "Name",
XML_ELEM_HEAD = "Head",
XML_ELEM_RH = "RightHand",
XML_ELEM_LH = "LeftHand",
XML_ELEM_CENTER = "Center",

XML_ATTR_FRAMEID = "_id",
XML_ATTR_ID = "id",
XML_ATTR_DURATION = "duration",
XML_ATTR_XOFF = "x",
XML_ATTR_YOFF = "y",
XML_ATTR_SHADXOFF = "shadow_x",
XML_ATTR_SHADYOFF = "shadow_y",

XML_ATTR_UNK16 = "unk16",

XML_ATTR_SEQID = "_id",
XML_ATTR_SEQ_REF = "seq_id"
;


//=================================================================================================================
//  SpriteXMLReader
//=================================================================================================================

//=================================================================================================================
//  SpriteXMLWriter
//=================================================================================================================
class SpriteXMLWriter
{
public:
    SpriteXMLWriter(const Sprite & spr)
        :m_spr(spr)
    {
    }


    void Write(QSaveFile & outf)
    {
        //1. Dump images into matching dir
        QFileInfo xmlfile(outf.fileName());
        QDir targetimg = xmlfile.dir();
        targetimg.mkdir(xmlfile.baseName());
        targetimg.cd(xmlfile.baseName());
        m_spr.getImages().DumpAllImages(targetimg.absolutePath(), m_spr.getPalette());

        //2. Write XML
        QXmlStreamWriter writer(&outf);
        writer.setAutoFormatting(true);

        writer.writeStartElement(XML_NODE_SPRITE);
        WriteProperties(writer);
        WriteImages(writer);
        WriteFrames(writer);
        WriteAnimSequences(writer);
        WriteAnimTable(writer);
        writer.writeEndElement();
    }

    void WriteProperties(QXmlStreamWriter & writer)
    {
        writer.writeStartElement(XML_NODE_PROPERTIES);
        writer.writeTextElement(XML_ELEM_COMP,      QString("%1").arg(static_cast<int>(m_spr.getTargetCompression())));
        writer.writeTextElement(XML_ATTR_SPRTYPE,   QString("%1").arg(static_cast<int>(m_spr.type())));
        writer.writeTextElement(XML_ELEM_TILE_MAPPING_MODE,     QString("%1").arg(static_cast<int>(m_spr.getTileMappingMode())));
        writer.writeTextElement(XML_ELEM_PALIS256,  QString("%1").arg(m_spr.is256Colors()));
        writer.writeTextElement(XML_ELEM_UNK11,     QString("%1").arg(m_spr.unk11()));

        writer.writeTextElement(XML_ELEM_UNK7,     QString("%1").arg(m_spr.unk7()));
        writer.writeTextElement(XML_ELEM_UNK8,     QString("%1").arg(m_spr.unk8()));
        writer.writeTextElement(XML_ELEM_UNK9,     QString("%1").arg(m_spr.unk9()));
        writer.writeTextElement(XML_ELEM_UNK10,    QString("%1").arg(m_spr.unk10()));


        writer.writeEndElement();
    }

    void WriteImages(QXmlStreamWriter & writer)
    {
        writer.writeStartElement(XML_NODE_IMAGES);
        writer.writeEndElement();
    }

    void WriteFrames(QXmlStreamWriter & writer)
    {
        writer.writeStartElement(XML_NODE_FRAMES);

        for(const MFrame * f : m_spr.getFrames())
        {
            WriteFrame(writer, *f);
        }

        writer.writeEndElement();
    }

    void WriteFrame(QXmlStreamWriter & writer, const MFrame & frm)
    {
        writer.writeComment(QString("%1").arg(frm.getFrameUID()));
        writer.writeStartElement(XML_NODE_FRAME);
        //writer.writeAttribute(XML_ATTR_FRAMEID, QString(frm.getFrameUID()));
        for(const MFramePart * p : frm)
        {
            writer.writeStartElement(XML_NODE_FRAMEPART);
            writer.writeTextElement(XML_ELEM_IMG,       QString("%1").arg(p->getFrameIndex()));
            writer.writeTextElement(XML_ELEM_TILE,      QString("%1").arg(p->getBlockNum()));
            writer.writeTextElement(XML_ELEM_PAL,       QString("%1").arg(p->getPalNb()));
            writer.writeTextElement(XML_ELEM_XOFF,      QString("%1").arg(p->getXOffset()));
            writer.writeTextElement(XML_ELEM_YOFF,      QString("%1").arg(p->getYOffset()));
            writer.writeTextElement(XML_ELEM_VFLIP,     QString("%1").arg(p->isVFlip()));
            writer.writeTextElement(XML_ELEM_HFLIP,     QString("%1").arg(p->isHFlip()));
            writer.writeTextElement(XML_ELEM_PRIO,      QString("%1").arg(p->getPriority()));
            writer.writeTextElement(XML_ELEM_RES,       QString("%1").arg(static_cast<int>(p->getResolutionType())));

            writer.writeTextElement(XML_ELEM_OBJM,      QString("%1").arg(static_cast<int>(p->getObjMode())));
            writer.writeTextElement(XML_ELEM_PALIS256,  QString("%1").arg(p->isColorPal256()));
            writer.writeTextElement(XML_ELEM_MOSAIC,    QString("%1").arg(p->isMosaicOn()));
            writer.writeTextElement(XML_ELEM_DISABLED,  QString("%1").arg(p->isDisabled()));
            writer.writeTextElement(XML_ELEM_ROT_CANVAS,QString("%1").arg(p->isRnSRotCanvas()));
            writer.writeTextElement(XML_ELEM_RNS,       QString("%1").arg(p->isRotAndScalingOn()));
            writer.writeTextElement(XML_ELEM_RNS_PARAM, QString("%1").arg(p->getRnSParam())),
            writer.writeEndElement();
        }

        //Attachment points if any
        writer.writeStartElement(XML_NODE_ATTACH);
        const EffectOffsetSet * markers = m_spr.getAttachMarkers(frm.getFrameUID());
        if(markers)
        {
            WriteAttachment(writer, XML_ELEM_HEAD,  markers->getHead());
            WriteAttachment(writer, XML_ELEM_RH,    markers->getRHand());
            WriteAttachment(writer, XML_ELEM_LH,    markers->getLHand());
            WriteAttachment(writer, XML_ELEM_CENTER,markers->getCenter());
        }
        writer.writeEndElement();
        writer.writeEndElement();
    }

    void WriteAttachment(QXmlStreamWriter & writer, const QString & name, const fmt::offset_t & off)
    {
        writer.writeStartElement(XML_ELEM_OFFSET);
        writer.writeAttribute(XML_ELEM_NAME, name);
        writer.writeAttribute(XML_ELEM_XOFF, QString("%1").arg(off.xoff));
        writer.writeAttribute(XML_ELEM_YOFF, QString("%1").arg(off.yoff));
        writer.writeEndElement();
    }

    void WriteAnimSequences(QXmlStreamWriter & writer)
    {
        writer.writeStartElement(XML_NODE_SEQUENCES);
        for(const AnimSequence * s : m_spr.getAnimSequences())
        {
            WriteAnimSequence(writer, *s);
        }
        writer.writeEndElement();
    }

    void WriteAnimSequence(QXmlStreamWriter & writer, const AnimSequence & seq)
    {
        writer.writeComment(QString("%1").arg(seq.nodeIndex()));
        writer.writeStartElement(XML_NODE_SEQUENCE);
        for(const AnimFrame * f : seq)
        {
            WriteAnimFrame(writer, *f);
        }
        writer.writeEndElement();
    }

    void WriteAnimFrame(QXmlStreamWriter & writer, const AnimFrame & frm)
    {
        writer.writeStartElement(XML_NODE_ANIMFRAME);
        writer.writeAttribute(XML_ATTR_DURATION,    QString("%1").arg(frm.duration()));
        writer.writeAttribute(XML_ATTR_ID,          QString("%1").arg(frm.frmidx()));
        writer.writeAttribute(XML_ATTR_XOFF,        QString("%1").arg(frm.xoffset()));
        writer.writeAttribute(XML_ATTR_YOFF,        QString("%1").arg(frm.yoffset()));
        writer.writeAttribute(XML_ATTR_SHADXOFF,    QString("%1").arg(frm.shadowx()));
        writer.writeAttribute(XML_ATTR_SHADYOFF,    QString("%1").arg(frm.shadowy()));
        writer.writeEndElement();
    }

    void WriteAnimGroup(QXmlStreamWriter & writer, const AnimGroup & grp)
    {
        writer.writeComment(QString("%1").arg(grp.nodeIndex()));
        writer.writeStartElement(XML_NODE_GROUP);
        writer.writeAttribute(XML_ATTR_UNK16, QString("0x%1").arg(grp.getUnk16(), 0, 16));
        for(const AnimSequenceReference * r : grp)
        {
            writer.writeComment(QString("%1").arg(r->nodeIndex()));
            writer.writeEmptyElement(XML_NODE_SEQUENCEREF);
            writer.writeAttribute(XML_ATTR_SEQ_REF, QString("%1").arg(r->getSeqRefID()));
        }
        writer.writeEndElement();
    }

    void WriteAnimTable(QXmlStreamWriter & writer)
    {
        writer.writeStartElement(XML_NODE_TABLE);
        for(const AnimTableSlot * g : m_spr.getAnimTable())
        {
            writer.writeComment(QString("%1").arg(g->nodeIndex()));
            fmt::animgrpid_t id = g->getGroupRef();
            if(id == fmt::NullGrpIndex)
                writer.writeEmptyElement(XML_NODE_GROUP);
            else
            {
                const AnimGroup *pgrp = m_spr.getAnimGroup(id);
                WriteAnimGroup(writer, *pgrp);
            }
        }
        writer.writeEndElement();
    }

private:
    const Sprite & m_spr;
};

//=================================================================================================================
//  Dump method
//=================================================================================================================
void Sprite::DumpSpriteToXML(const QString & fpath)
{
    if(canParse() && !wasParsed())
        ParseSpriteData();

    QSaveFile sf(fpath);
    if(sf.open(QIODevice::WriteOnly) && sf.error() == QFileDevice::NoError)
    {
        SpriteXMLWriter(*this).Write(sf);
        sf.commit();
    }
    else
    {
        throw BaseException(sf.errorString());
    }
}
