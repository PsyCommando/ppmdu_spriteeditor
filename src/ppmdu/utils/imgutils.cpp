#include "imgutils.hpp"
#include <QPixmap>
#include <QBrush>

namespace utils
{
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

    QByteArray Expand4BppTo8Bpp(const QByteArray & src)
    {
        QByteArray out;
        out.reserve(src.size() * 2);
        for(auto by : src)
        {
            out.push_back(by & 0x0F);
            out.push_back((by >> 4) & 0x0F);
        }
        return out;
    }

    std::vector<uint8_t> Expand4BppTo8Bpp(const std::vector<uint8_t> & src)
    {
        std::vector<uint8_t> out;
        out.reserve(src.size() * 2);
        for(auto by : src)
        {
            out.push_back(by & 0x0F);
            out.push_back((by >> 4) & 0x0F);
        }
        return out;
    }

    std::vector<uint8_t> Reduce8bppTo4bpp(const std::vector<uint8_t> &src )
    {
        std::vector<uint8_t> out;
        out.reserve(src.size() / 2);
        for(size_t cntpix = 0; (cntpix + 1) < src.size(); cntpix += 2)
            out.push_back((src[cntpix] & 0x0F) | ((src[cntpix+1] << 4) & 0xF0));
        return out;
    }

    std::vector<uint8_t> Reduce8bppTo4bpp(const QImage &src )
    {
        std::vector<uint8_t> out;
        size_t sz = src.size().width() * src.size().height();
        out.reserve(sz / 2);

        size_t cntpix = 0;
        for(; (cntpix + 1) < sz; cntpix += 2)
        {
            int x0 = cntpix % src.width();
            int y0 = cntpix / src.width();
            int x1 = (cntpix+1) % src.width();
            int y1 = (cntpix+1) / src.width();
            out.push_back((src.pixelIndex(x0,y0) & 0x0F) | ((src.pixelIndex(x1,y1) << 4) & 0xF0));
        }

        //Handle leftover last if its the case
        if(cntpix != sz)
        {
            int x0 = cntpix % src.width();
            int y0 = cntpix / src.width();
            out.push_back(src.pixelIndex(x0,y0) & 0x0F);
        }

        return out;
    }

    std::vector<uint8_t> ImgToRaw(const QImage & src)
    {
        const int BPP = src.pixelFormat().bitsPerPixel();
        std::vector<uint8_t> out;
        for(int  y = 0; y < src.height(); ++y)
        {
            for(int x = 0; x < src.width(); ++x)
            {
                if( BPP == 8 )
                    out.push_back(static_cast<uint8_t>(src.pixelIndex(x, y)));
                else if(BPP == 32)
                    utils::writeBytesFrom<uint32_t>(static_cast<uint32_t>(src.pixel(x, y)), std::back_inserter(out) );
                else //Unsupported format!
                    Q_ASSERT(false);
            }
        }
        return out;
    }

    QVector<QRgb> ConvertSpritePalette(std::vector<uint32_t> & colors)
    {
        QVector<QRgb> outcol;
        outcol.reserve(colors.size());

        for( const auto & acol : colors )
            outcol.push_back( QRgb(acol >> 8) | 0xFF000000 ); //Force alpha to 255
        return outcol;
    }

    std::vector<uint32_t> ConvertSpritePaletteFromQt(const QVector<QRgb> & colors)
    {
        std::vector<uint32_t> outcol;
        outcol.reserve(colors.size());

        for( const auto & acol : colors )
            outcol.push_back( (acol << 8) | 0x00000080 ); //Force alpha to 0x80
        return outcol;
    }

    std::vector<uint8_t> TileFromImg( const QImage & src )
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
        const size_t imgnbtileswidth  = src.width()  / TileWidth;
        const size_t imgnbtilesheight = src.height() / TileHeight;
        const size_t NbPixelsPerTile  = (TileWidth * TileHeight);
        const size_t NbPixels         = (imgnbtileswidth * imgnbtilesheight) * NbPixelsPerTile;
        std::vector<uint8_t> out(NbPixels, 0);

        for( int y = 0; y < src.height(); ++y )
        {
            for( int x = 0; x < src.width(); ++x )
            {
                size_t rowtile = (y / TileHeight);
                size_t coltile = (x / TileWidth);
                size_t tile = ((rowtile * imgnbtileswidth) + coltile);
                size_t tileX = x % TileWidth;
                size_t tileY = y % TileHeight;
                size_t pixel = (tile * NbPixelsPerTile) + (tileX + (tileY * TileWidth));
                Q_ASSERT(pixel < out.size());
                out[pixel] = src.pixelIndex(x, y);
            }
        }
        return out;
    }

};
