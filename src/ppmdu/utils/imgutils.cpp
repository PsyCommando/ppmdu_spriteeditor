#include "imgutils.hpp"
#include <QPixmap>
#include <QBrush>

namespace utils
{
    QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors )
    {
        static const size_t NbColorsPerRow = 16;
        static const size_t ColorSquareRes = 16;

        const size_t NbRows = (colors.size() % NbColorsPerRow == 0)? (colors.size() / NbColorsPerRow) : ((colors.size() / NbColorsPerRow) + 1);
        QPixmap palette( ColorSquareRes * NbColorsPerRow, ColorSquareRes * NbRows);
        QPainter mypaint(&palette);

        int curY = 0;
        for( size_t cntcol = 0; cntcol < colors.size(); /*++cntcol*/ )
        {
            for( size_t cntr = 0; (cntr < NbColorsPerRow) && (cntcol < colors.size()); ++cntr, ++cntcol )
            {
                mypaint.setBrush(QBrush( QColor(colors.at(cntcol)) ));
                mypaint.drawRect( cntr * ColorSquareRes, curY, ColorSquareRes, ColorSquareRes );
            }
            curY += NbColorsPerRow;
        }
        return std::move(palette);
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
        return std::move(out);
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
        return std::move(out);
    }

    std::vector<uint8_t> Reduce8bppTo4bpp(const std::vector<uint8_t> &src )
    {
        std::vector<uint8_t> out;
        out.reserve(src.size() / 2);
        for(size_t cntpix = 0; (cntpix + 1) < src.size(); cntpix += 2)
            out.push_back((src[cntpix] & 0x0F) | ((src[cntpix+1] << 4) & 0xF0));
        return std::move(out);
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

        return std::move(out);
    }

    std::vector<uint8_t> ImgToRaw(const QImage & src)
    {
        const int BPP = src.pixelFormat().bitsPerPixel();
        std::vector<uint8_t> out;
        for(size_t  y = 0; y < src.height(); ++y)
        {
            for(size_t x = 0; x < src.width(); ++x)
            {
                if( BPP == 8 )
                    out.push_back(static_cast<uint8_t>(src.pixelIndex(x, y)));
                else if(BPP == 32)
                    utils::writeBytesFrom<uint32_t>(static_cast<uint32_t>(src.pixel(x, y)), std::back_inserter(out) );
                else //Unsupported format!
                    Q_ASSERT(false);
            }
        }
        return std::move(out);
    }

//    template<class _ByteCntTy>
//        QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors )
//    {
//        static const size_t TilePixWidth  = 8;
//        static const size_t TilePixHeight = 8;
//        const size_t NbPixelsPerTile    = TilePixWidth * TilePixHeight;
//        const size_t NbTilesPerRow      = pixwidth / TilePixWidth;
//        QByteArray tiled;
//        tiled.resize(src.size());

////        QImage debugimg((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
////        debugimg.setColorTable(colors);
////        debugimg.save("./beforetile.png", "png");

//        QVector<QByteArray> tiles( src.size() / NbPixelsPerTile );
//        size_t cntcurtile = 0;
//        for( size_t cntpix = 0; cntpix < src.size(); )
//        {
//            tiles[cntcurtile].resize(NbPixelsPerTile);
//            for(size_t cnttilepix = 0; cnttilepix < NbPixelsPerTile && cntpix < src.size(); ++cnttilepix, ++cntpix)
//                tiles[cntcurtile][cnttilepix] = src[cntpix];
//            ++cntcurtile;
//        }

//        //debug
////        for( size_t cntimage = 0; cntimage < tiles.size(); ++cntimage )
////        {
////            QImage atile((unsigned char *)tiles[cntimage].data(), 8, 8, QImage::Format_Indexed8 );
////            atile.setColorTable(colors);
////            atile.save( QString("./tile%1.png").arg(cntimage), "png");
////        }

//        auto lambdaCopyLineTiles= [&]( size_t firsttileid, size_t tilerow, size_t & outpixpos )
//        {
//            for( size_t cnttile = 0; cnttile < NbTilesPerRow; ++cnttile )
//            {
//                for(size_t cntpix = 0; cntpix < TilePixWidth && outpixpos < tiled.size(); ++cntpix, ++outpixpos )
//                {
//                    tiled[outpixpos] = tiles[firsttileid + cnttile][(tilerow * TilePixWidth) + cntpix];
//                }
//            }
//        };

