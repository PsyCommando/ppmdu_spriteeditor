#include "imgutils.hpp"
#include <QPixmap>

namespace utils
{
    QVector<QRgb> ConvertSpritePalette(const std::vector<uint32_t> & colors)
    {
        QVector<QRgb> outcol;
        outcol.reserve(colors.size());

        for(const auto & acol : colors)
            outcol.push_back( QRgb(acol >> 8) | 0xFF000000 ); //Force alpha to 255
        return outcol;
    }

    std::vector<uint32_t> ConvertSpritePaletteFromQt(const QVector<QRgb> & colors)
    {
        std::vector<uint32_t> outcol;
        outcol.reserve(colors.size());

        for(const auto & acol : colors)
            outcol.push_back( (acol << 8) | 0x00000080 ); //Force alpha to 0x80
        return outcol;
    }


    QByteArray Expand4BppTo8Bpp(const QByteArray & src)
    {
        QByteArray out;
        out.reserve(src.size() * 2);
        auto itend = src.end();
        auto itwhere = std::back_inserter(out);
        for(auto itcur = src.begin(); itcur != itend;)
        {
            itcur = Pixel4bppTo8bpp(itcur, itend, itwhere);
        }
        return out;
    }

    std::vector<uint8_t> Expand4BppTo8Bpp(const std::vector<uint8_t> & src)
    {
        std::vector<uint8_t> out;
        out.reserve(src.size() * 2);
        auto itend = src.end();
        auto itwhere = std::back_inserter(out);
        for(auto itcur = src.begin(); itcur != itend;)
        {
            itcur = Pixel4bppTo8bpp(itcur, itend, itwhere);
        }
        return out;
    }

    std::vector<uint8_t> Reduce8bppTo4bpp(const std::vector<uint8_t> &src )
    {
        std::vector<uint8_t> out;
        out.reserve(src.size() / 2);
        auto itend = src.end();
        auto itwhere = std::back_inserter(out);
        for(auto itcur = src.begin(); itcur != src.end();)
        {
            itcur = Pixel8bppTo4bpp(itcur, itend, itwhere);
        }
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
            out.push_back(Pixel8bppTo4bpp(src.pixelIndex(x0,y0), src.pixelIndex(x1,y1)));
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

    std::vector<uint8_t> ImgToRaw4bpp(const QImage &src)
    {
        Q_ASSERT(src.pixelFormat().bitsPerPixel() == 8);
        std::vector<uint8_t> out;
        out.reserve((src.width() * src.height()) / 2);
        for(int  y = 0; y < src.height(); ++y)
        {
            for(int x = 0; x < src.width(); x+=2)
            {
                uint8_t p1 = src.pixelIndex(x, y),
                        p2 = (src.width() >= x + 1)? src.pixelIndex(x+1, y) : 0;
                out.push_back(Pixel8bppTo4bpp(p1, p2));
            }
        }
        return out;
    }

    std::vector<uint8_t> ImgToRaw8bpp(const QImage & src)
    {
        Q_ASSERT(src.pixelFormat().bitsPerPixel() == 8);
        std::vector<uint8_t> out;
        out.reserve(src.width() * src.height());
        for(int  y = 0; y < src.height(); ++y)
        {
            for(int x = 0; x < src.width(); ++x)
            {
                out.push_back(static_cast<uint8_t>(src.pixelIndex(x, y)));
            }
        }
        return out;
    }



    std::vector<uint8_t> Tile8bppFromImg( const QImage & src )
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

    std::vector<uint8_t> Tile8bpp(const std::vector<uint8_t> &src, int w, int h)
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
        const size_t imgnbtileswidth  = w  / TileWidth;
        const size_t imgnbtilesheight = h / TileHeight;
        const size_t NbPixelsPerTile  = (TileWidth * TileHeight);
        const size_t NbPixels         = (imgnbtileswidth * imgnbtilesheight) * NbPixelsPerTile;
        std::vector<uint8_t> out(NbPixels, 0);

        for( int y = 0; y < h; ++y )
        {
            for( int x = 0; x < w; ++x )
            {
                size_t rowtile = (y / TileHeight);
                size_t coltile = (x / TileWidth);
                size_t tile = ((rowtile * imgnbtileswidth) + coltile);
                size_t tileX = x % TileWidth;
                size_t tileY = y % TileHeight;
                size_t pixel = (tile * NbPixelsPerTile) + (tileX + (tileY * TileWidth));
                Q_ASSERT(pixel < out.size());
                size_t srcpix = (y * w + x);
                out[pixel] = src[srcpix];
            }
        }
        return out;
    }

