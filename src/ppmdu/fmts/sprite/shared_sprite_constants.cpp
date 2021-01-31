#include "shared_sprite_constants.hpp"
#include <src/ppmdu/fmts/sprite/sprite_content.hpp>

namespace fmt
{
    const std::map<eSpriteTileMappingModes, std::string> SpriteMappingModeNames
    {
        {eSpriteTileMappingModes::Mapping1D, "1D"},
        {eSpriteTileMappingModes::Mapping2D, "2D"},
    };
};
