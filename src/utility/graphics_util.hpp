#ifndef GRAPHICS_UTIL_HPP
#define GRAPHICS_UTIL_HPP
#include <algorithm>
#include <QComboBox>
#include <QString>
#include <QImage>

//Shift the hue by the given amount, if overflows, the hue value just circle backs, and adds the difference to 0
int RotateHue(int hue, int shift);

class Sprite;
void FillComboBoxWithSpriteImages(const Sprite * spr, QComboBox & cmb);
void FillComboBoxWithSpritePalettes(const Sprite * spr, QComboBox & cmb, bool is256color);
void FillComboBoxWithFramePartPriorities(QComboBox & cmb);


extern const char * MinusOneImgRes;
//Returns the empty image for representing a minus one frame
const QImage & GetMinusOneImage();

#endif // GRAPHICS_UTIL_HPP
