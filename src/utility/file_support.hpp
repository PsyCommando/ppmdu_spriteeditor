#ifndef FILE_SUPPORT_HPP
#define FILE_SUPPORT_HPP
/*
 * Useful things for handling file formats
*/
#include <QString>
#include <QHash>
#include <src/data/sprite/sprite.hpp>

//
//File Extensions
//
extern const QString FileExtPack;
extern const QString FileExtWAN;
extern const QString FileExtWAT;

//
//File Filters
//
extern const QHash<QString, QString>    SupportedFileFiltersByTypename;
extern const QVector<QString>           PaletteFileFilter; //list of file filters matching the ePaletteDumpType values below!

const QString &AllSupportedFileFilter();

//Palette
enum struct ePaletteDumpType : unsigned int
{
    RIFF_Pal = 0,
    TEXT_Pal,
    GIMP_PAL,
    PNG_PAL, //Palette from a PNG image!
    INVALID,
};
ePaletteDumpType FilterStringToPaletteType(const QString & selectedfilter);
const QString & GetPaletteFileFilterString( ePaletteDumpType ty );

//
//Helper Functions
//
void ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type);
void DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type);
void DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type);

#endif // FILE_SUPPORT_HPP
