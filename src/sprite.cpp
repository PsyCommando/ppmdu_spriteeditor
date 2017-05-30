#include "sprite.h"
#include <QGraphicsScene>
#include <QBitmap>
#include <QPainter>
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
const char * ElemName_AnimFrame     = "Anim Frame";
const char * ElemName_FramePart     = "Frame Part";


//
//  Sprite
//
Sprite * EffectOffsetContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}



//
//  SpritePropertiesHandler
//
SpritePropertiesHandler::SpritePropertiesHandler(Sprite *owner, QObject *parent)
    :QObject(parent), m_powner(owner)
{}

SpritePropertiesHandler::~SpritePropertiesHandler()
{
    qDebug("SpritePropertiesHandler::~SpritePropertiesHandler()\n");
}

void SpritePropertiesHandler::sendSpriteLoaded()
{
    emit spriteLoaded();
}

void SpritePropertiesHandler::setOwner(Sprite *own)
{
    m_powner = own;
}

void SpritePropertiesHandler::setSpriteType(fmt::eSpriteType ty)
{
    //Convert Sprite Type as neccessary!
    qDebug("SpritePropertiesHandler::setSpriteType(): Sprite type changed to %d!", static_cast<int>(ty));
}
