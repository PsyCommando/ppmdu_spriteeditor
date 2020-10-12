#ifndef PALETTE_HPP
#define PALETTE_HPP
#include <QVector>
#include <QRgb>
#include <src/data/treenodeterminal.hpp>

extern const QString ElemName_Palette;

//*******************************************************************
//  PaletteContainer
//*******************************************************************
//Category tree node containing all the palettes for a sprite.
class PaletteContainer : public TreeNodeTerminal
{
    friend class PaletteModel;
public:
    PaletteContainer( TreeNode * parent );
    PaletteContainer( PaletteContainer && mv );
    PaletteContainer( const PaletteContainer & cp );
    PaletteContainer & operator=( const PaletteContainer & cp );
    PaletteContainer & operator=( PaletteContainer && mv );
    ~PaletteContainer();

    // TreeNode interface
public:
    TreeNode *                  clone() const override;
    eTreeElemDataType           nodeDataTy() const override;
    const QString &             nodeDataTypeName() const override;
    inline bool                 nodeIsMutable()const override {return false;}
    QString                     nodeDisplayName() const override;

public:
    inline QVector<QRgb>        &getPalette()                           {return m_pal;}
    inline const QVector<QRgb>  &getPalette()const                      {return m_pal;}
    inline void                 setPalette(QVector<QRgb>        & pal)  {m_pal = pal;}
    inline void                 setPalette(const QVector<QRgb>  & pal)  {m_pal = pal;}
    inline void                 setPalette(QVector<QRgb>        && pal) {m_pal = qMove(pal);}

private:
    QVector<QRgb> m_pal;
};

#endif // PALETTE_HPP
