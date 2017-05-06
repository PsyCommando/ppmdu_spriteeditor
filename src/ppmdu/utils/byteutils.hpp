#ifndef BYTEUTILS_HPP
#define BYTEUTILS_HPP
#include <cstdint>
#include <limits>
#include <cassert>

namespace utils
{

    /*********************************************************************************************
        readBytesAs
            Tool to read integer values from a byte vector!
            ** The iterator's passed as input, has its position changed !!
    *********************************************************************************************/
    template<class T, class _init>
        inline T readBytesAs( _init & itin, _init itend, bool basLittleEndian = true )
    {
        static_assert( std::numeric_limits<T>::is_integer, "readBytesAs() : Type T is not an integer!" );
        T out_val = 0;

        if( basLittleEndian )
        {
            unsigned int i = 0;
            for( ; (itin != itend) && (i < sizeof(T)); ++i, ++itin )
            {
                T tmp = (*itin);
                out_val |= ( tmp << (i * 8) ) & ( 0xFF << (i*8) );
            }

            if( i != sizeof(T) )
            {
                assert(false);
                throw std::runtime_error( "readBytesAs(): Not enough bytes to read from the source container!" );
            }
        }
        else
        {
            int i = (sizeof(T)-1);
            for( ; (itin != itend) && (i >= 0); --i, ++itin )
            {
                T tmp = (*itin);
                out_val |= ( tmp << (i * 8) ) & ( 0xFF << (i*8) );
            }

            if( i != -1 )
            {
                assert(false);
                throw std::runtime_error( "readBytesAs(): Not enough bytes to read from the source container!" );
            }
        }
        return out_val;
    }


    template<class Ty, class init>
        init readBytesAs( init where, init end, Ty & value, bool blittleendian = true )
    {
        value = readBytesAs<Ty>(where, end, blittleendian);
        return where;
    }


    /*********************************************************************************************
        writeBytesFrom
            Tool to write integer values into a byte vector!
            Returns the new pos of the iterator after the operation.
    *********************************************************************************************/
    template<class T, class _outit>
        inline _outit writeBytesFrom( T val, _outit itout, bool basLittleEndian = true )
    {
        static_assert( std::numeric_limits<T>::is_integer, "writeBytesFrom() : Type T is not an integer!" );

        if(basLittleEndian)
        {
            for(unsigned int i = 0; i < sizeof(T); ++i, ++itout)
            {
                T tempshift = 0;
                tempshift = ( val >> (i * 8) ) & 0xFF;
                (*itout) = tempshift & 0xFF;
            }
        }
        else
        {
            for(int i = (sizeof(T)-1); i >= 0 ; --i, ++itout)
            {
                T tempshift = 0;
                tempshift = ( val >> (i * 8) ) & 0xFF;
                (*itout) = tempshift & 0xFF;
            }
        }

        return itout;
    }


    /*
     * fillBytes
    */
    template<class init, class outit>
        init fillBytes( init where, init end, outit destbeg, outit destend )
    {
        //
        for( ;where != end && destbeg != destend; ++where, ++destbeg)
            *destbeg = *where;
        //
        return where;
    }

    /*
     *
    */
    template<class init, class outit>
        init fillBytes( init where, init end, outit destbeg )
    {
        //
        for( ;where != end; ++where, ++destbeg)
            *destbeg = *where;
        //
        return where;
    }


};

#endif // BYTEUTILS_HPP
