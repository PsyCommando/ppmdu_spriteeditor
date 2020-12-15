#ifndef PALETTE_HELPERS_HPP
#define PALETTE_HELPERS_HPP
#include <QString>
#include <QModelIndex>
#include <src/utility/file_support.hpp>

class Sprite;

//=====================================
//Helper Functions
//=====================================
void ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type);
void DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type);
void DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type);

#endif // PALETTE_HELPERS_HPP
