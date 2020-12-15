#ifndef SPRITE_RENDERER_HPP
#define SPRITE_RENDERER_HPP
#include <QObject>
#include <QPixmap>
#include <src/ui/rendering/animated_sprite_item.hpp>
#include <src/data/sprite/sprite_container.hpp>
#include <src/utility/baseqtexception.hpp>


//------------ Exceptions ------------

/*
 * Thrown when the sequence that's being accessed doesn't exist.
*/
class ExBadAnimSequence : public BaseException{public: using BaseException::BaseException;};


//------------ SpriteRenderer ------------
/*
 * This class is created once per opened sprite container.
 * Its job is to locate and render specific things from the contained sprites
*/
class SpriteRenderer : public QObject
{
    Q_OBJECT
public:
    using animseqid_t   = fmt::animseqid_t;
    using sprid_t       = SpriteContainer::sprid_t;
    virtual ~SpriteRenderer();

    //Sprite rendering
    QPair<QVector<cachedframe>,QVector<QColor>> RenderSequence(const Sprite & sprt, SpriteRenderer::animseqid_t seqid)const;
    QVector<cachedframe>    RenderFrames(const Sprite & sprt, animseqid_t seqid)const;

    //Convert a palette from a sprite QRgb palette to a QColor palette
    QVector<QColor>         ConvertPalette(const Sprite & sprt)const;

    //Renders a palette flatly on an image. colperrow decides how many colors to put on a single row of the rendered palette. swatchsize sets how big the squares for each colors are
    QImage                  RenderPalette(const Sprite & sprt, size_t colperrow = 16, size_t swatchsize = 16)const;

    //Export

    //frameswidth : The amount of frames to fit in a single row. -1 for automatic
    QImage RenderAnimationSheet(const Sprite & sprt)const;
};


#endif // SPRITE_RENDERER_HPP
