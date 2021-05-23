#include "file_support.hpp"
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/color_utils.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <src/extfmt/riff_palette.hpp>
#include <src/extfmt/text_palette.hpp>
#include <src/extfmt/gpl_palette.hpp>
#include <src/data/content_manager.hpp>
#include <src/utility/program_settings.hpp>
#include <src/utility/portability.hpp>
#include <QFileDialog>
#include <QWidget>
#include <QApplication>

const QSize MaxValidCellPartSize {32, 32};

//Helper for generating the supported file filters at runtime
template<typename CHILD_T>
    class SupportedFilters
{
    typedef CHILD_T child_t;
    QString filter;
    bool binitialized {false};
public:
    SupportedFilters() {}
    virtual ~SupportedFilters() {}

    operator const QString & ()const
    {
        return filter;
    }

    virtual void InitSupported()
    {
        AddAllFromHash(fileFilters());
        MakeAllSupportedEntry(fileFilters());
        binitialized = true;
    }

    bool isInitialized()const {return binitialized;}

protected:
    virtual const QMap<QString, QString> fileFilters()const = 0;

    void AddAllFromHash(const QMap<QString, QString>& htable)
    {
        for(const auto & entry : htable)
        {
            if(filter.size() > 0)
                filter = QString("%1;;%2").arg(filter).arg(entry);
            else
                filter = QString("%1").arg(entry);
        }
    }

    void MakeAllSupportedEntry(const QMap<QString, QString>& htable)
    {
        QString supportedext;
        for(const auto & entry : htable)
        {
            if(supportedext.size() > 0)
                supportedext = QString("%1 *.%2").arg(supportedext).arg(htable.key(entry));
            else
                supportedext = QString("*.%1").arg(htable.key(entry));
        }
        supportedext = QString("All supported (%1)").arg(supportedext);
        filter = QString("%1;;%2").arg(supportedext).arg(filter);
    }
};

//=================================================================
// File Extensions
//=================================================================
const QString FileExtPack   = "bin";
const QString FileExtWAN    = "wan";
const QString FileExtWAT    = "wat";

const QString FileExtXML    = "xml";

const QString FileExtPNG    = "png";
const QString FileExtBMP    = "bmp";

const QString FileExtGPL    = QString::fromStdString(utils::GPL_PAL_Filext);
const QString FileExtPAL    = QString::fromStdString(utils::RIFF_PAL_Filext);
const QString FileExtTXTPAL = QString::fromStdString(utils::TEXT_PAL_Filext);

//=================================================================
// Misc File Filters
//=================================================================
const QString &AllSupportedXMLFileFilter()
{
    static const QString XMLFileFilter = "XML files (*.xml)";
    return XMLFileFilter;
}


//=================================================================
// Game File Filters
//=================================================================
const QMap<QString, QString> SupportedFileFiltersByTypename
{
    {FileExtWAN,    "WAN Sprite (*.wan)"},
    {FileExtWAT,    "WAT Sprite (*.wat)"},
    {FileExtPack,   "Pack Files (*.bin)"},
};

//game files support
class all_supported_game_filetypes : public SupportedFilters<all_supported_game_filetypes>
{
public:
    const QMap<QString, QString> fileFilters()const override {return SupportedFileFiltersByTypename;}
};

const QString &AllSupportedGameFileFilter()
{
    static all_supported_game_filetypes supported;
    if(!supported.isInitialized())
        supported.InitSupported();
    return supported;
}

//Sprites support
class all_supported_game_sprites_filetypes : public SupportedFilters<all_supported_game_sprites_filetypes>
{
public:
    void InitSupported()override
    {
        m_supported = SupportedFileFiltersByTypename;
        m_supported.remove(FileExtPack); //All but packfile
        SupportedFilters::InitSupported();
    }

    const QMap<QString, QString> fileFilters()const override {return m_supported;}
protected:
    QMap<QString, QString> m_supported;
};

const QString &AllSupportedGameSpritesFileFilter()
{
    static all_supported_game_sprites_filetypes supported;
    if(!supported.isInitialized())
        supported.InitSupported();
    return supported;
}


//=================================================================
// Image File Filters
//=================================================================
const QMap<QString, QString> SupportedImageFilesFilters
{
    {FileExtPNG, "PNG Image File (*.png)"},
    {FileExtBMP, "Bitmap Image File (*.bmp)"},
};

class all_supported_images_filetypes : public SupportedFilters<all_supported_images_filetypes>
{
public:
    const QMap<QString, QString> fileFilters()const override {return SupportedImageFilesFilters;}
};

const QString &AllSupportedImagesFilesFilter()
{
    static all_supported_images_filetypes supported;
    if(!supported.isInitialized())
        supported.InitSupported();
    return supported;
}

//=================================================================
// Palettes File Filters
//=================================================================
const QMap<QString, QString> SupportedExportPaletteFilesFilters
{
    {FileExtGPL,    "GIMP GPL Palette (*.gpl)"},
    {FileExtPAL,    "Microsoft RIFF Palette (*.pal)"},
    {FileExtTXTPAL, "Text File Hex Color (*.txt)"},
    {FileExtPNG,    SupportedImageFilesFilters[FileExtPNG]},
    {FileExtBMP,    SupportedImageFilesFilters[FileExtBMP]},
};

