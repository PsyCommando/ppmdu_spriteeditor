#include "compression_handler.hpp"

namespace filetypes
{
    const std::vector<std::string> CompressionFormatsNames
    {
        "PKDPX",
        "AT4PX",
        "AT4PN",
        "INVALID",
        "NONE",
    };


    uint16_t DecompressPKDPX( std::vector<uint8_t>::const_iterator             itinputbeg,
                              std::vector<uint8_t>::const_iterator             itinputend,
                              std::back_insert_iterator<std::vector<uint8_t> > itoutwhere )
    {
        //Get header
        pkdpx_header myhdr;
        itinputbeg = myhdr.ReadFromContainer( itinputbeg, itinputend );
        compression::px_info_header pxinf = static_cast<compression::px_info_header>(myhdr);

        //1 - make buffer
        std::vector<uint8_t> buffer;
        buffer.reserve( pxinf.decompressedsz );

        //2 - decompress
        compression::DecompressPX( pxinf,
                                   itinputbeg,
                                   itinputend,
                                   buffer,
                                   false );

        //3 - copy buffer
        std::copy( buffer.begin(), buffer.end(), itoutwhere );

        return static_cast<uint16_t>(buffer.size());
    }
};
