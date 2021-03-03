#include "frame.hpp"
#include <QImage>
#include <QBitmap>

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

MFrame::MFrame(TreeNode *framescnt)
    :parent_t(framescnt)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

MFrame::MFrame(const MFrame &cp)
    :parent_t(cp)
{}

MFrame::MFrame(MFrame &&mv)
    :parent_t(mv)
{}

MFrame::~MFrame()
{}

MFrame &MFrame::operator=(const MFrame &cp)
{
    parent_t::operator=(cp);
    return *this;
}

MFrame &MFrame::operator=(MFrame &&mv)
{
    parent_t::operator=(mv);
    return *this;
}

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

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect * out_area, bool makebgtransparent, const Sprite* parentsprite) const
{
    qDebug() << "MFrame::AssembleFrame(): starting";
    if(parentsprite->getPalette().empty()) //no point drawing anything..
        return QImage();

    //#TODO: Implement checks for the other paramters for a frame, and for mosaic and etc!
    QImage                  imgres(512,512, QImage::Format_ARGB32_Premultiplied);
    QImage                  bg;
    QPainter                painter(&imgres);
    QRect                   bounds = calcFrameBounds();
    const QVector<QRgb> &   pal = parentsprite->getPalette();

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

    //Make first color transparent
    if(makebgtransparent)
    {
        painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    }
    else
    {
        //Set first pal color as bg color!
        //painter.setBackground(QColor(pal.front()));
        //painter.setBackgroundMode(Qt::BGMode::OpaqueMode);
        bg = QImage(512,512, QImage::Format_ARGB32_Premultiplied);
    }

    QPainter bgpainter(&bg);
    //Draw all the parts of the frame
    Q_FOREACH(const MFramePart * part, m_container)
    {
        //const fmt::step_t & pdat = part->getPartData();
        qDebug() << "MFrame::AssembleFrame(): Drawing part!";
        QPixmap pix = part->drawPartToPixmap(parentsprite);

//        const Image* pimg = parentsprite->getImage(part.getFrameIndex()); // returns null if -1 frame or out of range!
//        QImage pix;
//        if(!pimg && plast) //check for -1 frames
//        {
//            const Image* plastimg = parentsprite->getImage(plast->getFrameIndex());
//            pix = plastimg->makeImage(pal);
//        }
//        else if(pimg)
//        {
//            pix = pimg->makeImage(pal);
//            plast = &part;
//        }

        //pix.setMask(pix.createMaskFromColor( QColor(pspr->getPalette().front()), Qt::MaskMode::MaskInColor ));

 //       if(part.isHFlip())
//            pix = pix.transformed( QTransform().scale(-1, 1) );
//        if(part.isVFlip())
//            pix = pix.transformed( QTransform().scale(1, -1) );

        const int           finalx    = part->getXOffset();
        const int           finaly    = part->getYWrappedOffset(); //simulate wrap-around past 256 Y
        const QVector<QRgb> partpal   = part->getPartPalette(pal);
        const QColor        alphacol  = partpal.empty()? QRgb() : partpal.front();
        const QBitmap       blendmask = pix.createMaskFromColor(alphacol, Qt::MaskMode::MaskInColor);
        if(!makebgtransparent)
        {
            QRect bgrect = pix.rect();
            bgrect.moveTo(xoffset + finalx, yoffset + finaly);
            bgpainter.setBrush(alphacol);
            bgpainter.setPen(Qt::PenStyle::NoPen);
            bgpainter.drawRect(bgrect);
        }
        pix.setMask(blendmask);
        painter.drawPixmap(xoffset + finalx, yoffset + finaly, pix);
        qDebug() << "MFrame::AssembleFrame(): Painted part to image!";
    }
    painter.end(); //Release the image immediately after painting, so we don't get sigsev when destroying the qpainter

    if(!makebgtransparent)
    {
        qDebug() << "MFrame::AssembleFrame(): Merging background layer..!";
        //Draw the actual image pixels over the background, and slap the result into the output image
        bgpainter.drawImage(0,0, imgres, Qt::ImageConversionFlag::AutoColor);
        imgres = bg;
        qDebug() << "MFrame::AssembleFrame(): Merged!";
    }
    qDebug() << "MFrame::AssembleFrame(): Painted all parts!";

    if(out_area)
    {
        *out_area = bounds;
        qDebug() << "MFrame::AssembleFrame(): Set bounds!";
    }
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

    Q_FOREACH(const MFramePart * part, m_container)
    {
        const auto  imgres = part->GetResolution();
        const int   xoff = part->getXOffset();
        const int   yoff = part->getYWrappedOffset(); //wrap around handling

        if(xoff < smallestx)
            smallestx = xoff;
        if((xoff + imgres.first) >= biggestx)
            biggestx = (xoff + imgres.first);

        if(yoff < smallesty)
            smallesty = yoff;
        if((yoff + imgres.second) >= biggesty)
            biggesty = (yoff + imgres.second);
    }

    return QRect( smallestx, smallesty, (biggestx - smallestx), (biggesty - smallesty) );
}

