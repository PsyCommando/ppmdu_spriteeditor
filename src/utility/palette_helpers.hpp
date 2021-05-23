#ifndef PALETTE_HELPERS_HPP
#define PALETTE_HELPERS_HPP
#include <QString>
#include <QModelIndex>
#include <QVector>
#include <QRgb>
#include <src/utility/file_support.hpp>

class Sprite;

//=====================================
//Helper Functions
//=====================================

//Imports a color palette from a supported file type. Must specify the expected palette format.
void ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type);

//Dumps a colorpalette from a sprite, to the specified path, in the specified file format.
void DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type);
void DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type);

//Paints the color palette as a series of 16x16 pixels squares. 16 squares per lines.
QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors );

#endif // PALETTE_HELPERS_HPP
