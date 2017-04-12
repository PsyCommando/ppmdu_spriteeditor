#ifndef SIR0_HPP
#define SIR0_HPP
#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include <src/ppmdu/utils/byteutils.hpp>

namespace fmt
{
    extern const std::array<char,4> SIR0_MagicNum;

    /*
     * SIR0_Data
    */
    class SIR0_Data
    {
    public:
        static const size_t     HDRLEN = 16;
        std::array<char,4>      magic;
        uint32_t                ptrsub;
        uint32_t                ptrtranslatetbl;
        std::vector<uint32_t>   ptroffsetslist;

        template<class init>
            static bool IsSIR0(init where, init end)
        {
            assert(false);
            return false;
        }

        //
        template<class init>
            init Read( init where, init end )
        {
            init beg = where;
            where = utils::fillBytes( where, end, magic.begin(), magic.end() );
            where = utils::readBytesAs( where, end, ptrsub );
            where = utils::readBytesAs( where, end, ptrtranslatetbl );
            std::advance(where, sizeof(uint32_t)); //Skip over the empty space here

            //Read the Ptroffset list!
            assert(false);

            return where;
        }

        //
        template<class outit>
            outit Write( outit where )
        {
            outit cntbeg = where;
            where = utils::fillBytes( SIR0_MagicNum.begin(), SIR0_MagicNum.end(), where );
            where = utils::writeBytesFrom( where, ptrsub );
            where = utils::writeBytesFrom( where, ptrtranslatetbl );


            //Write list
            std::advance(cntbeg, ptrtranslatetbl ); //Skip over to the spot we gotta write the list at!
            cntbeg = WritePtrOffsetList(cntbeg);

            return where;
        }

    private:

        //
        template<class outit>
            outit WritePtrOffsetList( outit where )
        {
            assert(false);
            return where;
        }
    };

}

#endif // SIR0_HPP