    std::vector<uint8_t> Tile4bpp(const std::vector<uint8_t> &src, int w, int h)
    {
        static const size_t TILE_WIDTH = 8;
        static const size_t TILE_BYTE_SZ = 32;
        static const size_t TILE_ROW_BYTE_SZ = 4;
        static const size_t NB_ROWS_PER_TILE = 4;
        const size_t NbTilesPerRows = w / TILE_WIDTH;
        const size_t TileByteIncrementOffset = NbTilesPerRows * TILE_BYTE_SZ; //Nb of bytes to skip between each rows of a tile

        std::vector<uint8_t> out(src.size(), 0);
        auto itbeg = src.begin();
        auto itend = src.end();
        size_t tileX = 0,       /*Current tile on the current row*/
               cnttiles = 0;    /*Current tile being processed*/

        for(auto it = itbeg; it != itend; ++it)
        {
            tileX = cnttiles % NbTilesPerRows; //The current tile on the current row of the whole image
            //Write a tile
            for(size_t cntrow = 0; cntrow < NB_ROWS_PER_TILE; ++cntrow)
            {
                //Grab an entire row of pixels from the tile in a single 4 bytes integer at a time
                uint32_t row = utils::readBytesAs<uint32_t>(it, itend);
                //Insert it at where the tile's next row should be in the image
                size_t insertionoffset = (cntrow * TileByteIncrementOffset) +
                                         (tileX * TILE_ROW_BYTE_SZ);
                auto itinsert = std::next(out.begin(), insertionoffset);
                utils::writeBytesFrom(row, itinsert);
            }
            ++cnttiles;
        }
        return out;
    }

    size_t GetPixelIndexFromRawTiled8bpp(size_t x, size_t y, size_t width, size_t tilew, size_t tileh)
    {
        static const size_t NBPixelTile = tilew * tileh;
        const size_t widthintiles  = width  / tilew;
        const size_t rowtile = (y / tileh);
        const size_t coltile = (x / tilew);
        const size_t tile = ((rowtile * widthintiles) + coltile);
        const size_t tileX = x % tilew;
        const size_t tileY = y % tileh;

        return (tile * NBPixelTile) + (tileX + (tileY * tilew));
    }

    size_t GetPixelIndexFromRawTiled4bpp(size_t x, size_t y, size_t width, size_t tilew, size_t tileh)
    {
        return GetPixelIndexFromRawTiled8bpp(x,y,width,tilew,tileh) / 2; //Gotta divide the index by 2 since there's 2 pixels per bytes
    }

    template<>
        uint8_t GetPixelFromRawTiled8bpp<const std::vector<uint8_t>&>(const std::vector<uint8_t> & cnt, size_t x, size_t y, size_t width, size_t tilew, size_t tileh)
    {
        return cnt[GetPixelIndexFromRawTiled8bpp(x, y, width, tilew, tileh)];
    }

    template<>
        uint8_t GetPixelFromRawTiled4bpp<const std::vector<uint8_t>&>(const std::vector<uint8_t>& cnt, size_t x, size_t y, size_t width, size_t tilew, size_t tileh)
    {
        const size_t pixelindex = GetPixelIndexFromRawTiled8bpp(x,y,width,tilew,tileh);
        const size_t byteindex = pixelindex / 2;
        //Even pixels are the lower bits, odds are the higher bits
        if(pixelindex % 2 == 0)
            return cnt[byteindex] & 0x0F;
        else
            return (cnt[byteindex] >> 4) & 0x0F;
    }

};
