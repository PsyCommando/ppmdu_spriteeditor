#ifndef PALETTE_HPP
#define PALETTE_HPP
#include <QVector>
#include <QRgb>
#include <QScopedPointer>
#include <src/data/treeelem.hpp>
#include <src/data/sprite/palettemodel.hpp>

extern const char * ElemName_Palette;

//*******************************************************************
//  PaletteContainer
//*******************************************************************
//Category tree node containing all the palettes for a sprite.
class PaletteContainer : public BaseTreeTerminalChild<&ElemName_Palette>
{
public:

    PaletteContainer( TreeElement * parent );
    PaletteContainer( PaletteContainer && mv );
    PaletteContainer( const PaletteContainer & cp );
    PaletteContainer & operator=( const PaletteContainer & cp );
    PaletteContainer & operator=( PaletteContainer && mv );
    ~PaletteContainer();

    void clone(const TreeElement *other)
    {
        const PaletteContainer * ptr = static_cast<const PaletteContainer*>(other);
        if(!ptr)
            throw std::runtime_error("PaletteContainer::clone(): other is not a PaletteContainer!");
        (*this) = *ptr;
    }

public:
    QVariant nodeData(int column, int role) const override;

    Sprite                      * parentSprite();
    inline bool                   nodeIsMutable()const override {return false;}
    inline PaletteModel         * getModel()                    {return m_model.data();}
    inline const PaletteModel   * getModel()const               {return m_model.data();}

    inline QVector<QRgb>        &getPalette()                           {return m_pal;}
    inline const QVector<QRgb>  &getPalette()const                      {return m_pal;}
    inline void                 setPalette(QVector<QRgb>        & pal)  {m_pal = pal;}
    inline void                 setPalette(const QVector<QRgb>  & pal)  {m_pal = pal;}
    inline void                 setPalette(QVector<QRgb>        && pal) {m_pal = qMove(pal);}

private:
    QScopedPointer<PaletteModel>    m_model;
    QVector<QRgb>                   m_pal;
};

#endif // PALETTE_HPP
