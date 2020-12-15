#ifndef TEXT_PALETTE_HPP
#define TEXT_PALETTE_HPP
/*
text_palette.hpp
description : Meant to export/import RGBA colors to ARGB hexadecimal color values inside a text file!
              This format is compatible with Paint.Net!
*/
#include <cstdint>
#include <vector>
#include <string>
#include <src/ppmdu/utils/color_utils.hpp>

namespace utils
{
    static const std::string TEXT_PAL_Filext = "txt";

    //Pass the color decoder that can decode the palette
    std::vector<uint8_t> ExportPaletteAsTextPalette( const std::vector<uint32_t> & srcargb,
                                                     funcoldec_t                   colhndlr );


    std::vector<uint32_t> ImportPaletteAsTextPalette( const std::vector<uint8_t> & raw,
                                                      funcolenc_t                  colhndlr);
};


#endif // TEXT_PALETTE_HPP