ePaletteDumpType FilterStringToPaletteType( const QString & selectedfilter )
{
    ePaletteDumpType fty = ePaletteDumpType::INVALID;
    if(selectedfilter == SupportedExportPaletteFilesFilters[FileExtPAL])
        fty = ePaletteDumpType::RIFF_Pal;
    else if(selectedfilter == SupportedExportPaletteFilesFilters[FileExtTXTPAL])
        fty = ePaletteDumpType::TEXT_Pal;
    else if(selectedfilter == SupportedExportPaletteFilesFilters[FileExtGPL])
        fty = ePaletteDumpType::GIMP_PAL;
    else if(selectedfilter == SupportedExportPaletteFilesFilters[FileExtPNG])
        fty = ePaletteDumpType::PNG_PAL;
    else if(selectedfilter == SupportedExportPaletteFilesFilters[FileExtBMP])
        fty = ePaletteDumpType::BMP_PAL;
    Q_ASSERT(fty < ePaletteDumpType::INVALID);
    return fty;
}

//We also support importing palettes from images!!!!
class all_supported_import_palette_filetypes : public SupportedFilters<all_supported_import_palette_filetypes>
{
public:
    void InitSupported()override
    {
        m_supported = SupportedExportPaletteFilesFilters;
        QMAP_INSERT_QMAP(SupportedImageFilesFilters, m_supported);
        SupportedFilters::InitSupported();
    }

    const QMap<QString, QString> fileFilters()const override {return m_supported;}
protected:
    QMap<QString, QString> m_supported;
};

class all_supported_export_palette_filetypes : public SupportedFilters<all_supported_export_palette_filetypes>
{
public:
    const QMap<QString, QString> fileFilters()const override {return SupportedExportPaletteFilesFilters;}
};

const QString &AllSupportedImportPaletteFilesFilter()
{
    static all_supported_import_palette_filetypes supported;
    if(!supported.isInitialized())
        supported.InitSupported();
    return supported;
}

const QString &AllSupportedExportPaletteFilesFilter()
{
    static all_supported_export_palette_filetypes supported;
    if(!supported.isInitialized())
        supported.InitSupported();
    return supported;
}

//=================================================================
// Helper Functions
//=================================================================

QStringList GetImagesPathsFromDialog(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileNames(parent, title, GetFileDialogDefaultImportPath(), AllSupportedImagesFilesFilter());
}

QString GetImagePathFromDialog(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, title, GetFileDialogDefaultImportPath(), AllSupportedImagesFilesFilter());
}

QString GetXMLOpenFile(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, title, GetFileDialogDefaultImportPath(), AllSupportedXMLFileFilter());
}

QString GetXMLSaveFile(const QString &title, QWidget *parent)
{
    return QFileDialog::getSaveFileName(parent, title, GetFileDialogDefaultExportPath(), AllSupportedXMLFileFilter());
}

bool IsImageResolutionValid(QSize imgres)
{
    for(const auto & supported : fmt::FrameResValues)
    {
        if( supported.first  == imgres.width() &&
            supported.second == imgres.height() )
            return true;
    }
    return false;
}


std::optional<QSize> GetNextBestImgPartResolution(QSize srcres)
{
    for(const auto & supported : fmt::FrameResValues)
    {
        if( supported.first  >= srcres.width() &&
            supported.second >= srcres.height() )
            return std::optional<QSize>(QSize{supported.first, supported.second});
    }
    return std::nullopt;
}


//
//
//
QString GetFileDialogDefaultPath()
{
    const QString lastpath = ProgramSettings::Instance().lastProjectPath();
    if(!lastpath.isNull())
        return lastpath;
    ContentManager & manager = ContentManager::Instance();
    if(manager.isContainerLoaded())
        return manager.getContainerParentDir();
    return QApplication::applicationDirPath();
}

QString GetFileDialogDefaultExportPath()
{
    const QString lastpath = ProgramSettings::Instance().lastExportPath();
    if(!lastpath.isNull())
        return lastpath;
    return GetFileDialogDefaultPath();
}

QString GetFileDialogDefaultImportPath()
{
    const QString lastpath = ProgramSettings::Instance().lastImportPath();
    if(!lastpath.isNull())
        return lastpath;
    return GetFileDialogDefaultPath();
}

void UpdateFileDialogProjectPath(const QString &newpath)
{
    const QFileInfo finf = newpath;
    if(!finf.exists())
        return;
    const QString lastprojpath = (finf.isDir())?
                                    finf.absolutePath() :
                                    finf.absoluteDir().absolutePath();
    ProgramSettings::Instance().setLastProjectPath(lastprojpath);
}

void UpdateFileDialogExportPath(const QString &newpath)
{
    const QFileInfo finf = newpath;
    if(!finf.exists())
        return;
    const QString lastexportpath = (finf.isDir())?
                                        finf.absolutePath() :
                                        finf.absoluteDir().absolutePath();
    ProgramSettings::Instance().setLastExportPath(lastexportpath);
}

void UpdateFileDialogImportPath(const QString &newpath)
{
    const QFileInfo finf = newpath;
    if(!finf.exists())
        return;
    const QString lastimportpath = (finf.isDir())?
                                        finf.absolutePath() :
                                        finf.absoluteDir().absolutePath();
    ProgramSettings::Instance().setLastImportPath(lastimportpath);
}
