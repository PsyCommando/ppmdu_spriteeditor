#ifndef GRAPHICS_UTIL_HPP
#define GRAPHICS_UTIL_HPP
#include <algorithm>

//Shift the hue by the given amount, if overflows, the hue value just circle backs, and adds the difference to 0
int RotateHue(int hue, int shift)
{
    hue = std::clamp(hue, 0, 255);
    shift = std::clamp(shift, -255, 255);

    if(hue + shift > 255)
        return (hue + shift) - 255;
    if(hue + shift < 0)
        return (hue + shift) - 255;
    return hue + shift;
}

#endif // GRAPHICS_UTIL_HPP
