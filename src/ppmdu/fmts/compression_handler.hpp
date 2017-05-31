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

    extern const std::vector<std::string> CompressionFormatsNames;

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



    /*
         pkdpx_header
             Structure of the header for an PKDPX file
     */
     struct pkdpx_header
     {
         static const unsigned int HEADER_SZ        = 20u;
         static const unsigned int NB_FLAGS         = 9u;
         //static const unsigned int MAGIC_NUMBER_LEN = MagicNumber_PKDPX_Len;

         std::array<uint8_t,MagicNumber_PKDPX_Len>  magicn;       //"PKDPX"
         uint16_t                              compressedsz; //The total size of the file compressed
         std::array<uint8_t,NB_FLAGS>          flaglist;     //list of flags used in the file
         uint32_t                              decompsz;     //The file size decompressed

         constexpr unsigned int size()const {return HEADER_SZ;}

         //Implementations specific to pkdpx_header
         template<class _outit>
             _outit WriteToContainer( _outit itwriteto )const
         {
             for( const uint8_t & abyte : MagicNumber_PKDPX )
             {
                 *itwriteto = abyte;
                 ++itwriteto;
             }

             itwriteto = utils::writeBytesFrom( compressedsz, itwriteto );

             for( const uint8_t & aflag : flaglist )
             {
                 *itwriteto = aflag;
                 ++itwriteto;
             }

             itwriteto = utils::writeBytesFrom( decompsz, itwriteto );

             return itwriteto;
         }

         template<class _init>
             _init ReadFromContainer( _init itReadfrom, _init itPastEnd )
         {
             for( uint8_t & abyte : magicn )
             {
                 abyte = utils::readBytesAs<uint8_t>(itReadfrom,itPastEnd);
                 //abyte = *itReadfrom;
                 //++itReadfrom;
             }

             compressedsz = utils::readBytesAs<decltype(compressedsz)>(itReadfrom,itPastEnd); //iterator is incremented

             for( uint8_t & aflag : flaglist )
             {
                 aflag = *itReadfrom;
                 ++itReadfrom;
             }

             decompsz = utils::readBytesAs<decltype(decompsz)>(itReadfrom,itPastEnd); //iterator is incremented

             return itReadfrom;
         }

         operator compression::px_info_header()const
         {
             compression::px_info_header pxinf;
             pxinf.compressedsz   = compressedsz;
             pxinf.controlflags   = flaglist;
             pxinf.decompressedsz = decompsz;
             return pxinf;
         }

         pkdpx_header & operator=( const compression::px_info_header & other )
         {
             compressedsz = other.compressedsz;
             flaglist     = other.controlflags;
             decompsz     = other.decompressedsz;
             return *this;
         }
     };


//
//
//

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
            std::vector<uint8_t> buffer;
            //#FIXME: huge wast of time here!!!!! The px functions only handle std::vector..
            std::vector<uint8_t> crutch(itsrcbeg, itsrcend); //Have to do this for now, until I rewrite the px utilities..
            compression::DecompressPX( hdr.pxinfo, crutch.begin(), crutch.end(), buffer);

            itout = std::copy( buffer.begin(), buffer.end(), itout );
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
        //#FIXME: huge wast of time here!!!!! The px functions only handle std::vector..
        std::vector<uint8_t> crutch(itsrcbeg, itsrcend); //Have to do this for now, until I rewrite the px utilities..

        compression::px_info_header hdr = compression::CompressPX( crutch.begin(), crutch.end(), std::back_inserter(outbuffer), compression::ePXCompLevel::LEVEL_3, true );

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


    /*******************************************************
        DecompressPKDPX
            Decompress an PKDPX file.
            Returns the size of the decompressed data!

            Params:
                - itinputbeg : beginning of the PX compressed data, right BEFORE the PKDPX header!

            NOTE:
            This version needs to create an internal
            buffer because of the way compression works.

            Use the version taking 2 output iterators or
            the one with the vector instead to avoid this!
    *******************************************************/
    uint16_t DecompressPKDPX( std::vector<uint8_t>::const_iterator             itinputbeg,
                              std::vector<uint8_t>::const_iterator             itinputend,
                              std::back_insert_iterator<std::vector<uint8_t> > itoutwhere );


//    uint16_t DecompressPKDPX( std::vector<uint8_t>::const_iterator             itinputbeg,
//                              std::vector<uint8_t>::const_iterator             itinputend,
//                              std::back_insert_iterator<std::vector<uint8_t> > itoutwhere )
//    {
//        //Get header
//        pkdpx_header myhdr;
//        itinputbeg = myhdr.ReadFromContainer( itinputbeg, itinputend );
//        compression::px_info_header pxinf = static_cast<compression::px_info_header>(myhdr);

//        //1 - make buffer
//        std::vector<uint8_t> buffer;
//        buffer.reserve( pxinf.decompressedsz );

//        //2 - decompress
//        compression::DecompressPX( pxinf,
//                                   itinputbeg,
//                                   itinputend,
//                                   buffer,
//                                   false );

//        //3 - copy buffer
//        std::copy( buffer.begin(), buffer.end(), itoutwhere );

//        return static_cast<uint16_t>(buffer.size());
//    }


};
#endif // PXHANDLER_HPP
