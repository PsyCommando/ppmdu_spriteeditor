#ifndef FILE_FORMAT_IDENTIFIER_HPP
#define FILE_FORMAT_IDENTIFIER_HPP
#include <vector>
#include <cstdint>
#include <algorithm>
#include <src/ppmdu/fmts/sir0.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/compression_handler.hpp>

namespace fmt
{

    /*
     * Examine the content in the specified range to find if the data possibly matches the format of a wan/wat sprite
    */
    template<typename init>
        bool DataIsSprite(init itbeg, init itend)
    {
        using filetypes::eCompressionFormats;
        using filetypes::IndentifyCompression;

        //Check for compressed header
        auto itdatbeg = itbeg;
        auto itdatend = itend;
        std::vector<uint8_t> decompressed;
        decompressed.reserve(10000);
        eCompressionFormats fmt = filetypes::IndentifyCompression(itbeg, itend);
        if(fmt != eCompressionFormats::NONE && fmt != eCompressionFormats::INVALID)
        {
            filetypes::Decompress(itbeg, itend, std::back_inserter(decompressed));
            itdatbeg = decompressed.begin();
            itdatend = decompressed.end();
        }

        //Check if SIR0 header
        if(!SIR0hdr::isSIR0(itdatbeg, std::next(itdatbeg, SIR0_MagicNum.size())))
            return false;

        //Test SIR0 header
        SIR0hdr hdr;
        hdr.Read(itdatbeg, itdatend);
        auto itsubhdr = std::next(itdatbeg, hdr.ptrsub);

        //Check wan header
        auto itsubend = std::next(itdatbeg, hdr.ptrtranslatetbl);
        auto itpadd = std::next(itsubhdr, hdr_wan::LEN);
        bool hdrlenmatch = false;

        if(itpadd != itsubend)
        {
            //If the end of the wan header doesn't match the start of the pointer offset table,
            // its either padding, or the header is longer than the wan header
            uint32_t nextval = 0;
            utils::readBytesAs<uint32_t>(itpadd, itsubend, nextval);
            hdrlenmatch = 0xAAAAAAAA == nextval; //Only matches if the extra bytes are padding
        }
        else
            hdrlenmatch = true;

        if(!hdrlenmatch)
            return false;

        hdr_wan wanhdr;
        wanhdr.read(itsubhdr, itpadd);
        if(wanhdr.spritety >= static_cast<uint16_t>(eSpriteType::INVALID))
            return false;
        //Check header pointers
        uint32_t flen = std::distance(itdatbeg, itdatend);
        if(wanhdr.ptraniminfo > flen || wanhdr.ptrimgdatinfo > flen)
            return false;
        return true;
    }
};

#endif // FILE_FORMAT_IDENTIFIER_HPP
