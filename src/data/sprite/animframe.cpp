#include "animframe.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequence.hpp>

const QString ElemName_AnimFrame = "Animation Frame";

//***********************************************************************************
//  AnimFrame
//***********************************************************************************
AnimFrame::AnimFrame(TreeNode *parent)
    :parent_t(parent)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

AnimFrame::AnimFrame(const AnimFrame &cp)
    :parent_t(cp)
{
    m_data = cp.m_data;
}

AnimFrame::AnimFrame(AnimFrame &&mv)
    : parent_t(mv)
{
    m_data = qMove(mv.m_data);
}

AnimFrame &AnimFrame::operator=(AnimFrame &&mv)
{
    m_data = qMove(mv.m_data);
    parent_t::operator=(mv);
    return *this;
}

AnimFrame &AnimFrame::operator=(const AnimFrame &cp)
{
    m_data = cp.m_data;
    parent_t::operator=(cp);
    return *this;
}

AnimFrame::~AnimFrame()
{
}

eTreeElemDataType AnimFrame::nodeDataTy() const
{
    return eTreeElemDataType::animFrame;
}

const QString &AnimFrame::nodeDataTypeName() const
{
    return ElemName_AnimFrame;
}

TreeNode *AnimFrame::clone() const
{
    return new AnimFrame(*this);
}

bool AnimFrame::operator==(const AnimFrame &other) const
{
    return this == &other;
}

bool AnimFrame::operator!=(const AnimFrame &other) const
{
    return !operator==(other);
}

QImage AnimFrame::makePreview(const Sprite * owner) const
{
    const MFrame * pframe = owner->getFrame(frmidx());
    if(pframe)
        return pframe->AssembleFrame(0,0, QRect(), nullptr, true, owner);
    return QImage();
}
