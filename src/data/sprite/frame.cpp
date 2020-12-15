#include "frame.hpp"
#include <QImage>

#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/image.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/framescontainer.hpp>
#include <src/data/sprite/models/framepart_delegate.hpp>

#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>

//=====================================================================================
//  MFrame
//=====================================================================================
const char * MFrame::PropPartID = "framePartID";

MFrame::~MFrame()
{}

bool MFrame::operator==(const MFrame &other) const
{
    return this == &other;
}

bool MFrame::operator!=(const MFrame &other) const
{
    return !operator==(other);
}

MFramePart *MFrame::appendNewFramePart()
{
    int insertidx = nodeChildCount();
    _insertChildrenNodes(insertidx, 1);
    return m_container[insertidx];
}


void MFrame::importFrame(const fmt::ImageDB::frm_t &frms)
{
//    getModel()->removeRows(0, nodeChildCount());
//    getModel()->insertRows(0, frms.size());

    //Resize container
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, frms.size());

    auto itparts = frms.begin();
    for( size_t cntid = 0; cntid < frms.size(); ++cntid, ++itparts )
    {
        m_container[cntid]->importPart(*itparts);
    }
}

fmt::ImageDB::frm_t MFrame::exportFrame() const
{
    fmt::ImageDB::frm_t lst;
    for(const MFramePart * part : m_container)
        lst.push_back(part->exportPart());
    return lst;
}

fmt::step_t *MFrame::getPart(int id)
{
    if(id < 0)
    {
        qDebug("MFrame::getPart(): Got negative part ID!\n");
        return nullptr;
    }
    else if( id >= m_container.size())
    {
        qCritical("MFrame::getPart(): Got partid out of range!(%d)\n", id);
    }
    return &(m_container[id]->getPartData());
}

const fmt::step_t *MFrame::getPart(int id)const
{
    return const_cast<MFrame*>(this)->getPart(id);
}

bool MFrame::ClearImageRefs(const QModelIndexList &indices, bool bdelete)
{
    int matches = 0;
    for(int i = 0; i < nodeChildCount(); )
    {
        MFramePart * p = static_cast<MFramePart *>(nodeChild(i));
        bool bgotmatch = false;
        for(const QModelIndex & idx : indices)
        {
            Image * img = static_cast<Image*>(idx.internalPointer());
            if(static_cast<fmt::frmid_t>(img->getImageUID()) == p->getFrameIndex())
            {
                bgotmatch = true;
                break;
            }
        }

        //remove part if we found an index
        if(bgotmatch)
        {
            if(bdelete)
                _removeChildrenNodes(i,1);
            else
                p->setFrameIndex(-1); //Just set it to -1
            ++matches;
        }
        else
            ++i;
    }
    return matches > 0;
}

QPixmap MFrame::AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect *out_area, const Sprite* parentsprite) const
{
    return qMove( QPixmap::fromImage(AssembleFrame(xoffset, yoffset, cropto, out_area, true, parentsprite)) );
}

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect * out_area/*=nullptr*/, bool makebgtransparent/*=true*/, const Sprite* parentsprite) const
{
    if(parentsprite->getPalette().empty()) //no point drawing anything..
        return QImage();

    //#TODO: Implement checks for the other paramters for a frame, and for mosaic and etc!
    QImage      imgres(512,512, QImage::Format_ARGB32_Premultiplied);
    QPainter    painter(&imgres);
    QRect       bounds = calcFrameBounds();

    //Try to make the bounds match cropto if smaller
    if( !cropto.isNull() )
    {
        if(bounds.x() > cropto.x())
            bounds.setX( cropto.x() );
        if(bounds.y() > cropto.y())
            bounds.setY( cropto.y() );

        if(bounds.width() < cropto.width())
            bounds.setWidth( cropto.width() );
        if(bounds.height() < cropto.height())
            bounds.setHeight( cropto.height() );
    }

    QVector<QRgb> pal = parentsprite->getPalette();
    //Make first color transparent
    if(makebgtransparent)
    {
        QColor firstcol(pal.front());
        firstcol.setAlpha(0);
        pal.front() = firstcol.rgba();
        painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    }
    else
    {
        //Set first pal color as bg color!
        painter.setBackground( QColor(parentsprite->getPalette().front()) );
        painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

    }

    //Draw all the parts of the frame
    const fmt::step_t * plast = nullptr; //A reference on the last valid frame, so we can properly copy it when encountering a -1 frame!
    for( const MFramePart * pwrap : m_container )
    {
        const fmt::step_t & part = pwrap->getPartData();
        //auto res = part.GetResolution();
        const Image* pimg = parentsprite->getImage(part.getFrameIndex()); // returns null if -1 frame or out of range!
        QImage pix;
        if(!pimg && plast) //check for -1 frames
        {
            const Image* plastimg = parentsprite->getImage(plast->getFrameIndex());
            pix = plastimg->makeImage(pal);
        }
        else if(pimg)
        {
            pix = pimg->makeImage(pal);
            plast = &part;
        }

        //pix.setMask(pix.createMaskFromColor( QColor(pspr->getPalette().front()), Qt::MaskMode::MaskInColor ));

        if(part.isHFlip())
            pix = pix.transformed( QTransform().scale(-1, 1) );
        if(part.isVFlip())
            pix = pix.transformed( QTransform().scale(1, -1) );

        int finalx = (part.getXOffset());
        int finaly = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset(); //simulate wrap-around past 256 Y
        painter.drawImage(xoffset + finalx, yoffset + finaly, pix );
    }

    if(out_area)
        *out_area = bounds;

//    imgres.save("./mframeassemble.png", "png");

//    imgres.copy( xoffset + bounds.x(),
//                        yoffset + bounds.y(),
//                        bounds.width(),
//                        bounds.height() ).save("./mframeassemble_cropped.png", "png");

    return imgres.copy( xoffset + bounds.x(),
                        yoffset + bounds.y(),
                        bounds.width(),
                        bounds.height() );
}

QRect MFrame::calcFrameBounds() const
{
    int smallestx = 512;
    int biggestx = 0;
    int smallesty  = 512;
    int biggesty  = 0;

    for( const MFramePart * pwrap : m_container )
    {
        const fmt::step_t & part = pwrap->getPartData();

        auto imgres = part.GetResolution();
        int xoff = part.getXOffset();
        int yoff = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset(); //wrap around handling

        if( xoff < smallestx)
            smallestx = xoff;
        if( (xoff + imgres.first) >= biggestx )
            biggestx = (xoff + imgres.first);

        if( yoff < smallesty)
            smallesty = yoff;
        if( (yoff + imgres.second) >= biggesty )
            biggesty = (yoff + imgres.second);
    }

    return QRect( smallestx, smallesty, (biggestx - smallestx), (biggesty - smallesty) );
}

TreeNode *MFrame::clone() const
{
    return new MFrame(*this);
}

eTreeElemDataType MFrame::nodeDataTy() const
{
    return eTreeElemDataType::frame;
}

const QString &MFrame::nodeDataTypeName() const
{
    return ElemName_Frame;
}
