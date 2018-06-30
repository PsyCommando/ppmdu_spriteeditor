/*
*/
#include "riff_palette.hpp"
#include <array>
#include <algorithm>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <QDebug>

using namespace std;
using namespace utils;

/*
http://worms2d.info/Palette_file
As like every other RIFF file, the PAL files start off with the RIFF header:

    4-byte RIFF signature "RIFF"
    4-byte file length in bytes (excluding the RIFF header)
    4-byte PAL signature "PAL " (note the space / 0x20 at the end) 

The PAL files then include 4 different chunks, of which only the "data" chunk is important. The other chunks "offl", "tran" and "unde" are all 32 bytes long, filled with 0x00. Their purpose remains unknown since they can be deleted and the game still accepts the PAL files. Like other RIFF chunks, the data chunk starts with its signature and length:

    4-byte data chunk signature "data"
    4-byte data chunk size excluding the chunk header
    2-byte PAL version. This version is always 0x0300.
    2-byte color entry count. This determines how many colors are following. 

Each color consists of 4 bytes holding the following data:

    1-byte red amount of color
    1-byte green amount of color
    1-byte blue amount of color
    1-byte "flags" - W:A PAL files always have the 0x00 flag, so no flag is set. 
*/
namespace utils
{
//    using gimg::colorRGB24;

    // --------------- RIFF Specific constants --------------- 
    static const int  RIFF_MAGIC_NUMBER_LEN   = 4;
    static const int  RIFF_PAL_SIG_LEN        = 4;
    static const int  RIFF_DATA_CHUNK_SIG_LEN = 4;

    //Total Length of the RIFF header + the data chunk header. Not including the 2 bytes for the 
    // version or the 2 bytes for the nb of colors
    static const int  RIFF_PAL_HEADER_AND_DATA_CHUNK_HEADER_TOTAL_LEN = RIFF_MAGIC_NUMBER_LEN + 
                                                                        4 + 
                                                                        RIFF_PAL_SIG_LEN + 
                                                                        RIFF_DATA_CHUNK_SIG_LEN + 
                                                                        4;

    static const array<uint8_t,RIFF_MAGIC_NUMBER_LEN>   RIFF_MAGIC_NUMBER   = { 0x52, 0x49, 0x46, 0x46 }; // "RIFF"
    static const array<uint8_t,RIFF_PAL_SIG_LEN>        RIFF_PAL_SIG        = { 0x50, 0x41, 0x4C, 0x20 }; // "PAL "
    static const array<uint8_t,RIFF_DATA_CHUNK_SIG_LEN> RIFF_DATA_CHUNK_SIG = { 0x64, 0x61, 0x74, 0x61 }; // "data"
    static const uint16_t      RIFF_PAL_VERSION                             = 0x300;

//==================================================================
// RIFF Palette Tools
//==================================================================
    //Exports a palette to a Microsoft RIFF ".pal" file
    std::vector<uint8_t> ExportTo_RIFF_Palette(const std::vector<uint32_t> &in_palette, funcoldec_t decoder)
    {
        qDebug("ExportTo_RIFF_Palette\n");
        const uint16_t nbColorEntries   = in_palette.size();  //nb of colors in the palette
        const uint32_t dataChunkSize    = (nbColorEntries * 4) + 4;          //Length of the actual data / list of colors. Add 4 because the version number and nb of colors are counted as part of it, and are 2bytes each
        const uint32_t fileLengthTotal  = RIFF_PAL_HEADER_AND_DATA_CHUNK_HEADER_TOTAL_LEN + dataChunkSize; //Length of the riff file starting after the file length field
        const uint32_t fileLengthHeader = fileLengthTotal - 8; //remove the 8 bytes that arent't counted when calculating the file size

        //#1 - Resize the output vector
        std::vector<uint8_t> out_riffpalette;
        out_riffpalette.reserve( fileLengthTotal );
        auto itbackins = std::back_inserter( out_riffpalette );

        //#2 - Write the header
        {
            //Write "RIFF"
            std::copy_n( RIFF_MAGIC_NUMBER.begin(), RIFF_MAGIC_NUMBER.size(), itbackins );

            //Write 4 bytes File Length
            writeBytesFrom( fileLengthHeader, itbackins );

            //Write "PAL "
            std::copy_n( RIFF_PAL_SIG.begin(), RIFF_PAL_SIG.size(), itbackins );

            //Write "data"
            std::copy_n( RIFF_DATA_CHUNK_SIG.begin(), RIFF_DATA_CHUNK_SIG.size(), itbackins );

            //Write 4 bytes data chunk size
            writeBytesFrom( dataChunkSize, itbackins );

            //Write 2 bytes riff pal version number
            writeBytesFrom( RIFF_PAL_VERSION, itbackins );

            //Write 2 bytes nb of colors
            writeBytesFrom( nbColorEntries, itbackins );
        }

        qDebug("ExportTo_RIFF_Palette: Writing colors\n");
        //#3 - Write the colors into the output vector
        for( auto & col : in_palette )
        {
            utils::rgbaComponents cmp = decoder(col);
            out_riffpalette.push_back(cmp.r);
            out_riffpalette.push_back(cmp.g);
            out_riffpalette.push_back(cmp.b);
            out_riffpalette.push_back(cmp.a);
            //Write the colors ordered as RGB, Ignore the X from the palette as its ignored, and its always 0x80 anyways
//            out_riffpalette.push_back( static_cast<uint8_t>(col >> 24) );
//            out_riffpalette.push_back( static_cast<uint8_t>(col >> 16) );
//            out_riffpalette.push_back( static_cast<uint8_t>(col >>  8) );
//            out_riffpalette.push_back( static_cast<uint8_t>(col)       );
        }
        
        qDebug("ExportTo_RIFF_Palette: Moving vector\n");
        //#4 - Profits!
        return std::move(out_riffpalette);
    }