int MFrame::getFrameUID() const
{
    return nodeIndex();
}

MFramePart * MFrame::getPartForBlockNum(int tilenum)
{
    Q_FOREACH(MFramePart * part, m_container)
    {
        if(!part->isPartReference() && tilenum == part->getBlockNum())
            return part;
    }
    return nullptr;
}
const MFramePart * MFrame::getPartForBlockNum(int tilenum)const
{
    return const_cast<MFrame*>(this)->getPartForBlockNum(tilenum);
}

int MFrame::calcCharBlocksLen() const
{
    int totallen = 0;
    int curtilenum = 0; //tile counter to keep track of what is the highest tile number so far
    for(int i = 0; i < nodeChildCount(); ++i)
    {
        const MFramePart * part = m_container[i];
        if(curtilenum < part->getBlockNum())
        {
            curtilenum = part->getBlockNum();
            totallen = part->getBlockNum() + part->getBlockLen();
        }
    }
    return totallen;
}

void MFrame::optimizeCharBlocksUsage()
{
    qDebug() << "MFrame::optimizeTileUsage(): Optimizing char blocks usage for frame#" <<nodeIndex();
    std::map<int, std::vector<MFramePart*>> tileidrefs; // -1 frame parts refering a given tile id

    //Gather refs and preserve them
    for(int i = 0; i < nodeChildCount(); ++i)
    {
        MFramePart * part = m_container[i];
        if(part->isPartReference())
            tileidrefs[part->getBlockNum()].push_back(part);
    }

    // Re-calculate the frame part tile number for each frame parts in the sprite
    uint16_t curtileofs = 0;
    for(int i = 0; i < nodeChildCount(); ++i)
    {
        MFramePart * part = m_container[i];
        if(part->isPartReference()) //Skip reference parts
            continue;

        qDebug() << "MFrame::optimizeTileUsage(): Changing part #" <<part->nodeIndex() <<" tile id from " <<part->getBlockNum() <<" to " <<curtileofs <<"!";
        part->setBlockNum(curtileofs);

        //Update references if any
        auto itrefs = tileidrefs.find(curtileofs);
        if(itrefs != tileidrefs.end())
        {
            for( MFramePart * ref : itrefs->second)
            {
                qDebug() << "MFrame::optimizeTileUsage(): Ref #" <<ref->nodeIndex() <<" to part #" <<part->nodeIndex() <<" tile id changed from " <<ref->getBlockNum() <<" to " <<curtileofs <<"!";
                ref->setBlockNum(curtileofs);
            }
        }

        curtileofs += part->getBlockLen(); //Add up the tiles to get the next available offset
    }
}

const Sprite *MFrame::findParentSprite() const
{
    const Sprite * spr = nullptr;
    const TreeNode * node = this;
    do
    {
        node = node->parentNode();
        if(node && node->nodeDataTypeName() == ElemName_Sprite)
        {
            qDebug() <<"MFrame::findParentSprite(): Found parent sprite#" <<node->nodeIndex() <<", for frame#" <<nodeIndex() <<"!";
            spr = static_cast<const Sprite*>(node);
            break;
        }
    }while(node);
    return spr;
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

bool MFrame::nodeShowChildrenOnTreeView() const
{
    return false;
}
