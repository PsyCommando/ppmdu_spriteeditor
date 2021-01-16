#ifndef SHARED_SPRITE_CONSTANTS_HPP
#define SHARED_SPRITE_CONSTANTS_HPP
#include <vector>
#include <array>

namespace fmt
{
    //Palette colors
    typedef uint32_t            rgbx_t;        //Data type for storing a color's 4 bytes data under the format RRGGBBXX (XX is unused)
    typedef int16_t             frmid_t;       //-1 is a nodraw frame, so must be signed
    typedef std::vector<rgbx_t> rbgx24pal_t;   //RGBX 24bits palette type

    extern const int SPR_PAL_NB_COLORS_SUBPAL;      //Amount of colors per sub-palette when in 4bpp mode

    //Frame resolutions
    extern const std::array<std::pair<uint16_t,uint16_t>, 12> FrameResValues;

    //Maximum possible size for a non-double size frame
    extern const std::pair<uint16_t, uint16_t> MaxFrameResValue;

    //Possible resolutions for individual parts of an assembled frame.
    //The value of the bits matches the 2 first bits of attr1 and attr2 respectively for representing the resolution!
    //SO PLEASE DO NOT REORDER OR OR ADD TO THIS TABLE!
    enum struct eFrameRes : uint16_t
    {
        Square_8x8   = 0,
        Square_16x16 = 1,
        Square_32x32 = 2,
        Square_64x64 = 3,

        Wider_16x8   = 4,
        Wider_32x8   = 5,
        Wider_32x16  = 6,
        Wider_64x32  = 7,

        Higher_8x16  = 8,
        Higher_8x32  = 9,
        Higher_16x32 = 10,
        Higher_32x64 = 11,
        Invalid,
    };

    //Sprite formats variants
    enum struct eSpriteType : uint16_t
    {
        Prop        = 0,
        Character   = 1,
        Effect      = 2,
        WAT         = 3,
        INVALID     = 4,
    };

    //Human readable names for the sprite types
    extern const std::vector<std::string> SpriteTypeNames;

    //Tiles length
    extern const int NDS_TILE_SIZE_4BPP; //Tiles in memory for the NDS are 8x8 32 bytes for 4bpp, and 64 bytes 8x8 for 8bpp
    extern const int NDS_TILE_SIZE_8BPP; //Tiles in memory for the NDS are 8x8 32 bytes for 4bpp, and 64 bytes 8x8 for 8bpp

    extern const int NDS_OAM_MAX_NB_TILES; //The number of available tiles for OAM objects
    extern const int NDS_OAM_NB_PIXELS_TILES; //The number of pixels per tile

    extern const int NB_PIXELS_WAN_TILES; //The number of pixels per "wan" tile. The tiles as indicated in the wan file are calculated on 16x16 pixels tiles, intead of 8x8 for some reasons
};

#endif // SHARED_SPRITE_CONSTANTS_HPP
