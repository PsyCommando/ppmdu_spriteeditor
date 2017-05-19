#ifndef IMGUTILS_HPP
#define IMGUTILS_HPP
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QVector>
#include <QImage>
#include <QByteArray>
#include <vector>


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

    std::vector<uint8_t> Reduce8bppTo4bpp( std::vector<uint8_t> & src );

    //Assumes 8bpp
    //QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const QByteArray & src, const QVector<QRgb> & colors );
    template<class _ByteCntTy>
        _ByteCntTy Untile( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        static const size_t TilePixWidth  = 8;
        static const size_t TilePixHeight = 8;
        const size_t NbPixelsPerTile    = TilePixWidth * TilePixHeight;
        const size_t NbTilesPerRow      = pixwidth / TilePixWidth;
        _ByteCntTy tiled;
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

        return std::move(tiled);
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

    template<class _ByteCntTy>
        QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        return std::move( RawToPixmap( pixwidth, pixheight, Untile(pixwidth, pixheight, src, colors), colors) );
    }



    std::vector<uint8_t> TileFromImg( const QImage & src );

};

#endif // IMGUTILS_HPP
