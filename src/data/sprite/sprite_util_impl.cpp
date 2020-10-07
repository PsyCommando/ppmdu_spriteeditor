#include "sprite.hpp"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
#include <QComboBox>
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







