#include "wa_sprite.hpp"
#include <src/ppmdu/fmts/sprite/sprite_content.hpp>

namespace fmt
{
     const int SPR_PAL_NB_COLORS_SUBPAL = 16;

    const std::pair<uint16_t, uint16_t> MaxFrameResValue { 64, 64 };

    const std::array<std::pair<uint16_t,uint16_t>, 12> FrameResValues
    {{
        { 8, 8},    //00 00
        {16,16},    //00 01
        {32,32},    //00 10
        {64,64},    //00 11

        {16, 8},    //01 00
        {32, 8},    //01 01
        {32,16},    //01 10
        {64,32},    //01 11

        { 8,16},    //10 00
        { 8,32},    //10 01
        {16,32},    //10 10
        {32,64},    //10 11
    }};


    const std::vector<std::string> SpriteTypeNames
    {
        "Props",
        "Characters",
        "Effects",
        "Others/WAT",

        "INVALID",
    };

    const animseqid_t NullSeqIndex = -1;
    const animgrpid_t NullGrpIndex = -1;

    //Tiles length
    const int NDS_TILE_SIZE_4BPP = 32; //Tiles in memory for the NDS are 8x8 32 bytes for 4bpp, and 64 bytes 8x8 for 8bpp
    const int NDS_TILE_SIZE_8BPP = 64; //Tiles in memory for the NDS are 8x8 32 bytes for 4bpp, and 64 bytes 8x8 for 8bpp

};
