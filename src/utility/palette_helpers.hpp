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
void ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type);
void DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type);
void DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type);

//
QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors );

#endif // PALETTE_HELPERS_HPP
