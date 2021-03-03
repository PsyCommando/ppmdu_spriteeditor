#ifndef FILE_SUPPORT_HPP
#define FILE_SUPPORT_HPP
/*
 * Useful things for handling file formats
*/
#include <QString>
#include <QSize>
#include <QMap>

//==============================================================
// Image stuff
//==============================================================
extern const QSize MaxValidCellPartSize;
bool IsImageResolutionValid(QSize imgres);
//allowDblSize determines whether the function may returns resolutions only achievable with the dblsize flag!
std::optional<QSize> GetNextBestImgPartResolution(QSize srcres);

//==============================================================
//File Extensions
//==============================================================
//Game Files
extern const QString FileExtPack;
extern const QString FileExtWAN;
extern const QString FileExtWAT;
//Data Files
extern const QString FileExtXML;
//Image Files
extern const QString FileExtPNG;
extern const QString FileExtBMP;
//Palette Files
extern const QString FileExtGPL;
extern const QString FileExtPAL;
extern const QString FileExtTXTPAL;

//==============================================================
//File Filters
//==============================================================
extern const QMap<QString, QString>    SupportedFileFiltersByTypename;
extern const QMap<QString, QString>    SupportedImageFilesFilters;
extern const QMap<QString, QString>    SupportedExportPaletteFilesFilters;

const QString &AllSupportedGameFileFilter();
const QString &AllSupportedGameSpritesFileFilter();
const QString &AllSupportedImagesFilesFilter();
const QString &AllSupportedImportPaletteFilesFilter();
const QString &AllSupportedExportPaletteFilesFilter();
const QString &AllSupportedXMLFileFilter();

//Palette Filters
enum struct ePaletteDumpType : unsigned int
{
    RIFF_Pal = 0,
    TEXT_Pal,
    GIMP_PAL,
    PNG_PAL, //Palette from a PNG image!
    BMP_PAL, //Palette from a bmp image
    INVALID,
};
ePaletteDumpType FilterStringToPaletteType(const QString & selectedfilter);
//const QString & GetPaletteFileFilterString( ePaletteDumpType ty );

//==============================================================
// Helpers
//==============================================================
class QWidget;
QStringList GetImagesPathsFromDialog(const QString & title, QWidget * parent = nullptr);
QString GetImagePathFromDialog(const QString & title, QWidget * parent = nullptr);

QString GetXMLOpenFile(const QString & title, QWidget * parent = nullptr);
QString GetXMLSaveFile(const QString & title, QWidget * parent = nullptr);

//Helper to obtain the path file dialogs should open at whenever one is opened
QString GetFileDialogDefaultPath();
QString GetFileDialogDefaultExportPath();
QString GetFileDialogDefaultImportPath();

//Helper to update the path we last used for a project, export, or import operation in the file dialog!
// Save the value to the program settins and make sure the last past can be restored in the file dialog.
void UpdateFileDialogProjectPath(const QString & newpath);
void UpdateFileDialogExportPath(const QString & newpath);
void UpdateFileDialogImportPath(const QString & newpath);

#endif // FILE_SUPPORT_HPP
