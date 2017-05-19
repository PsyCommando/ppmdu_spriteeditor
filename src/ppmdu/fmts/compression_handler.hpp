#ifndef COMPRESSION_HANDLER_HPP
#define COMPRESSION_HANDLER_HPP
/*
compression_handler.hpp
2017/05/05
psycommando@gmail.com
Description: Handles both formats of compressed data used in pmd2. PKDPX and AT4PX. Eventually will also handle AT4PN.
*/
#include <cassert>
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/px_compression.hpp>

namespace filetypes
{
    static const size_t CompressionMagicNumberLen = 5;

    static const unsigned int                              MagicNumber_PKDPX_Len = CompressionMagicNumberLen;
    static const std::array<uint8_t,MagicNumber_PKDPX_Len> MagicNumber_PKDPX{{ 0x50, 0x4B, 0x44, 0x50, 0x58 }}; //"PKDPX"

    static const unsigned int                              MagicNumber_AT4PX_Len = CompressionMagicNumberLen;
    static const std::array<uint8_t,MagicNumber_AT4PX_Len> MagicNumber_AT4PX{{ 0x41, 0x54, 0x34, 0x50, 0x58 }};//"AT4PX"

    static const unsigned int                              MagicNumber_AT4PN_Len = CompressionMagicNumberLen;
    static const std::array<uint8_t,MagicNumber_AT4PN_Len> MagicNumber_AT4PN{{ 0x41, 0x54, 0x34, 0x50, 0x4E }};//"AT4PN"

    /*
    */
    enum struct eCompressionFormats
    {
        PKDPX,
        AT4PX,
        AT4PN,
        INVALID,
    };

    /*
     * Information on the compressed data
    */
    struct pxinfohdr
    {
        compression::px_info_header pxinfo; //
        eCompressionFormats         fmt;    //Header/compression format
    };


    /*
     * ExInvalidCompressionFormat
     *      Exception thrown when the compression format isn't recognized!
    */
    class ExInvalidCompressionFormat : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    template<class _init>
    eCompressionFormats IndentifyCompression(_init beg, _init end)
    {
        //#1. Fetch magic number
        std::vector<uint8_t> magic;
        auto binsmagic = std::back_inserter(magic);
        for( size_t cnt = 0; cnt < CompressionMagicNumberLen && beg != end; *binsmagic = *beg, ++cnt, ++beg );

        //#2. Identify
        eCompressionFormats fmt = eCompressionFormats::INVALID;
        if( std::equal(magic.begin(), magic.end(), MagicNumber_PKDPX.begin() ) ) //PKDPX
            fmt = eCompressionFormats::PKDPX;
        else if( std::equal(magic.begin(), magic.end(), MagicNumber_AT4PX.begin() ) ) //AT4PX
            fmt = eCompressionFormats::AT4PX;
        else if( std::equal(magic.begin(), magic.end(), MagicNumber_AT4PN.begin() ) ) //AT4PN
            fmt = eCompressionFormats::AT4PN;
        return fmt;
    }

