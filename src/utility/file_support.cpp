#include "file_support.hpp"
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/color_utils.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <src/extfmt/riff_palette.hpp>
#include <src/extfmt/text_palette.hpp>
#include <src/extfmt/gpl_palette.hpp>
#include <QFileDialog>
#include <QWidget>

const QSize MaxValidCellPartSize {32, 32};

//Helper for generating the supported file filters at runtime
template<typename CHILD_T>
    class SupportedFilters
{
    typedef CHILD_T child_t;
    QString filter;
public:
    SupportedFilters()
    {
        InitSupported();
    }
    virtual ~SupportedFilters() {}

    operator const QString & ()const
    {
        return filter;
    }

    virtual void InitSupported()
    {
        //CHILD_T * pchild = static_cast<CHILD_T*>(this);
        //pchild->InitSupported();
        AddAllFromHash(fileFilters());
        MakeAllSupportedEntry(fileFilters());
        filter = QString("All supported (%s)").arg(filter);
    }

protected:
    virtual const QHash<QString, QString> fileFilters()const = 0;

    void AddAllFromHash(const QHash<QString, QString>& htable)
    {
        for(const auto & entry : htable)
        {
            if(filter.size() > 0)
                filter = QString("%1;%2").arg(filter).arg(entry);
            else
                filter = QString("%1").arg(entry);
        }
    }

    void MakeAllSupportedEntry(const QHash<QString, QString>& htable)
    {
        QString f;
        for(const auto & entry : htable)
        {
            if(f.size() > 0)
                f = QString("%1, *.%2").arg(f).arg(entry);
            else
                f = QString("*.%1").arg(entry);
        }
        f = QString("All supported (%1)").arg(f);
        filter = QString("%1;%2").arg(filter).arg(f);
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
const QString XMLFileFilter = "XML files (*.xml)";

const QString &AllSupportedXMLFileFilter()
{
    return XMLFileFilter;
}


//=================================================================
// Game File Filters
//=================================================================
const QHash<QString, QString> SupportedFileFiltersByTypename
{
    {FileExtWAN,    "WAN Sprite (*.wan)"},
    {FileExtWAT,    "WAT Sprite (*.wat)"},
    {FileExtPack,   "Pack Files (*.bin)"},
};

//game files support
class all_supported_game_filetypes : public SupportedFilters<all_supported_game_filetypes>
{
public:
    const QHash<QString, QString> fileFilters()const override {return SupportedFileFiltersByTypename;}
};

const QString &AllSupportedGameFileFilter()
{
    static const all_supported_game_filetypes supported;
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

    const QHash<QString, QString> fileFilters()const override {return m_supported;}
protected:
    QHash<QString, QString> m_supported;
};

const QString &AllSupportedGameSpritesFileFilter()
{
    static const all_supported_game_sprites_filetypes supported;
    return supported;
}


//=================================================================
// Image File Filters
//=================================================================
const QHash<QString, QString> SupportedImageFilesFilters
{
    {FileExtPNG, "PNG Image File (*.png)"},
    {FileExtBMP, "Bitmap Image File (*.bmp)"},
};

class all_supported_images_filetypes : public SupportedFilters<all_supported_images_filetypes>
{
public:
    const QHash<QString, QString> fileFilters()const override {return SupportedImageFilesFilters;}
};

const QString &AllSupportedImagesFilesFilter()
{
    static const all_supported_images_filetypes supported;
    return supported;
}

//=================================================================
// Palettes File Filters
//=================================================================
const QHash<QString, QString> SupportedExportPaletteFilesFilters
{
    {FileExtGPL,    "GPL Palette File (*.gpl)"},
    {FileExtPAL,    "GPL Palette File (*.pal)"},
    {FileExtTXTPAL, "GPL Palette File (*.txt)"},
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
    else if(selectedfilter == SupportedImageFilesFilters[FileExtPNG])
        fty = ePaletteDumpType::PNG_PAL;
    else if(selectedfilter == SupportedImageFilesFilters[FileExtBMP])
        fty = ePaletteDumpType::BMP_PAL;
    Q_ASSERT(fty < ePaletteDumpType::INVALID);
    return fty;
}

class all_supported_import_palette_filetypes : public SupportedFilters<all_supported_import_palette_filetypes>
{
public:
    void InitSupported()override
    {
        m_supported = SupportedExportPaletteFilesFilters;
        m_supported.insert(SupportedImageFilesFilters);
        SupportedFilters::InitSupported();
    }

    const QHash<QString, QString> fileFilters()const override {return m_supported;}
protected:
    QHash<QString, QString> m_supported;
};

class all_supported_export_palette_filetypes : public SupportedFilters<all_supported_export_palette_filetypes>
{
public:
    const QHash<QString, QString> fileFilters()const override {return SupportedExportPaletteFilesFilters;}
};

const QString &AllSupportedImportPaletteFilesFilter()
{
    static const all_supported_import_palette_filetypes supported;
    return supported;
}

const QString &AllSupportedExportPaletteFilesFilter()
{
    static const all_supported_export_palette_filetypes supported;
    return supported;
}

//=================================================================
// Helper Functions
//=================================================================

QStringList GetImagesPathsFromDialog(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileNames(parent, title, QString(), AllSupportedImagesFilesFilter());
}

QString GetImagePathFromDialog(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, title, QString(), AllSupportedImagesFilesFilter());
}

QString GetXMLOpenFile(const QString &title, QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent, title, QString(), XMLFileFilter);
}

QString GetXMLSaveFile(const QString &title, QWidget *parent)
{
    return QFileDialog::getSaveFileName(parent, title, QString(), XMLFileFilter);
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


