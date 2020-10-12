#include "palettecontainer.hpp"
#include <src/data/sprite/sprite.hpp>

const QString ElemName_Palette = "Palette";

//============================================================================================
//  PaletteContainer
//============================================================================================
PaletteContainer::PaletteContainer(TreeNode *parent)
    :TreeNodeTerminal(parent)
{
}

PaletteContainer::PaletteContainer(PaletteContainer &&mv)
    :TreeNodeTerminal(mv)
{
    operator=(qMove(mv));
}

PaletteContainer::PaletteContainer(const PaletteContainer &cp)
    :TreeNodeTerminal(cp)
{
    operator=(cp);
}

PaletteContainer &PaletteContainer::operator=(const PaletteContainer &cp)
{
    m_pal = cp.m_pal;
    return *this;
}

PaletteContainer &PaletteContainer::operator=(PaletteContainer &&mv)
{
    m_pal = qMove(mv.m_pal);
    return *this;
}

PaletteContainer::~PaletteContainer()
{
    //qDebug("PaletteContainer::~PaletteContainer()\n");
}

TreeNode *PaletteContainer::clone() const
{
    return new PaletteContainer(*this);
}

eTreeElemDataType PaletteContainer::nodeDataTy() const
{
    return eTreeElemDataType::palette;
}

const QString &PaletteContainer::nodeDataTypeName() const
{
    return ElemName_Palette;
}

QString PaletteContainer::nodeDisplayName()const
{
    return nodeDataTypeName();
}
