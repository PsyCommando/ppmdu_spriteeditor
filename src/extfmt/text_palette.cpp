#include "text_palette.hpp"
#include <sstream>
#include <iomanip>

std::vector<uint8_t> utils::ExportPaletteAsTextPalette(const std::vector<uint32_t> &srcargb, utils::funcoldec_t colhndlr)
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

    std::string stro(palcontent.str());
    return std::vector<uint8_t>( stro.begin(), stro.end() );
}

std::vector<uint32_t> utils::ImportPaletteAsTextPalette(const std::vector<uint8_t> &raw, utils::funcolenc_t colhndlr)
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
    return out;
}
