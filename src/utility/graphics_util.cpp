#include "graphics_util.hpp"
#include <src/data/sprite/sprite.hpp>

const char * MinusOneImgRes     {":/resources/imgs/minus_one.png"};
const char * MFramePart_TileRef {"ID:-1 Tile Reference"};

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

void FillComboBoxWithSpriteImages(const Sprite * spr, QComboBox & cmb)
{
    cmb.clear();
    const ImageContainer * pcnt = &(spr->getImages());
    cmb.setIconSize( QSize(32,32) );
    cmb.setStyleSheet(pcnt->ComboBoxStyleSheet());

    //Add tile reference ID
    cmb.addItem(QPixmap::fromImage(GetMinusOneImage()), MFramePart_TileRef, QVariant(-1)); //Set user data to -1
    //Add actual images!
    for( int cntimg = 0; cntimg < pcnt->nodeChildCount(); ++cntimg )
    {
        const Image* pimg = pcnt->getImage(cntimg);
        QPixmap pmap = QPixmap::fromImage(pimg->makeImage(spr->getPalette()));
        QString text = pimg->getImageDescription();
        cmb.addItem( QIcon(pmap), text, QVariant(pimg->getID()) ); //Set user data to image's UID
    }
    cmb.setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

void FillComboBoxWithSpritePalettes(const Sprite * spr, QComboBox & cmb, bool is256color)
{
    cmb.clear();
    if(is256color)
    {
        //In 256 mode we don't get to pick a palette other than 0
        cmb.setIconSize(QSize(128,64));
        cmb.setEnabled(false);
        cmb.addItem(spr->MakePreviewPalette(), "Palette #0", QVariant(0));
    }
    else
    {
        //In 16 color/16 palettes mode we get to pick a palette
        cmb.setIconSize(QSize(128,16));
        for(int i = 0; i < fmt::step_t::MAX_NB_PAL; ++i)
            cmb.addItem(spr->MakePreviewSubPalette(i), QString("Palette #%1").arg(i), QVariant(i));
    }
    cmb.setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

void FillComboBoxWithFramePartPriorities(QComboBox & cmb)
{
    cmb.clear();
    cmb.setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    cmb.addItems(FRAME_PART_PRIORITY_NAMES);
}

const QImage &GetMinusOneImage()
{
    static QImage minusOne(MinusOneImgRes);
    return minusOne;
}
