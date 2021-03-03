#include "palette_helpers.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/color_utils.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <src/extfmt/riff_palette.hpp>
#include <src/extfmt/text_palette.hpp>
#include <src/extfmt/gpl_palette.hpp>
#include <src/utility/portability.hpp>
#include <QBrush>
#include <QPainter>

QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors )
{
    static const int NbColorsPerRow = 16;
    static const int ColorSquareRes = 16;

    const int NbRows = (colors.size() % NbColorsPerRow == 0)? (colors.size() / NbColorsPerRow) : ((colors.size() / NbColorsPerRow) + 1);
    QPixmap palette( ColorSquareRes * NbColorsPerRow, ColorSquareRes * NbRows);
    QPainter mypaint(&palette);

    int curY = 0;
    for( int cntcol = 0; cntcol < colors.size();)
    {
        for( int cntr = 0; (cntr < NbColorsPerRow) && (cntcol < colors.size()); ++cntr, ++cntcol )
        {
            mypaint.setBrush(QBrush( QColor(colors.at(cntcol)) ));
            mypaint.drawRect( cntr * ColorSquareRes, curY, ColorSquareRes, ColorSquareRes );
        }
        curY += NbColorsPerRow;
    }
    return palette;
}

//=================================================================
// Palette Handling
//=================================================================
void ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type)
{
    if(!spr)
        throw std::range_error("ImportPalette(): Invalid sprite index!!");

    std::vector<uint8_t> fdata = utils::ReadFileToByteVector(path.toStdString());

    switch(type)
    {
    case ePaletteDumpType::RIFF_Pal:
        {
            //can't use move here since we need the implicit convertion on copy
            std::vector<uint32_t> imported = utils::ImportFrom_RIFF_Palette(fdata, utils::RGBToARGB);
            spr->setPalette( QVECTOR_RAMGE_CTOR(QVector<QRgb>, imported.begin(), imported.end()) ); //since QRgb is ARGB, we use this encoder!
            break;
        }
    case ePaletteDumpType::TEXT_Pal:
        {
            //can't use move here since we need the implicit convertion on copy
            std::vector<uint32_t> imported = utils::ImportPaletteAsTextPalette(fdata, utils::RGBToARGB);
            spr->setPalette( QVECTOR_RAMGE_CTOR(QVector<QRgb>, imported.begin(), imported.end()) );
            break;
        }
    case ePaletteDumpType::GIMP_PAL:
        {
            //can't use move here since we need the implicit convertion on copy
            std::vector<uint32_t> imported = utils::ImportGimpPalette(fdata, utils::RGBToARGB);
            spr->setPalette( QVECTOR_RAMGE_CTOR(QVector<QRgb>, imported.begin(), imported.end()) );
            break;
        }
    case ePaletteDumpType::PNG_PAL:
        {
            //can't use move here since we need the implicit convertion on copy
            QImage png(path, "png");
            spr->setPalette(png.colorTable());
            break;
        }
    case ePaletteDumpType::BMP_PAL:
        {
            //can't use move here since we need the implicit convertion on copy
            QImage bitmap(path, "bmp");
            spr->setPalette(bitmap.colorTable());
            break;
        }
    default:
        throw std::invalid_argument("ImportPalette(): Invalid palette type!");
    };
}

void DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type)
{
    if(!sprite.isValid())
        throw std::invalid_argument("DumpPalette(): Invalid sprite index!!");

    const Sprite * spr = reinterpret_cast<Sprite*>(sprite.internalPointer());
    DumpPalette(spr, path, type);
}

void DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type)
{
    if(!spr)
        throw std::range_error("DumpPalette(): Invalid sprite index!!");

    std::vector<uint8_t> fdata;
    std::vector<uint32_t> pal;
    for(const QRgb & col : spr->getPalette())
    {
        pal.push_back(static_cast<uint32_t>(col));
    }

    switch(type)
    {
    case ePaletteDumpType::RIFF_Pal:
        {
            qDebug("Exporting RIFF Palette\n");
            fdata = utils::ExportTo_RIFF_Palette(pal, utils::ARGBToComponents); //sice QRgb is ARGB, we use this decoder!
            qDebug("Exporting RIFF Palette, conversion complete!\n");
            break;
        }
    case ePaletteDumpType::TEXT_Pal:
        {
            fdata = utils::ExportPaletteAsTextPalette(pal, utils::ARGBToComponents);
            break;
        }
    case ePaletteDumpType::GIMP_PAL:
        {
            fdata = utils::ExportGimpPalette(pal, utils::ARGBToComponents);
            break;
        }
    default:
        throw std::invalid_argument("DumpPalette(): Invalid palette destination type!");
    };

    qDebug("Exporting Palette, Writing to file!\n");
    QSaveFile sf(path);
    if(!sf.open( QSaveFile::WriteOnly ))
        throw std::runtime_error(QString("DumpPalette(): Couldn't open file \"%1\" for writing!\n").arg(path).toStdString());

    if( sf.write( (char*)fdata.data(), fdata.size() ) < static_cast<qint64>(fdata.size()) )
        qWarning("DumpPalette(): The amount of bytes written to file differs from the expected filesize!\n");

    qDebug("Exporting Palette, written! Now commiting\n");

    if(!sf.commit())
        throw std::runtime_error(QString("DumpPalette(): Commit to \"%1\" failed!\n").arg(path).toStdString());
    qDebug("Exporting Palette, commited!\n");
}

