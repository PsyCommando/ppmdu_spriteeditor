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

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect * out_area/*=nullptr*/, bool makebgtransparent/*=true*/, const Sprite* parentsprite) const
{
    qDebug() << "MFrame::AssembleFrame(): starting";
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

    //QVector<QRgb> pal = parentsprite->getPalette();
    //Make first color transparent
    if(makebgtransparent)
    {
        painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    }
    else
    {
        //Set first pal color as bg color!
        painter.setBackground( QColor(parentsprite->getPalette().front()) );
        painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

    }

    //Draw all the parts of the frame
    //const fmt::step_t * plast = nullptr; //A reference on the last valid frame, so we can properly copy it when encountering a -1 frame!
    for( const MFramePart * pwrap : m_container )
    {
        const fmt::step_t & part = pwrap->getPartData();

        //If frame index is completely out of range, something weird is up
//        if(part.getFrameIndex() > parentsprite->getImages().size())
//        {
//            //This means something is very off
//            qDebug() << "MFrame::AssembleFrame() : Got a reference to an image completely out of range!!";
//            break;
//        }
        qDebug() << "MFrame::AssembleFrame(): Drawing part!";
        QImage pix = pwrap->drawPart(parentsprite);

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

        int finalx = (part.getXOffset());
        int finaly = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset(); //simulate wrap-around past 256 Y
        painter.drawImage(xoffset + finalx, yoffset + finaly, pix );
        qDebug() << "MFrame::AssembleFrame(): Painted part to image!";
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

int MFrame::getFrameUID() const
{
    return nodeIndex();
}

#if 0
QVector<uint8_t> MFrame::generateTilesBuffer(const Sprite * spr, int uptopartidx) const
{
    //const int TILESZ = fmt::NDS_TILE_SIZE_8BPP * 4; //Always 8bpp since we converted image data to 8bpp when importing
    QVector<uint8_t> tilebuffer(fmt::NDS_OAM_MAX_NB_TILES * fmt::ImageDB::FRAME_TILE_SZ_BYTES, 0);
    const int lastEntry = uptopartidx != -1 ? uptopartidx : nodeChildCount();

    for(int i = 0; i < lastEntry; ++i)
    {
        const MFramePart * part = m_container[i];
        //We only place things when there's an actual valid frame index
        if(part->getFrameIndex() >= 0)
        {
            int tileoffset = part->getTileNum() * fmt::ImageDB::FRAME_TILE_SZ_BYTES;
            const Image * img = spr->getImage(part->getFrameIndex());
            if(!img)
                continue;
            auto itbuf = tilebuffer.begin();
            std::advance(itbuf, tileoffset);

            if((tileoffset + img->getByteSize()) > tilebuffer.size())
                throw BaseException("MFrame::generateTilesBuffer(): Data past end of buffer!");
            std::vector<uint8_t> raw;
            raw = img->getRaw();
            std::copy(raw.begin(), raw.end(), itbuf);
        }
    }
    return tilebuffer;
}
#endif

//const QVector<QImage> & MFrame::updateTileBuffer()
//{
//    qDebug() << "MFrame::updateTileBuffer(): Updating frame#" <<nodeIndex() <<"'s buffer..";
//    const Sprite * spr = findParentSprite();
//    if(!spr)
//        throw BaseException("MFrame::updateTileBuffer(): No parent sprite in hierarchy found!!");
//    m_cachedTileBuffer.resize(0);
//    QVector<uint8_t> raw = generateTilesBuffer(spr);
//    const int nbTiles = raw.size() / fmt::ImageDB::FRAME_TILE_SZ_BYTES;
//    int offset = 0;
//    for(int i = 0; i < nbTiles; ++i)
//    {
//        unsigned char * tilebeg = raw.data() + offset;
//        QImage tmp(tilebeg, fmt::ImageDB::FRAME_TILE_SZ_PIXELS, fmt::ImageDB::FRAME_TILE_SZ_PIXELS, QImage::Format::Format_Indexed8);
//        tmp.setColorTable(spr->getPalette());
//        m_cachedTileBuffer.push_back(tmp);
//        offset += fmt::ImageDB::FRAME_TILE_SZ_BYTES;
//    }
//    qDebug() << "MFrame::updateTileBuffer(): Updated!";
//    return m_cachedTileBuffer;
//}

//const QVector<QImage> &MFrame::getCachedTileBuffer() const
//{
//    return m_cachedTileBuffer;
//}

MFramePart * MFrame::getPartForCharBlockNum(int tilenum)
{
    Q_FOREACH(MFramePart * part, m_container)
    {
        if(!part->isPartReference() && tilenum == part->getCharBlockNum())
            return part;
    }
    return nullptr;
}
const MFramePart * MFrame::getPartForCharBlockNum(int tilenum)const
{
    return const_cast<MFrame*>(this)->getPartForCharBlockNum(tilenum);
}

int MFrame::calcCharBlocksLen() const
{
    int totallen = 0;
    int curtilenum = 0; //tile counter to keep track of what is the highest tile number so far
    for(int i = 0; i < nodeChildCount(); ++i)
    {
        const MFramePart * part = m_container[i];
        if(curtilenum < part->getCharBlockNum())
        {
            curtilenum = part->getCharBlockNum();
            totallen = part->getCharBlockNum() + part->getCharBlockLen();
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
            tileidrefs[part->getCharBlockNum()].push_back(part);
    }

    // Re-calculate the frame part tile number for each frame parts in the sprite
    uint16_t curtileofs = 0;
    for(int i = 0; i < nodeChildCount(); ++i)
    {
        MFramePart * part = m_container[i];
        if(part->isPartReference()) //Skip reference parts
            continue;

        qDebug() << "MFrame::optimizeTileUsage(): Changing part #" <<part->nodeIndex() <<" tile id from " <<part->getCharBlockNum() <<" to " <<curtileofs <<"!";
        part->setTileNum(curtileofs);

        //Update references if any
        auto itrefs = tileidrefs.find(curtileofs);
        if(itrefs != tileidrefs.end())
        {
            for( MFramePart * ref : itrefs->second)
            {
                qDebug() << "MFrame::optimizeTileUsage(): Ref #" <<ref->nodeIndex() <<" to part #" <<part->nodeIndex() <<" tile id changed from " <<ref->getCharBlockNum() <<" to " <<curtileofs <<"!";
                ref->setTileNum(curtileofs);
            }
        }

        curtileofs += part->getCharBlockLen(); //Add up the tiles to get the next available offset
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


//bool MFrame::_insertChildrenNode(TreeNode *node, int destrow, bool doupdate)
//{
//    bool result = parent_t::_insertChildrenNode(node, destrow);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_insertChildrenNodes(int row, int count, bool doupdate)
//{
//    bool result = parent_t::_insertChildrenNodes(row, count);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow, bool doupdate)
//{
//    bool result = parent_t::_insertChildrenNodes(nodes, destrow);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_removeChildrenNode(TreeNode *node, bool doupdate)
//{
//    bool result = parent_t::_removeChildrenNode(node);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_removeChildrenNodes(int row, int count, bool doupdate)
//{
//    bool result = parent_t::_removeChildrenNodes(row, count);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_removeChildrenNodes(const QList<TreeNode *> &nodes, bool doupdate)
//{
//    bool result = parent_t::_removeChildrenNodes(nodes);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_deleteChildrenNode(TreeNode *node, bool doupdate)
//{
//    bool result = parent_t::_deleteChildrenNode(node);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_deleteChildrenNodes(int row, int count, bool doupdate)
//{
//    bool result = parent_t::_deleteChildrenNodes(row, count);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_deleteChildrenNodes(const QList<TreeNode *> &nodes, bool doupdate)
//{
//    bool result = parent_t::_deleteChildrenNodes(nodes);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode, bool doupdate)
//{
//    bool result = parent_t::_moveChildrenNodes(row, count, destrow, destnode);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_moveChildrenNodes(const QModelIndexList &indices, int destrow, QModelIndex destparent, bool doupdate)
//{
//    bool result = parent_t::_moveChildrenNodes(indices, destrow, destparent);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}

//bool MFrame::_moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent, bool doupdate)
//{
//    bool result = parent_t::_moveChildrenNodes(nodes, destrow, destparent);
//    if(doupdate)
//        updateTileBuffer();
//    return result;
//}