    void ExportTo_RIFF_Palette(const std::vector<uint32_t> &in_palette, const std::string & outputpath, funcoldec_t decoder )
    {
        vector<uint8_t> output;
        output = std::move(ExportTo_RIFF_Palette( in_palette, decoder ));
        WriteByteVectorToFile( outputpath, output );
    }

    //Import a Microsoft RIFF ".pal" file into a RGB24 color palette
    std::vector<uint32_t> ImportFrom_RIFF_Palette(const std::vector<uint8_t> & in_riffpalette , funcolenc_t encoder)
    {
        uint16_t nbcolors = 0;

        //#1 - Validate palette
        std::vector<uint32_t>           out_palette;
        vector<uint8_t>::const_iterator foundRiff = find_first_of( in_riffpalette.begin(), in_riffpalette.end(), RIFF_MAGIC_NUMBER.begin(), RIFF_MAGIC_NUMBER.end() );
        vector<uint8_t>::const_iterator foundPal;
        vector<uint8_t>::const_iterator foundDataChunk;

        if( foundRiff != in_riffpalette.end() )
        {
            foundPal = find_first_of( in_riffpalette.begin(), in_riffpalette.end(), RIFF_PAL_SIG.begin(), RIFF_PAL_SIG.end() );
            if( foundPal != in_riffpalette.end() )
            {
                foundDataChunk = std::find_first_of( in_riffpalette.begin(), in_riffpalette.end(), RIFF_DATA_CHUNK_SIG.begin(), RIFF_DATA_CHUNK_SIG.end() );
                if( foundDataChunk != in_riffpalette.end() )
                {
                    //#2 - move 10 bytes forward and read the nb of colors
                    foundDataChunk += 10;
                    nbcolors = readBytesAs<uint16_t>( foundDataChunk, in_riffpalette.end() );
                }
                else
                    throw runtime_error("Invalid RIFF palette!");
            }
            else
                throw runtime_error("Invalid RIFF palette!");
        }
        else
            throw runtime_error("Invalid RIFF palette!");

        //#3 - Populate the palette
        const unsigned int StopAt = (in_riffpalette.size()-3);
        for( unsigned int i = 0x18; i < StopAt; )
        {
            //uint32_t tmpcolor = 0;
            uint32_t r = 0, g = 0, b = 0, a = 0;
            //Everything in here is in RGB order, and there is the useless flag byte to skip at the end as well

            //Red
            r = (in_riffpalette[i] /*<< 24*/);

            //Green
            ++i;
            g = (in_riffpalette[i] /*<< 16*/);

            //Blue
            ++i;
            b = (in_riffpalette[i] /*<<  8*/);

            //Alpha
            ++i;
            a = in_riffpalette[i];

            //Push the color into the color palette
            out_palette.push_back(encoder(r,g,b,a));

            ++i;
        }
        return std::move( out_palette );
    }

    std::vector<uint32_t> ImportFrom_RIFF_Palette( const std::string & inputpath, funcolenc_t encoder )
    {
        vector<uint8_t>  riffpaldata(ReadFileToByteVector( inputpath ));
        vector<uint32_t> output     (ImportFrom_RIFF_Palette( riffpaldata, encoder ));
        return std::move(output);
    }

};
