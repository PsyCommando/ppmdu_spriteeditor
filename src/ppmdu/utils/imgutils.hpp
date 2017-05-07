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

    //
    QPixmap PaintPaletteToPixmap( const QVector<QRgb> & colors );

    //Split each bytes into 2 pixels
    QByteArray Expand4BppTo8Bpp(const QByteArray & src);

    //Assumes 8bpp
    QPixmap UntileIntoImg( unsigned int pixwidth, unsigned int pixheight, const QByteArray & src, const QVector<QRgb> & colors );

};

#endif // IMGUTILS_HPP
