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

    QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const QByteArray & src, const QVector<QRgb> & colors )
    {
        static const size_t TilePixWidth  = 8;
        static const size_t TilePixHeight = 8;
        const size_t NbPixelsPerTile    = TilePixWidth * TilePixHeight;
        const size_t NbTilesPerRow      = pixwidth / TilePixWidth;
        QByteArray tiled;
        tiled.resize(src.size());

//        QImage debugimg((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
//        debugimg.setColorTable(colors);
//        debugimg.save("./beforetile.png", "png");

        QVector<QByteArray> tiles( src.size() / NbPixelsPerTile );
        size_t cntcurtile = 0;
        for( size_t cntpix = 0; cntpix < src.size(); )
        {
            tiles[cntcurtile].resize(NbPixelsPerTile);
            for(size_t cnttilepix = 0; cnttilepix < NbPixelsPerTile && cntpix < src.size(); ++cnttilepix, ++cntpix)
                tiles[cntcurtile][cnttilepix] = src[cntpix];
            ++cntcurtile;
        }

        //debug
//        for( size_t cntimage = 0; cntimage < tiles.size(); ++cntimage )
//        {
//            QImage atile((unsigned char *)tiles[cntimage].data(), 8, 8, QImage::Format_Indexed8 );
//            atile.setColorTable(colors);
//            atile.save( QString("./tile%1.png").arg(cntimage), "png");
//        }

        auto lambdaCopyLineTiles= [&]( size_t firsttileid, size_t tilerow, size_t & outpixpos )
        {
            for( size_t cnttile = 0; cnttile < NbTilesPerRow; ++cnttile )
            {
                for(size_t cntpix = 0; cntpix < TilePixWidth && outpixpos < tiled.size(); ++cntpix, ++outpixpos )
                {
                    tiled[outpixpos] = tiles[firsttileid + cnttile][(tilerow * TilePixWidth) + cntpix];
                }
            }
        };

        size_t cntoutpix = 0;
        size_t curtile = 0;
        for( ; cntoutpix < tiled.size(); )
        {
            for( size_t cnttilerow =0; cnttilerow < (TilePixHeight); ++cnttilerow )
            {
                lambdaCopyLineTiles(curtile, cnttilerow, cntoutpix);
            }
            curtile += NbTilesPerRow;
        }

//        size_t cnttile = 0;
//        for( size_t cntpix = 0; cntpix < src.size(); /*++cntpix*/ ++cnttile )
//        {

//            for( size_t cnttilepix = 0; cnttilepix < NbPixelsPerTile && cntpix < src.size(); ++cnttilepix/*, ++cntpix*/ )
//            {
//                size_t currow = cnttilepix / TilePixWidth;  //The row of the tile we're currently reading from (0,1,2,3)
//                size_t curdestimgrow = currow * pixwidth;   //The index of the first pixel of the image row we'll place the current tile's row at
//                for(size_t cnttilepixrow = 0; cnttilepixrow < TilePixWidth && (cnttilepix < NbPixelsPerTile && cntpix < src.size()); ++cnttilepixrow, ++cntpix)
//                {
//                    size_t curdestpixel = curdestimgrow + (cnttile * TilePixWidth);
//                    tiled[curdestpixel] = src[cntpix];
//                }
//            }

//        }

        QImage img((unsigned char *)tiled.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
        img.setColorTable(colors);
//        img.save("./aftertile.png", "png");
        return QPixmap::fromImage(img, Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
    }



    QVector<QRgb> ConvertSpritePalette(std::vector<uint32_t> & colors)
    {
        QVector<QRgb> outcol;
        outcol.reserve(colors.size());

        for( const auto & acol : colors )
            outcol.push_back( QRgb(acol >> 8) | 0xFF000000 ); //Force alpha to 255
        return std::move(outcol);
    }

};
