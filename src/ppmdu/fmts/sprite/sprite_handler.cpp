#include "sprite_handler.hpp"

namespace fmt
{
    const uint8_t WA_SpriteHandler::PADDING_BYTE = 0xAA;

    void WA_SpriteHandler::setSpriteType(eSpriteType ty)
    {
        m_sprty = ty;
    }
};