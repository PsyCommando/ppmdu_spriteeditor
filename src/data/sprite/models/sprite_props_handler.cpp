#include "sprite_props_handler.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/sprite_props_delegate.hpp>

//=================================================================================================================
//  SpritePropertiesHandler
//=================================================================================================================
SpritePropertiesHandler::SpritePropertiesHandler(Sprite *owner, QObject *parent)
    :QObject(parent),
      m_powner(owner),
      m_pDelegate(new SpritePropertiesDelegate(m_powner, this)),
      m_pModel(new SpritePropertiesModel(m_powner, this))
{}

SpritePropertiesHandler::~SpritePropertiesHandler()
{
    qDebug("SpritePropertiesHandler::~SpritePropertiesHandler()\n");
}

void SpritePropertiesHandler::setOwner(Sprite *own)
{
    m_powner = own;
    m_pModel.reset(new SpritePropertiesModel(m_powner));
    m_pDelegate.reset(new SpritePropertiesDelegate(m_powner));
}

SpritePropertiesDelegate *SpritePropertiesHandler::delegate()
{
    return m_pDelegate.data();
}

SpritePropertiesModel *SpritePropertiesHandler::model()
{
    return m_pModel.data();
}

void SpritePropertiesHandler::setSpriteType(fmt::eSpriteType ty)
{
    //Convert Sprite Type as neccessary!
    qDebug("SpritePropertiesHandler::setSpriteType(): Sprite type changed to %d!", static_cast<int>(ty));
}
