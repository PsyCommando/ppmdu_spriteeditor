#ifndef RIFF_PALETTE_HPP
#define RIFF_PALETTE_HPP
/*
riff_palette.hpp
2014/10/10
psycommando@gmail.com
Description: Utilities for importing and exporting RIFF color palettes.
             The format is relatively supported. Krita can handle it.
*/
#include <cstdint>
#include <vector>
#include <string>
#include <src/ppmdu/utils/color_utils.hpp>

namespace utils
{
    static const std::string RIFF_PAL_Filext = "pal";

    //==============================================================================
    //  RGBA32
    //==============================================================================
    //The format for the raw color palettes is expected to be RGBA32 big endian.

    //Import
    std::vector<uint32_t> ImportFrom_RIFF_Palette( const std::vector<uint8_t> & in_riffpalette, funcolenc_t encoder );
    std::vector<uint32_t> ImportFrom_RIFF_Palette(const std::string          & inputpath , funcolenc_t encoder);

    //Export
    std::vector<uint8_t> ExportTo_RIFF_Palette( const std::vector<uint32_t> & in_palette, funcoldec_t decoder);
    void ExportTo_RIFF_Palette(const std::vector<uint32_t> & in_palette, const std::string    & outputpath , funcoldec_t decoder);
};

#endif
