#ifndef PACKFILE_HPP
#define PACKFILE_HPP
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <cassert>
#include <mutex>
#include <src/ppmdu/utils/byteutils.hpp>

namespace fmt
{
    const std::string FExt_PACKFILE = "bin";

    constexpr uint8_t GenPadding()
    {
        return 0xFF;
    }

    /*
     * packfile_hdr
    */
    struct packfile_hdr
    {
        static const size_t SzFEntry = 8; //bytes (each toc entry is 8 bytes)
        struct fentry
        {
            uint32_t offset;
            uint32_t length;
        };
        std::vector<fentry> tocentries;
        //uint32_t            extrapaddinglen; //AKA the difference between the last entry in the toc and the first file in the data section!
    };


    /*
     * PackFileLoader
    */
    class PackFileLoader
    {
    public:

        template<class init>
            init Read( init beg, init end )
        {
            auto oldbeg = beg;
            //Read header
            uint32_t nullval   = utils::readBytesAs<uint32_t>( beg, end );
            uint32_t nbentries = utils::readBytesAs<uint32_t>( beg, end );
            if(nullval != 0)
            {
                //Error!
                throw std::runtime_error("PackFileLoader::Read(): Wrong file format! Doesn't begin with 4 bytes of 0!");
            }
            m_hdr.tocentries.resize(nbentries);

            //Read ToC
            for( auto & entry : m_hdr.tocentries )
            {
                entry.offset = utils::readBytesAs<uint32_t>(beg, end);
                entry.length = utils::readBytesAs<uint32_t>(beg, end);
                assert( entry.offset != 0 && entry.length != 0 );
            }

            //Allocate
            m_data.reserve(m_hdr.tocentries.back().offset + m_hdr.tocentries.back().length);

            //Load the entire file as data to simplify things
            std::copy( oldbeg, end, std::back_inserter(m_data) );
            return end;
        }

        template<class outit>
            outit CopyEntryData( size_t idx, outit dest )
        {
            //Get offset
            const packfile_hdr::fentry & entry = m_hdr.tocentries.at(idx);

            auto itcp = m_data.begin();
            std::advance(itcp, entry.offset);
            auto itend = itcp;
            std::advance(itend, entry.length);
            return std::copy( itcp, itend, dest );
        }

        inline size_t size()const { return m_hdr.tocentries.size(); }

        inline const packfile_hdr::fentry & GetEntryInfo(size_t idx)const { return m_hdr.tocentries.at(idx); }

    private:
        packfile_hdr         m_hdr;
        std::vector<uint8_t> m_data;
    };

    /*
     * PackFileWriter
     * Helper for writing an entire pack file to disk! Builds the final header and file structure as we add subfiles.
    */
    class PackFileWriter
    {
    public:
        static const uint32_t AlignHeaderOn = 256;  //bytes
        static const uint32_t AlignFilesOn  = 16;   //bytes

        template<class init>
            void AppendSubFile( init beg, init end )
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            auto itput = std::back_inserter(m_data);
            size_t sz = std::distance(beg, end);
            if( sz > std::numeric_limits<uint32_t>::max() )
                throw std::overflow_error("PackFileWriter::AppendSubFile(): File is longer than a 32 bit unsigned integer can contain!");

            packfile_hdr::fentry curentry{ static_cast<uint32_t>(m_data.size()), static_cast<uint32_t>(sz) };

            itput = std::copy( beg, end, itput );

            //Add a ToC entry for the file!
            m_hdr.tocentries.push_back(std::move(curentry));

            //Add padding
            utils::AppendPaddingBytes( itput, m_data.size(), AlignFilesOn, GenPadding());
        }

        template<class outit>
            size_t Write( outit where )
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            //Calculate Header + toc + padding len
            uint32_t hdrlen       = 8 + ((m_hdr.tocentries.size() + 1) * packfile_hdr::SzFEntry);
            uint32_t hdrandpadlen = utils::CalculatePaddedLengthTotal(hdrlen, AlignHeaderOn);

            //Write Leading 0
            where = utils::writeBytesFrom( static_cast<uint32_t>(0), where );

            //Write ToC size
            where = utils::writeBytesFrom( static_cast<uint32_t>(m_hdr.tocentries.size()), where );

            //Write ToC entries
            for( const packfile_hdr::fentry & entry : m_hdr.tocentries )
            {
                where = utils::writeBytesFrom(static_cast<uint32_t>(entry.offset + hdrandpadlen), where); //Add the estimated header len to the offset
                where = utils::writeBytesFrom(static_cast<uint32_t>(entry.length), where);
            }

            //Add null entry
            where = std::fill_n( where, packfile_hdr::SzFEntry, static_cast<uint8_t>(0) );

            //Add padding
            utils::AppendPaddingBytes( where, hdrlen, AlignHeaderOn, GenPadding());
//            where = std::generate_n(where, utils::CalculateLengthPadding(hdrlen, AlignHeaderOn), GenPadding);

            //Write the data blob!
            where = std::copy( m_data.begin(), m_data.end(), where );

            return hdrandpadlen + m_data.size();
        }

    private:
        std::mutex           m_mtx;
        packfile_hdr         m_hdr;
        std::vector<uint8_t> m_data;
    };

};
#endif // PACKFILE_HPP
