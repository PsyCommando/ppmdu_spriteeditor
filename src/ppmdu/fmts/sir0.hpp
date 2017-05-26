#ifndef SIR0_HPP
#define SIR0_HPP
#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/integer_encoding.hpp>

namespace fmt
{
    extern const std::array<char,4> SIR0_MagicNum;
    extern const uint8_t            SIR0_PADDING;



//======================================================================================================================================
//  Functions for encoding pointer offset lists.
//======================================================================================================================================

    /*
        EncodeASIR0Offset

    */
    template<class _backinsoutit>
        inline _backinsoutit EncodeASIR0Offset( _backinsoutit itw, uint32_t offset )
    {
        bool hasHigherNonZero = false; //This tells the loop whether it needs to encode null bytes, if at least one higher byte was non-zero
        //Encode every bytes of the 4 bytes integer we have to
        for( int32_t i = sizeof(int32_t); i > 0; --i )
        {
            uint8_t currentbyte = ( offset >> (7 * (i - 1)) ) & 0x7Fu;
            if( i == 1 )                                            //the lowest byte to encode is special
                *(itw++) = currentbyte;               //If its the last byte to append, leave the highest bit to 0 !
            else if( currentbyte != 0 || hasHigherNonZero )         //if any bytes but the lowest one! If not null OR if we have encoded a higher non-null byte before!
            {
                *(itw++) = (currentbyte | 0x80u);       //Set the highest bit to 1, to signifie that the next byte must be appended
                hasHigherNonZero = true;
            }
        }
        return itw;
    }

    /*
        EncodeSIR0PtrOffsetList
    */
    template<class _backinsoutit, class _fwdinit>
        _backinsoutit EncodeSIR0PtrOffsetList( _fwdinit itbeg, _fwdinit itend, _backinsoutit itw, bool prefixheaderoffsets = true )
    {
        uint32_t lastoffset = 0; //used to add up the sum of all the offsets up to the current one

        //If asked put the offsets of the 2 pointers in the SIR0 header!
        if(prefixheaderoffsets)
        {
            *(itw++)    = 4;    //encoded offset of the ptr sub-header/content
            *(itw++)    = 4;    //encoded offset of the ptr encoded offset table
            lastoffset  = 8;
        }

        for( ; itbeg != itend; ++itbeg )
        {
            const auto & anoffset = *itbeg;
            itw                   = EncodeASIR0Offset( itw, (anoffset - lastoffset) );
            lastoffset            = anoffset; //set the value to the latest offset, so we can properly subtract it from the next offset.
        }
        //Append the closing 0
        *(itw++) = 0;
        return itw;
    }

    /*
        DecodeSIR0PtrOffsetList
    */
    template<class _init>
        std::vector<uint32_t> DecodeSIR0PtrOffsetList( _init beg, _init end )
    {
        using namespace std;
        vector<uint32_t> decodedptroffsets;

        auto itcurbyte  = beg;
        auto itlastbyte = end;

        uint32_t offsetsum = 0; //This is used to sum up all offsets and obtain the offset relative to the file, and not the last offset
        uint32_t buffer    = 0; //temp buffer to assemble longer offsets
        uint8_t curbyte    = *itcurbyte;
        bool    LastHadBitFlag = false; //This contains whether the byte read on the previous turn of the loop had the bit flag indicating to append the next byte!

        while( itcurbyte != itlastbyte && ( LastHadBitFlag || (*itcurbyte) != 0 ) )
        {
            curbyte = *itcurbyte;
            buffer |= curbyte & 0x7Fu;

            if( (0x80u & curbyte) != 0 )
            {
                LastHadBitFlag = true;
                buffer <<= 7u;
            }
            else
            {
                LastHadBitFlag = false;
                offsetsum += buffer;
                decodedptroffsets.push_back(offsetsum);
                buffer = 0;
            }

            ++itcurbyte;
        }

        return std::move(decodedptroffsets);
    }


