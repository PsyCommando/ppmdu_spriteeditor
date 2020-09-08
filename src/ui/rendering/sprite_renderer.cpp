#include "sprite_renderer.hpp"

QRect FindLargestFrameBounds(const AnimSequence & seq, const Sprite & sprt)
{
    QRect boundsbiggest;

    int x1 = 512;
    int y1 = 512;
    int x2 = 0;
    int y2 = 0;
    //First pass, find largest frame
    for(const auto & afrm : seq )
    {
        QRect  area;
        const MFrame * pfrm = sprt.getFrame(afrm.frmidx());
        if(pfrm != nullptr )
            area = pfrm->calcFrameBounds();

        if( x2 <= area.width() + area.x() )
            x2 = area.width() + area.x();
        if( y2 <= area.height() + area.y() )
            y2 = area.height() + area.y();

        if( x1 > area.x() )
            x1 = area.x();
        if( y1 > area.y() )
            y1 = area.y();
    }
    return QRect(QPoint(x1, y1), QPoint(x2, y2) );
}

SpriteRenderer::~SpriteRenderer()
{

}

QPair<QVector<cachedframe>, QVector<QColor> > SpriteRenderer::RenderSequence(const Sprite & sprt, SpriteRenderer::animseqid_t seqid) const
{
    return qMakePair(RenderFrames(sprt, seqid), ConvertPalette(sprt));
}

QVector<cachedframe> SpriteRenderer::RenderFrames(const Sprite & sprt, SpriteRenderer::animseqid_t seqid) const
{
    QVector<cachedframe> cachedframes;
    const AnimSequence  * pseq = sprt.getAnimSequence(seqid);

    if(pseq)
    {
        //Get largest frame resolution, get the diff for each smaller frames  and add half to align the imge.
        QRect boundsbiggest = FindLargestFrameBounds(*pseq, sprt);

        //Second pass, assemble and crop to largest frame
        for(const auto & afrm : (*pseq) )
        {
            QImage target;
            QRect  area;
            const MFrame * pfrm = sprt.getFrame(afrm.frmidx());
            //
            if(pfrm != nullptr )
                target = qMove(pfrm->AssembleFrame(afrm.xoffset(), afrm.yoffset(), boundsbiggest, &area));
            else
                qDebug("AnimatedSpriteItem::LoadSequence(): Got invalid frame index %d!\n", afrm.frmidx());

            cachedframe dest;
            dest.area     = area;
            dest.shadowx  = afrm.shadowx();
            dest.shadowy  = afrm.shadowy();
            dest.offsetx  = afrm.xoffset();
            dest.offsety  = afrm.yoffset();
            dest.duration = afrm.duration();
            dest.img      = QPixmap::fromImage(target);
            cachedframes.push_back(qMove(dest));
        }

        //Align all the frames so they're all center aligned to eachothers
        for(auto & afrm : cachedframes )
        {
            //try to align frames
            if( afrm.area.x() > boundsbiggest.x() )
                afrm.area.setX(afrm.area.x() + (afrm.area.x() - boundsbiggest.x()) ); //add the difference!
            if( afrm.area.y() > boundsbiggest.y() )
                afrm.area.setY(afrm.area.y() + (afrm.area.y() - boundsbiggest.y()) ); //add the difference!
        }
    }
    else
    {
        throw ExBadAnimSequence( QString("AnimatedSpriteItem::LoadSequence(): Invalid sequence id %d!\n").arg(seqid) );
    }
    return cachedframes;
}

QImage SpriteRenderer::RenderPalette(const Sprite &sprt, size_t colperrow, size_t swatchsize) const
{
    const QVector<QRgb> pal = sprt.getPalette();
    const size_t NbColors = pal.size();
//    const size_t NbRows = (NbColors % colperrow == 0)?
//                              (NbColors / colperrow) :
//                              ((NbColors / colperrow) + 1);
    int curY = 0;
    QGraphicsScene rendertarget;
    for(size_t cntcolr = 0; cntcolr < NbColors;)
    {
        for(size_t cntr = 0; (cntr < colperrow) && (cntcolr < NbColors); ++cntr, ++cntcolr)
        {
            QGraphicsRectItem * rect = rendertarget.addRect(0,0, swatchsize, swatchsize, QPen(), QBrush(pal[cntcolr]));//rendertarget.addPixmap( QPixmap::fromImage(dat.value<QImage>()) );
            rect->setPos( cntr * swatchsize, curY );
        }
        curY += colperrow;
    }

    QImage result(rendertarget.width(), rendertarget.height(), QImage::Format_Indexed8);
    QPainter paintr(&result);
    rendertarget.render(&paintr);
    return result;
}

QVector<QColor> SpriteRenderer::ConvertPalette(const Sprite & sprt) const
{
    QVector<QColor> converted;
    converted.reserve(sprt.getPalette().size());
    for(const QRgb & col : sprt.getPalette())
        converted.push_back(QColor(col));
    return converted;
}

QImage SpriteRenderer::RenderAnimationSheet(const Sprite &/*sprt*/) const
{
    QImage result;
    //#TODO
    return result;
}
