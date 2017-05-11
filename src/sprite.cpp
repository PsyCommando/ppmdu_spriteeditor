#include "sprite.h"
#include <src/ppmdu/fmts/wa_sprite.hpp>

const char * ElemName_EffectOffset  = "Effect Offsets";
const char * ElemName_Palette       = "Palette";
const char * ElemName_Images        = "Images";
const char * ElemName_Image         = "Image";
const char * ElemName_FrameCnt      = "Frames";
const char * ElemName_Frame         = "Frame";
const char * ElemName_AnimSequence  = "Anim Sequence";
const char * ElemName_AnimSequences = "Anim Sequences";
const char * ElemName_AnimTable     = "Animation Table";
const char * ElemName_AnimGroup     = "Anim Group";


//const QString EffectOffsetContainer::ElemName = "Effect Offsets";
//const QString PaletteContainer::ElemName = "Palette";
//const QString ImageContainer::ElemName = "Images";
//const QString MFrame::ElemName = "Frame";
//const QString FramesContainer::ElemName = "Frames";
//const QString AnimSequence::ElemName = "Seq";
//const QString AnimSequences::ElemName = "Sequences";
//const QString AnimTable::ElemName = "Anim. Table";

//unsigned long long Sprite::spriteCnt = 0;
//QStack<unsigned long long> Sprite::spriteIDRecycler;

Sprite * EffectOffsetContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *AnimTable::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *AnimGroup::parentSprite()
{
    static_cast<AnimTable*>(parent())->parentSprite();
}

Sprite *AnimSequences::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *AnimSequence::parentSprite()
{
    static_cast<AnimSequences*>(parent())->parentSprite();
}

Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *MFrame::parentSprite()
{
    return static_cast<FramesContainer*>(parent())->parentSprite();
}

Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parent())->parentSprite();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parent());
}

QVariant Image::imgData(int column, int role)
{
    QVariant res;
    switch(column)
    {
    case 0: //preview
        if( role == Qt::DecorationRole )
            res.setValue(makePixmap(parentSprite()->getPalette()));
        else if( role == Qt::SizeHintRole )
            res.setValue( QSize(m_img.size().width() *2, m_img.size().height() *2) );
        break;
    case 1: //depth
        res.setValue(QString("%1bpp").arg(m_depth));
        break;
    case 2: //resolution
        res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
    };
    return std::move(res);
}