    /*
     * SIR0hdr
    */
    template<class _outit> class SIR0_WriterHelper;
    struct SIR0hdr
    {
    public:
        static const size_t     HDRLEN = 16;
        std::array<char,4>      magic;
        uint32_t                ptrsub;
        uint32_t                ptrtranslatetbl;
        std::vector<uint32_t>   ptroffsetslist;

        SIR0hdr()
            :ptrtranslatetbl(0),ptrsub(0)
        {
        }


        template<class _init>
            bool isSIR0(_init where, _init end)
        {
            return std::equal(SIR0_MagicNum.begin(), SIR0_MagicNum.end(), where );
        }

        //
        template<class init>
            init Read( init where, init end, bool bdecodeptrlst = true )
        {
            init beg = where;
            where = utils::fillBytes( where, end, magic.begin(), magic.end() );
            where = utils::readBytesAs( where, end, ptrsub );
            where = utils::readBytesAs( where, end, ptrtranslatetbl );
            std::advance(where, sizeof(uint32_t)); //Skip over the empty space here
            init afterhdr = where;

            //decode pointers
            if(bdecodeptrlst)
                ptroffsetslist = std::move(DecodeSIR0PtrOffsetList(std::next(beg, ptrtranslatetbl), end));
            return afterhdr;
        }

        /*
         *  Write
         *  - where    : Where to write the SIR0 header + ptr list and data!
         *  - itdatabeg: Beginning of the data to be wrapped in the SIR0.
         *  - itdataend: End of the data to be wrapped in the SIR0.
        */
        template<class outit, class datainit>
            outit Write( outit where, datainit itdatabeg, datainit itdataend, uint8_t padchar = SIR0_PADDING )
        {
            where = std::copy( SIR0_MagicNum.begin(), SIR0_MagicNum.end(), where );
            where = utils::writeBytesFrom( ptrsub, where);
            where = utils::writeBytesFrom( ptrtranslatetbl , where);
            where = utils::writeBytesFrom<uint32_t>( 0u, where); //put ending 0!

            //Copy and count data!
            size_t bytecnt = HDRLEN;
            for( ; itdatabeg != itdataend; ++ itdatabeg, ++where, ++bytecnt )
                (*where) = (*itdatabeg);

            //Add padding before the ptr offset list
            bytecnt += utils::AppendPaddingBytes( where, bytecnt, 16, padchar );

            //Write list
            return EncodeSIR0PtrOffsetList( ptroffsetslist.begin(), ptroffsetslist.end(), where );
        }

    private:

    };



    /*
        Helper class meant to handle the SIR0 pointer marking as seamlessly as possible.
        Just instantiate and use its functions to write to the target!
    */
    template<class _outit> class SIR0_WriterHelper
    {
    public:
        typedef _outit iterout_t;

        SIR0_WriterHelper(_outit itout, SIR0hdr  &hdr)
            :m_curoffset(SIR0hdr::HDRLEN), m_out(itout), m_sir0(hdr)
        {
        }

        template<class T>
            inline uint32_t writeVal(T val, bool blittleendian = true)
        {
            m_curoffset += sizeof(T);
            m_out = utils::writeBytesFrom(val, m_out, blittleendian);
            return getCurOffset();
        }

        inline uint32_t writePtr(uint32_t val)
        {
            //Skip null pointers
            if(val != 0)
                m_sir0.ptroffsetslist.push_back(m_curoffset);
            writeVal(val);
            return getCurOffset();
        }

        inline void putPadding(uint32_t alignon, const uint8_t padbyte = 0)
        {
            m_curoffset += utils::AppendPaddingBytes(m_out, m_curoffset, alignon, padbyte);
        }

        inline uint32_t getCurOffset()const {return m_curoffset;}
        inline SIR0hdr & getSIR0Cnt() {return m_sir0;}

    private:
        iterout_t m_out;
        SIR0hdr  &m_sir0;
        uint32_t  m_curoffset;
    };

}

#endif // SIR0_HPP
