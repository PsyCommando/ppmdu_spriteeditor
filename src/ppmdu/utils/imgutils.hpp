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
//========================================
// Palette Helpers
//========================================
    QVector<QRgb>           ConvertSpritePalette      (const std::vector<uint32_t>  & colors);
    std::vector<uint32_t>   ConvertSpritePaletteFromQt(const QVector<QRgb>          & colors);

//========================================
// Common Pixel Convertion
//========================================

    inline std::pair<uint8_t, uint8_t> Pixel4bppTo8bpp(uint8_t p)
    {
        return std::make_pair(p & 0x0F, (p >> 4) & 0x0F);
    }
    inline uint8_t Pixel8bppTo4bpp(uint8_t p1, uint8_t p2)
    {
        return (p1 & 0x0F) | ((p2 << 4) & 0xF0);
    }

    template<typename _init,typename _outit>
        _init Pixel4bppTo8bpp(_init itwhere, _init itend, _outit & itout)
    {
        const auto result = Pixel4bppTo8bpp(utils::readBytesAs<uint8_t>(itwhere, itend));
        itout = utils::writeBytesFrom(result.first,  itout);
        itout = utils::writeBytesFrom(result.second, itout);
        return itwhere;
    }

    template<typename _init,typename _outit>
        _init Pixel8bppTo4bpp(_init itwhere, _init itend, _outit & itout)
    {
        //Consts because we can't rely on the ordering of operations in function parameters
        const uint8_t p1 = utils::readBytesAs<uint8_t>(itwhere, itend);
        const uint8_t p2 = utils::readBytesAs<uint8_t>(itwhere, itend);
        itout = utils::writeBytesFrom(Pixel8bppTo4bpp(p1, p2), itout);
        return itwhere;
    }

//========================================
// Tiled Container Access
//========================================

    //
    // Pixel Index Getter
    //
    size_t GetPixelIndexFromRawTiled8bpp(size_t x, size_t y, size_t width, size_t tilew = 8, size_t tileh = 8);
    size_t GetPixelIndexFromRawTiled4bpp(size_t x, size_t y, size_t width, size_t tilew = 8, size_t tileh = 8);

    //
    // Pixel Value Getters
    //

    template<class _ByteCntTy>
        uint8_t GetPixelFromRawTiled8bpp(_ByteCntTy cnt, size_t x, size_t y, size_t width, size_t tilew = 8, size_t tileh = 8);

    template<class _ByteCntTy>
        uint8_t GetPixelFromRawTiled4bpp(_ByteCntTy cnt, size_t x, size_t y, size_t width, size_t tilew = 8, size_t tileh = 8);

//========================================
// Container Pixel Depth Conversion
//========================================

    QByteArray           Expand4BppTo8Bpp(const QByteArray & src);
    std::vector<uint8_t> Expand4BppTo8Bpp(const std::vector<uint8_t> & src);

    std::vector<uint8_t> Reduce8bppTo4bpp( const std::vector<uint8_t> & src );
    std::vector<uint8_t> Reduce8bppTo4bpp(const QImage &src );


//========================================
// Container Pixel Tiling Conversion
//========================================
    template<class _ByteCntTy>
        _ByteCntTy Untile8bpp( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src)
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
        _ByteCntTy untiled;
        untiled.resize(src.size());

        for(size_t y = 0; y < pixheight; ++y)
        {
            for(size_t x = 0; x < pixwidth; ++x)
            {
                const size_t pixelin = GetPixelIndexFromRawTiled8bpp(x,y,pixwidth, TileWidth, TileHeight);
                const size_t pixelout = (y * pixwidth + x);
                Q_ASSERT(pixelout < untiled.size());

                if(pixelin >= src.size())
                    untiled[pixelout] = 0; //if the input is too small just zero out the extra!
                else
                    untiled[pixelout] = src[pixelin];
            }
        }

        return std::move(untiled);
    }

    template<class _ByteCntTy>
        _ByteCntTy Untile4bpp( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src)
    {
        static const size_t TileWidth  = 8;
        static const size_t TileHeight = 8;
        _ByteCntTy untiled;
        untiled.resize(src.size());

        for(size_t y = 0; y < pixheight; ++y)
        {
            for(size_t x = 0; x < pixwidth; x+=2)
            {
                const size_t pixelin = GetPixelIndexFromRawTiled4bpp(x,y,pixwidth, TileWidth, TileHeight);
                const size_t pixelout = (y * pixwidth + x) / 2; //divide by 2 because 4bpp
                Q_ASSERT(pixelout < untiled.size());

                if(pixelin >= src.size())
                    untiled[pixelout] = 0; //if the input is too small just zero out the extra!
                else
                    untiled[pixelout] = src[pixelin];
            }
        }
        return untiled;
    }

    template<class _ByteCntTy>
        QPixmap Untile8bppIntoPixmap( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>() )
    {
        return std::move(Raw8bppToPixmap( pixwidth, pixheight, Untile8bpp(pixwidth, pixheight, src, colors), colors));
    }

    std::vector<uint8_t> Tile8bppFromImg(const QImage & src);

    std::vector<uint8_t> Tile8bpp(const std::vector<uint8_t> & src, int w, int h);
    std::vector<uint8_t> Tile4bpp(const std::vector<uint8_t> & src, int w, int h);

//========================================
// Container Conversion
//========================================
    template<class _ByteCntTy>
        QPixmap Raw4bppToPixmap( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>())
    {
        return QPixmap::fromImage(Raw4bppToImg(pixwidth, pixheight, src, colors), Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
    }

    template<class _ByteCntTy>
        QPixmap Raw8bppToPixmap( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>())
    {
        return QPixmap::fromImage(Raw8bppToImg(pixwidth, pixheight, src, colors), Qt::ColorOnly | Qt::ThresholdDither | Qt::AvoidDither);
    }

    template<class _ByteCntTy>
        QImage Raw4bppToImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>())
    {
        const std::vector<uint8_t> expanded = Expand4BppTo8Bpp(src);
        QImage img((unsigned char *)expanded.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
        img.setColorTable(colors);
        return img.copy(); //Must return a copy, otherwise cause copy elision to moves the current image and creates bad references to the expanded buffer!
    }

    template<class _ByteCntTy>
        QImage Raw8bppToImg( unsigned int pixwidth, unsigned int pixheight, const _ByteCntTy & src, const QVector<QRgb> & colors = QVector<QRgb>())
    {
        QImage img((unsigned char *)src.data(), pixwidth, pixheight, QImage::Format_Indexed8 );
        img.setColorTable(colors);
        return img.copy(); //Must return a copy, otherwise cause copy elision to moves the current image and creates bad references to the src buffer!
    }

    std::vector<uint8_t> ImgToRaw4bpp(const QImage & src);
    std::vector<uint8_t> ImgToRaw8bpp(const QImage & src);

};

#endif // IMGUTILS_HPP