//        size_t cntoutpix = 0;
//        size_t curtile = 0;
//        for( ; cntoutpix < tiled.size(); )
//        {
//            for( size_t cnttilerow =0; cnttilerow < (TilePixHeight); ++cnttilerow )
//            {
//                lambdaCopyLineTiles(curtile, cnttilerow, cntoutpix);
//            }
//            curtile += NbTilesPerRow;
//        }

//        QImage img((unsigned char *)tiled.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
//        img.setColorTable(colors);
////        img.save("./aftertile.png", "png");
//        return QPixmap::fromImage(img, Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
//    }



    QVector<QRgb> ConvertSpritePalette(std::vector<uint32_t> & colors)
    {
        QVector<QRgb> outcol;
        outcol.reserve(colors.size());

        for( const auto & acol : colors )
            outcol.push_back( QRgb(acol >> 8) | 0xFF000000 ); //Force alpha to 255
        return std::move(outcol);
    }

    std::vector<uint32_t> ConvertSpritePaletteFromQt(const QVector<QRgb> & colors)
    {
        std::vector<uint32_t> outcol;
        outcol.reserve(colors.size());

        for( const auto & acol : colors )
            outcol.push_back( (acol << 8) | 0x00000080 ); //Force alpha to 0x80
        return std::move(outcol);
    }

    std::vector<uint8_t> TileFromImg( const QImage & src )
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
//        std::vector<uint8_t> out;
//        out.reserve(src.byteCount());
//        auto itbackins = std::back_inserter(out);

//        auto lambdacopytile = [&]( size_t begx, size_t begy, auto itout )
//        {
//            for( size_t x = begx; (x-begx) < TileWidth; ++x )
//            {
//                for(size_t y = begy; (y-begy) < TileHeight; ++y)
//                {
//                    *itout = static_cast<uint8_t>(src.pixelIndex(x,y));
//                }
//            }
//        };

//        for( size_t y = 0; y < src.height(); y+=TileHeight )
//        {
//            for(size_t x = 0; x < src.width(); x+=TileWidth)
//            {
//                lambdacopytile(x, y, itbackins);
//            }
//        }

//        return std::move(out);


        const size_t imgnbtileswidth  = src.width()  / TileWidth;
        const size_t imgnbtilesheight = src.height() / TileHeight;
        const size_t NbPixelsPerTile  = (TileWidth * TileHeight);
        const size_t NbPixels         = (imgnbtileswidth * imgnbtilesheight) * NbPixelsPerTile;
        std::vector<uint8_t> out(NbPixels, 0);

        //std::vector<std::vector<uint8_t>> tiles( imgnbtileswidth * imgnbtilesheight, std::vector<uint8_t>(TileWidth * TileHeight, 0) );

        for( size_t y = 0; y < src.height(); ++y )
        {
            for( size_t x = 0; x < src.width(); ++x )
            {
                size_t rowtile = (y / TileHeight);
                size_t coltile = (x / TileWidth);
                size_t tile = ((rowtile * imgnbtileswidth) + coltile);
                size_t tileX = x % TileWidth;
                size_t tileY = y % TileHeight;

                size_t pixel = (tile * NbPixelsPerTile) + (tileX + (tileY * TileWidth));
                Q_ASSERT(pixel < out.size());

                out[pixel] = src.pixelIndex(x, y);
                //tiles[tile][tileX + (tileY * TileWidth)] = src.pixel(x, y);
            }
        }
        return out;

//        auto lbdReadTile = [&](size_t pos)
//        {
//            size_t tilelocalpixelidx = pos % (TileWidth*TileHeight); //The pixel index within only the current tile
//            size_t curtilerow        = (tilelocalpixelidx / TileWidth);
//            size_t readpos = curtilerow * src.width(); //pixel to read  from the source
//            return readpos;
//        };

//        size_t pixelsz = 1; //bytes
//        //TODO: Calculate pixel size for the format

//        auto lbdReadPixel = [&](size_t pos)
//        {

//        };

//        for(size_t cntby = 0; cntby < src.byteCount(); cntby +=  )
    }

};
