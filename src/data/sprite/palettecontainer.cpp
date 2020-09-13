#include "palettecontainer.hpp"
#include <src/data/sprite/sprite.hpp>

const char * ElemName_Palette = "Palette";

//============================================================================================
//  PaletteContainer
//============================================================================================
PaletteContainer::PaletteContainer(TreeElement *parent)
    :BaseTreeTerminalChild(parent), m_model(new PaletteModel(&m_pal))
{
    setNodeDataTy(eTreeElemDataType::palette);
}

PaletteContainer::PaletteContainer(PaletteContainer &&mv)
    :BaseTreeTerminalChild(mv)
{
    setNodeDataTy(eTreeElemDataType::palette);
    operator=(qMove(mv));
}

PaletteContainer::PaletteContainer(const PaletteContainer &cp)
    :BaseTreeTerminalChild(cp)
{
    setNodeDataTy(eTreeElemDataType::palette);
    operator=(cp);
}

PaletteContainer &PaletteContainer::operator=(const PaletteContainer &cp)
{
    m_pal = cp.m_pal;
    m_model.reset(new PaletteModel(&m_pal));
    return *this;
}

PaletteContainer &PaletteContainer::operator=(PaletteContainer &&mv)
{
    m_pal = qMove(mv.m_pal);
    m_model.reset(new PaletteModel(&m_pal));
    return *this;
}

PaletteContainer::~PaletteContainer()
{
    qDebug("PaletteContainer::~PaletteContainer()\n");
}

QVariant PaletteContainer::nodeData(int column, int role) const
{
    if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

