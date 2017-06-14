#ifndef IMGUTILS_HPP
#define IMGUTILS_HPP
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <QImage>
#include <QByteArray>
#include <vector>
#include "byteutils.hpp"


namespace utils
{

    //
    QVector<QRgb> ConvertSpritePalette(std::vector<uint32_t> & colors);
    std::vector<uint32_t> ConvertSpritePaletteFromQt(const QVector<QRgb> & colors);

    //
    QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors );

    //Split each bytes into 2 pixels
    QByteArray Expand4BppTo8Bpp(const QByteArray & src);
    std::vector<uint8_t> Expand4BppTo8Bpp(const std::vector<uint8_t> & src);

    std::vector<uint8_t> Reduce8bppTo4bpp( const std::vector<uint8_t> & src );
    std::vector<uint8_t> Reduce8bppTo4bpp(const QImage &src );

    //Assumes 8bpp
    //QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const QByteArray & src, const QVector<QRgb> & colors );
    template<class _ByteCntTy>
        _ByteCntTy Untile( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & /*colors*/ = QVector<QRgb>() )
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
        //const size_t NbPixelsPerTile    = TileWidth * TileHeight;
        //const size_t NbTilesPerRow      = pixwidth / TileWidth;
        _ByteCntTy untiled;
        untiled.resize(src.size());

//        QImage debugimg((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
//        debugimg.setColorTable(colors);
//        debugimg.save("./beforetile.png", "png");

//        QVector<QByteArray> tiles( src.size() / NbPixelsPerTile );
//        size_t cntcurtile = 0;
//        for( size_t cntpix = 0; cntpix < src.size(); )
//        {
//            tiles[cntcurtile].resize(NbPixelsPerTile);
//            for(size_t cnttilepix = 0; cnttilepix < NbPixelsPerTile && cntpix < src.size(); ++cnttilepix, ++cntpix)
//                tiles[cntcurtile][cnttilepix] = src[cntpix];
//            ++cntcurtile;
//        }

        //debug
//        for( size_t cntimage = 0; cntimage < tiles.size(); ++cntimage )
//        {
//            QImage atile((unsigned char *)tiles[cntimage].data(), 8, 8, QImage::Format_Indexed8 );
//            atile.setColorTable(colors);
//            atile.save( QString("./tile%1.png").arg(cntimage), "png");
//        }

        //This copies all pixels of the same line within several tiles on the same row
//        auto lambdaCopyLineTiles= [&]( size_t firsttileid, size_t tilerow, size_t & outpixpos )
//        {
//            //Double check here so if we don't have enough pixels to fill the expected resolution we don't crash
//            for( size_t cnttile = 0; cnttile < NbTilesPerRow && (firsttileid + cnttile) < tiles.size(); ++cnttile )
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


        const size_t imgnbtileswidth  = pixwidth  / TileWidth;
        //const size_t imgnbtilesheight = src.height() / TileHeight;
        const size_t NbPixelsPerTile  = (TileWidth * TileHeight);
        //const size_t NbPixels         = (imgnbtileswidth * imgnbtilesheight) * NbPixelsPerTile;
        //std::vector<uint8_t> out(NbPixels, 0);

        //std::vector<std::vector<uint8_t>> tiles( imgnbtileswidth * imgnbtilesheight, std::vector<uint8_t>(TileWidth * TileHeight, 0) );

        for( size_t y = 0; y < pixheight; ++y )
        {
            for( size_t x = 0; x < pixwidth; ++x )
            {
                size_t rowtile = (y / TileHeight);
                size_t coltile = (x / TileWidth);
                size_t tile = ((rowtile * imgnbtileswidth) + coltile);
                size_t tileX = x % TileWidth;
                size_t tileY = y % TileHeight;

                size_t pixelin  = (tile * NbPixelsPerTile) + (tileX + (tileY * TileWidth));
                size_t pixelout = (y * pixwidth + x);
                //Q_ASSERT(pixelin  < src.size());
                Q_ASSERT(pixelout < untiled.size());

                if( pixelin >= src.size() )
                    untiled[pixelout] = 0; //Handle images that didn't get an accurate resolution assigned
                else
                    untiled[pixelout] = src[pixelin];
                //tiles[tile][tileX + (tileY * TileWidth)] = src.pixel(x, y);
            }
        }


        return std::move(untiled);
    }

    template<class _ByteCntTy>
        QPixmap RawToPixmap( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        QImage img((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
        img.setColorTable(colors);
        return QPixmap::fromImage(img, Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
    }

    template<class _ByteCntTy>
        QImage RawToImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        QImage img((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
        img.setColorTable(colors);
        return std::move(img);
    }

    std::vector<uint8_t> ImgToRaw(const QImage & src);

    template<class _ByteCntTy>
        QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        return std::move( RawToPixmap( pixwidth, pixheight, Untile(pixwidth, pixheight, src, colors), colors) );
    }



    std::vector<uint8_t> TileFromImg( const QImage & src );

};

#endif // IMGUTILS_HPP
