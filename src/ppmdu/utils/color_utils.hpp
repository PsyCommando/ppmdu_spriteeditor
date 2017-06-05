#ifndef COLOR_UTILS_HPP
#define COLOR_UTILS_HPP

#include <cstdint>
#include <functional>

namespace utils
{
    struct rgbaComponents
    {
        uint8_t r,g,b,a;
    };

    //Convenience function types for color encoder and decoders!
    typedef std::function<rgbaComponents(uint32_t)>                      funcoldec_t;
    typedef std::function<uint32_t(uint32_t,uint32_t,uint32_t,uint32_t)> funcolenc_t;


//
//  ENCODERS
//

    //Encodes a 32 bits integer as a ARGB color entry!
    inline uint32_t RGBToARGB( uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255 )
    {
        return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
    }

    //Encodes a 32 bits integer as a RGBA color entry!
    inline uint32_t RGBToRGBA( uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255 )
    {
        return ((r & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (a & 0xFF);
    }


//
//  DECODERS
//

    //Decode a ARGB 32 bits color to its components
    inline rgbaComponents ARGBToComponents(uint32_t argb)
    {
        return rgbaComponents{ 0xFF & (argb >> 16), //red
                               0xFF & (argb >>  8), //green
                               0xFF &  argb,        //blue
                               0xFF & (argb >> 24), //alpha
                             };
    }

    //Decode a RGBA 32 bits color to its components
    inline rgbaComponents RGBAToComponents(uint32_t rgba)
    {
        return rgbaComponents{ 0xFF & (rgba >> 24), //red
                               0xFF & (rgba >> 16), //green
                               0xFF & (rgba >>  8), //blue
                               0xFF &  rgba,        //alpha
                             };
    }
};

#endif // COLOR_UTILS_HPP
