#include "animframe.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequence.hpp>

const QString ElemName_AnimFrame = "Anim Frame";

//***********************************************************************************
//  AnimFrame
//***********************************************************************************
QImage AnimFrame::makePreview(const Sprite * owner) const
{
    //#TODO: split this from ui code!
    const MFrame * pframe = owner->getFrame(frmidx());
    if(pframe)
        return pframe->AssembleFrame(0,0, QRect(), nullptr, true, owner);
    return QImage();
}
