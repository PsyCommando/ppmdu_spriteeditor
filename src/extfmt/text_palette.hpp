#ifndef TEXT_PALETTE_HPP
#define TEXT_PALETTE_HPP
/*
text_palette.hpp
description : Meant to export/import RGBA colors to ARGB hexadecimal color values inside a text file!
              This format is compatible with Paint.Net!
*/
#include <cstdint>
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>
#include <src/ppmdu/utils/color_utils.hpp>

namespace utils
{
    static const std::string TEXT_PAL_Filext = "txt";



    //Pass the color decoder that can decode the palette
    std::vector<uint8_t> ExportPaletteAsTextPalette( const std::vector<uint32_t> & srcargb,
                                                     funcoldec_t                   colhndlr )
    {
        std::stringstream palcontent;
        palcontent <<std::hex <<std::uppercase;

        for( uint32_t col : srcargb )
        {
            rgbaComponents cmp = colhndlr(col);
            palcontent <<std::setfill('0')
                       <<std::setw(2) <<static_cast<int>(cmp.a)
                       <<std::setw(2) <<static_cast<int>(cmp.r)
                       <<std::setw(2) <<static_cast<int>(cmp.g)
                       <<std::setw(2) <<static_cast<int>(cmp.b)
                       <<"\n";
        }

        std::string       stro(palcontent.str());
        std::vector<uint8_t> out = std::vector<uint8_t>( stro.begin(), stro.end() );
        return std::move(out);
    }


    std::vector<uint32_t> ImportPaletteAsTextPalette( const std::vector<uint8_t> & raw,
                                                      funcolenc_t                  colhndlr)
    {
        std::vector<uint32_t> out;
        auto lambdaparse = [&colhndlr]( const std::string & str )
        {
            uint32_t rcol = std::stoul(str, 0, 16);
            uint32_t a = (rcol >> 24) & 0xFF,
                     r = (rcol >> 16) & 0xFF,
                     g = (rcol >>  8) & 0xFF,
                     b = rcol & 0xFF;
            return colhndlr(r,g,b,a);
        };

        auto itlast = raw.begin();
        auto itf    = std::find(raw.begin(), raw.end(), '\n');
        for( ; itf != raw.end() ; itf = std::find(itf, raw.end(), '\n') )
        {
            std::string str(itlast, itf);
            if(str.length() >= 2) //make sure we don't grab an empty line!
                out.push_back(lambdaparse(str));
            ++itf; //advance past the '\n'
            itlast = itf;
        }
        //Add the last entry if the file didn't end wu
        if( itlast != raw.end() )
        {
            std::string str(itlast, itf);
            if(str.length() >= 2) //make sure we don't grab an empty line!
                out.push_back(lambdaparse(str));
        }
        return std::move(out);
    }



};


#endif // TEXT_PALETTE_HPP
