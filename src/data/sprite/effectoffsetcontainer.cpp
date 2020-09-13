#include "effectoffsetcontainer.hpp"
#include <src/data/sprite/sprite.hpp>

const char * ElemName_EffectOffset  = "Effect Offsets";

Sprite * EffectOffsetContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}
