#ifndef BYTEUTILS_HPP
#define BYTEUTILS_HPP
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cassert>
#include <cmath>

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


    /*********************************************************************************************
        IsolateBits
            Isolate some adjacent bits inside a value of type T,
            and shift them back to offset 0.

            * nbbits    : Nb of bits to isolate. Used to make the bitmask.
            * bitoffset : Offset of the bits from the right to the left. From the last bit.

            Ex1 : we want those bits 0000 1110, the params are ( 0xE, 3, 1 ),  returns 0000 0111
            Ex2 : we want those bits 0011 0000, the params are ( 0x30, 2, 4 ), returns 0000 0011
    *********************************************************************************************/
    template<class T>
        inline T IsolateBits( T src, unsigned int nbBits, unsigned int bitoffset )
    {
        static_assert( std::is_pod<T>::value, "IsolateBits(): Tried to isolate bits of a non-POD type!!" );
        T mask = static_cast<T>( ( pow( 2, nbBits ) - 1u ) ); //Subtact one to get the correct mask
        return ( ( src & (mask << bitoffset) ) >> bitoffset );
    }



    /*********************************************************************************************
        CalculateLengthPadding
            Calculates the length of padding required to align an address/length
            on the given value!

            Parameters:
                - length : The length to align.
                - alignon: The number by which the length should be divisible by.

            Return:
                The number of padding bytes necessary to align the given length on
                "alignon".
    *********************************************************************************************/
    template<typename _SizeTy>
        inline _SizeTy CalculateLengthPadding( _SizeTy length, _SizeTy alignon )
    {
        return ( (length % alignon) != 0 )? (alignon - (length % alignon)) : 0;
    }

    /*********************************************************************************************
        CalculatePaddedLengthTotal
            Calculate the nb of padding bytes needed to align a given length on
            the divisor specified, and add those to the length specified. Returning
            the total amount of bytes after adding the padding bytes.

            Parameters:
                - length : The length to align.
                - alignon: The number by which the length should be divisible by.

            Return:
                The total amount of bytes after adding the padding bytes.
    *********************************************************************************************/
    template<typename _SizeTy>
        inline _SizeTy CalculatePaddedLengthTotal( _SizeTy length, _SizeTy alignon )
    {
        return CalculateLengthPadding(length, alignon) + length;
    }

    /*********************************************************************************************
        AppendPaddingBytes
            This function takes a back insert iterator and the length of the container to append padding
            to, along with the length to align on to determine how much padding is needed.

            Return the nb of padding bytes that were inserted!
    *********************************************************************************************/
    template<class _backinit>
        size_t AppendPaddingBytes( _backinit itinsertat, size_t lentoalign, size_t alignon, const uint8_t PadByte = 0 )
    {
        size_t lenpadding = CalculateLengthPadding(lentoalign, alignon);
        for( size_t ctpad = 0; ctpad < lenpadding; ++ctpad, ++itinsertat )
            itinsertat = PadByte;
        return lenpadding;
    }

};

#endif // BYTEUTILS_HPP