    /*
     * Decompress
     *  Identifies and extract the compressed data from a known/supported compression format.
     *  Parameters:
     *      - pxinfparsed :(optional) pointer to a pxinfohdr struct where to copy the parsed details from the input data!
    */
    template<class _init, class _outit>
        _init Decompress( _init itsrcbeg, _init itsrcend, _outit itout, pxinfohdr * pxinfparsed = nullptr )
    {
        //#1. Fetch magic number
        //std::vector<uint8_t> magic;
        //auto binsmagic = std::back_inserter(magic);
        //for( size_t cnt = 0; cnt < CompressionMagicNumberLen && itsrcbeg != itsrcend; *binsmagic = *itsrcbeg, ++cnt, ++itsrcbeg );

        //#2. Identify
        eCompressionFormats fmt = IndentifyCompression(itsrcbeg, itsrcend);
        std::advance(itsrcbeg,CompressionMagicNumberLen);
//        if( std::equal(magic.begin(), magic.end(), MagicNumber_PKDPX.begin() ) ) //PKDPX
//            fmt = eCompressionFormats::PKDPX;
//        else if( std::equal(magic.begin(), magic.end(), MagicNumber_AT4PX.begin() ) ) //AT4PX
//            fmt = eCompressionFormats::AT4PX;
//        else if( std::equal(magic.begin(), magic.end(), MagicNumber_AT4PN.begin() ) ) //AT4PN
//            fmt = eCompressionFormats::AT4PN;

        if(fmt == eCompressionFormats::INVALID)
            throw ExInvalidCompressionFormat("Decompress(): Unsupported compression format!");

        //#3. Parse header
        pxinfohdr hdr;
        switch(fmt)
        {
            case eCompressionFormats::AT4PX:
            case eCompressionFormats::PKDPX:
            {
                hdr.pxinfo.compressedsz = utils::readBytesAs<uint16_t>(itsrcbeg, itsrcend);
                for( size_t cntfl = 0; (cntfl < compression::px_info_header::NB_FLAGS) && (itsrcbeg != itsrcend); (hdr.pxinfo.controlflags[cntfl] = *itsrcbeg), ++cntfl, ++itsrcbeg );

                if( fmt == eCompressionFormats::AT4PX )
                    hdr.pxinfo.decompressedsz = utils::readBytesAs<uint16_t>(itsrcbeg, itsrcend); //AT4PX decompressed size is 16 bits in its header
                else
                    hdr.pxinfo.decompressedsz = utils::readBytesAs<uint32_t>(itsrcbeg, itsrcend); //PKDPX decompressed size is 32 bits in its header
                break;
            }
        case eCompressionFormats::AT4PN:
            {
                hdr.pxinfo.compressedsz = utils::readBytesAs<uint16_t>(itsrcbeg, itsrcend);
                //no other known data!
                break;
            }
        };

        //#4. Copy header data if needed
        if(pxinfparsed)
            *pxinfparsed = hdr;

        //#5. Decompress the actual data
        if( fmt == eCompressionFormats::AT4PX || fmt == eCompressionFormats::PKDPX )
        {
            //We have to use a temporary buffer here, because of the way the algorithm works.
            //std::vector<uint8_t> buffer;
            compression::DecompressPX( hdr.pxinfo, itsrcbeg, itsrcend, itout );

            //itout = std::copy( buffer.begin(), buffer.end(), itout );
            std::advance( itsrcbeg, hdr.pxinfo.compressedsz );
        }
        else if(fmt == eCompressionFormats::AT4PN)
        {
            //unsupported right now!
            assert(false);
            throw std::runtime_error("Decompress(): AT4PN is currently unsuported!");
        }

        return itsrcbeg;
    }


    /*
    */
    template<class _init, class _outit>
        _outit CompressPX(eCompressionFormats fmt, _init itsrcbeg, _init itsrcend, _outit itout)
    {
        //#1. Compress
        std::vector<uint8_t> outbuffer;
        compression::px_info_header hdr = compression::CompressPX( itsrcbeg, itsrcend, std::back_inserter(outbuffer), compression::ePXCompLevel::LEVEL_3, true );

        //#2. Write header!
        std::array<uint8_t,CompressionMagicNumberLen> magic;

        if( fmt == eCompressionFormats::AT4PX )
            magic = MagicNumber_AT4PX;
        else
            magic = MagicNumber_PKDPX;

        itout = std::copy(magic.begin(), magic.end(), itout);
        itout = utils::writeBytesFrom(hdr.compressedsz, itout);
        itout = std::copy(hdr.controlflags.begin(), hdr.controlflags.end(), itout);


        if( fmt == eCompressionFormats::AT4PX )
            itout = utils::writeBytesFrom<uint16_t>(static_cast<uint16_t>(hdr.decompressedsz), itout); //For AT4PX the decompressed size is a 16 bits integer!
        else
            itout = utils::writeBytesFrom(hdr.decompressedsz, itout);

        //#3. Write compressed data!
        itout = std::copy( outbuffer.begin(), outbuffer.end(), itout );


        return itout;
    }

    /*
    */
    template<class _init, class _outit>
        _outit Compress( eCompressionFormats fmt, _init itsrcbeg, _init itsrcend, _outit itout )
    {

        switch(fmt)
        {
        case eCompressionFormats::AT4PX:
        case eCompressionFormats::PKDPX:
        {
            itout = CompressPX(fmt, itsrcbeg, itsrcend, itout);
            break;
        }
        case eCompressionFormats::AT4PN:
        {
            assert(false);
            break;
        }
        default:
            throw std::runtime_error("Compress(): Invalid compression format!!");
        };

        return itout;
    }

};
#endif // PXHANDLER_HPP
