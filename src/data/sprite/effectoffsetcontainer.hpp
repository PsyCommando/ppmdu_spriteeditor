#ifndef EFFECTOFFSETCONTAINER_HPP
#define EFFECTOFFSETCONTAINER_HPP
#include <src/data/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

class Sprite;
extern const char * ElemName_EffectOffset;
//====================================================================
//  EffectOffsetContainer
//====================================================================
class EffectOffsetContainer : public BaseTreeTerminalChild<&ElemName_EffectOffset>
{
public:

    EffectOffsetContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::effectOffsets);
    }

    ~EffectOffsetContainer()
    {
        qDebug("EffectOffsetContainer::~EffectOffsetContainer()\n");
    }

    void clone(const TreeElement *other)
    {
        const EffectOffsetContainer * ptr = static_cast<const EffectOffsetContainer*>(other);
        if(!ptr)
            throw std::runtime_error("EffectOffsetContainer::clone(): other is not a EffectOffsetContainer!");
        (*this) = *ptr;
    }

    QVariant nodeData(int column, int role) const override
    {
        if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    Sprite * parentSprite();

    std::vector<fmt::effectoffset> exportEffects()const
    {
        return m_efx;
    }

    void importEffects( const std::vector<fmt::effectoffset> & efx )
    {
        m_efx = efx;
    }

    bool nodeIsMutable()const override {return false;}

private:
    std::vector<fmt::effectoffset> m_efx;
};

#endif // EFFECTOFFSETCONTAINER_HPP
