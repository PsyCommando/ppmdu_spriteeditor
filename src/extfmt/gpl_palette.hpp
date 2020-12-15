#ifndef GPL_PALETTE_HPP
#define GPL_PALETTE_HPP
/*
gpl_palette.hpp
2017/06/05
psycommando@gmail.com
Description: Utilities for importing and exporting Gimp palette files!
             The format is relatively supported. Krita, GIMP and several open source softwares can handle it.
*/
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <src/ppmdu/utils/color_utils.hpp>


namespace utils
{
    static const std::string GPL_PAL_Filext       = "gpl";
    static const char        GPL_PAL_TblStartChar = '#';    //also comment character

    //Pass the color decoder for the color format of the palette!
    std::vector<uint8_t> ExportGimpPalette( const std::vector<uint32_t> & srcargb,
                                            funcoldec_t                   colhndlr,
                                            const std::string           & name = std::string(),
                                            int                           columns = 16 );

    //Pass the color encoder for the color format of the palette to encode!
    std::vector<uint32_t> ImportGimpPalette(const std::vector<uint8_t> & raw, funcolenc_t colhndlr);

};
#endif // GPL_PALETTE_HPP
