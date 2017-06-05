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
#include <sstream>
#include <iomanip>
#include <algorithm>
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
                                            int                           columns = 16 )
    {
        std::stringstream sstr;

        //Header
        sstr << "GIMP Palette\n"
             << "Name: " << name <<"\n"
             << "Columns: " <<columns <<"\n"
             <<"#\n"
             <<std::setfill(' ');
        //Colors as RGB decimal
        for( uint32_t col : srcargb )
        {
            rgbaComponents rescol = colhndlr(col);
//            uint32_t r = (col >> 16) & 0xFF,
//                     g = (col >>  8) & 0xFF,
//                     b =  col & 0xFF;
            sstr <<std::setw(3) <<static_cast<int>(rescol.r)
                 <<" "
                 <<std::setw(3) <<static_cast<int>(rescol.g)
                 <<" "
                 <<std::setw(3) <<static_cast<int>(rescol.b)
                 <<"\n";
        }
        std::string res(sstr.str());
        return std::move( std::vector<uint8_t>(res.begin(), res.end()) );
    }

    //Pass the color encoder for the color format of the palette to encode!
    std::vector<uint32_t> ImportGimpPalette(const std::vector<uint8_t> & raw, funcolenc_t colhndlr)
    {
        std::vector<uint32_t> out;
        auto itbegcol = std::find(raw.begin(), raw.end(), GPL_PAL_TblStartChar);
        ++itbegcol; //go to next char

        //Skip all lines that begin with '#'!
        while( itbegcol != raw.end() )
        {
            ++itbegcol; //go to next char after the eol char
            if( itbegcol != raw.end() && *itbegcol == GPL_PAL_TblStartChar)
                itbegcol = std::find(itbegcol, raw.end(), '\n');
            else
                break;
        }

        //Define a lambda here to avoid repeating code!
        auto lambdacvstr = [&colhndlr](std::stringstream & parser)->uint32_t
        {
            uint32_t r = 0, g = 0, b = 0, a = 255;
            parser >> r >> g >> b;
//            uint32_t result = (a << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            return colhndlr(r, g, b, a);
        };

        //Here we either found the first line that doesn't start with #, or reached the end of the file!
        if(itbegcol != raw.end())
        {
            auto itlast = itbegcol;
            auto itf    = std::find(itbegcol, raw.end(), '\n');
            std::stringstream parser;

            for( ; itf != raw.end() ; itf = std::find(itf, raw.end(), '\n') )
            {
                std::string strparsed = std::string(itlast, itf);
                parser = std::move(std::stringstream(strparsed));
                out.push_back(lambdacvstr(parser));
                ++itf;
                itlast = itf;
            }

            //Parse the last entry not ending with eol, if neccessary!
            if(itlast != raw.end() && std::distance(itlast, itf) >= 5)
            {
                parser = std::move(std::stringstream(std::string(itlast, itf)));
                out.push_back(lambdacvstr(parser));
            }
        }
        return std::move(out);
    }

};
#endif // GPL_PALETTE_HPP
