#include "frame.hpp"
#include <QImage>

#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/image.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/framescontainer.hpp>
#include <src/data/sprite/framesdelegate.hpp>

const char *                ElemName_Frame = "Frame";
const size_t                FramesHeaderNBColumns = static_cast<unsigned int>(eFramesColumnsType::HeaderNBColumns);
const std::vector<QString>  FramesHeaderColumnNames
{
    QString(("")),

    //The rest below is for the parts/step to assemble the frame!
    QString(("Img ID")),
    QString(("Tile ID")),
    QString(("Palette")),
    QString(("Unk#0")),
    QString(("Offset")),
    QString(("Flip")),
    QString(("Rotation & Scaling")),
    QString(("Mosaic")),
    QString(("Mode")),
    QString(("Priority")),
};

//=====================================================================================
//  MFrame
//=====================================================================================
const char * MFrame::PropPartID = "framePartID";

MFrame::MFrame(TreeElement *parent)
    :paren_t(parent)
{
    m_pdelegate.reset(new MFrameDelegate(this));
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame::MFrame(const MFrame &cp)
    :paren_t(cp)
{
    m_pdelegate.reset(new MFrameDelegate(this));
    operator=(cp);
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame::MFrame(MFrame &&mv)
    :paren_t(mv)
{
    m_pdelegate.reset(new MFrameDelegate(this));
    operator=(mv);
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame::~MFrame()
{

}

MFrame &MFrame::operator=(const MFrame &cp)
{
    paren_t::operator=(cp);
    m_pdelegate.reset(new MFrameDelegate(this));
    return *this;
}

MFrame &MFrame::operator=(MFrame &&mv)
{
    paren_t::operator=(mv);
    m_pdelegate.reset(new MFrameDelegate(this));
    return *this;
}

void MFrame::clone(const TreeElement *other)
{
    const MFrame * ptr = static_cast<const MFrame*>(other);
    if(!ptr)
        throw std::runtime_error("MFrame::clone(): other is not a MFrame!");
    (*this) = *ptr;
}

bool MFrame::operator==(const MFrame &other) const
{
    return this == &other;
}

bool MFrame::operator!=(const MFrame &other) const
{
    return !operator==(other);
}

void MFrame::importFrame(const fmt::ImageDB::frm_t &frms)
{
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, frms.size());

    auto itparts = frms.begin();
    for( size_t cntid = 0; cntid < frms.size(); ++cntid, ++itparts )
    {
        m_container[cntid].importPart(*itparts);
    }
}

fmt::ImageDB::frm_t MFrame::exportFrame() const
{
    fmt::ImageDB::frm_t lst;
    for(const MFramePart & part : m_container)
        lst.push_back(part.exportPart());
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
    return &(m_container[id].getPartData());
}

const fmt::step_t *MFrame::getPart(int id)const
{
    return const_cast<MFrame*>(this)->getPart(id);
}

Sprite *MFrame::parentSprite()
{
    return static_cast<FramesContainer*>(parentNode())->parentSprite();
}

QPixmap MFrame::AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect *out_area) const
{
    return qMove( QPixmap::fromImage(AssembleFrame(xoffset, yoffset, cropto, out_area)) );
}

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect * out_area, bool makebgtransparent) const
{
    Sprite * pspr = const_cast<MFrame*>(this)->parentSprite();
    if(pspr->getPalette().empty()) //no point drawing anything..
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



    QVector<QRgb> pal = pspr->getPalette();
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
        painter.setBackground( QColor(pspr->getPalette().front()) );
        painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

    }

    //Draw all the parts of the frame
    const fmt::step_t * plast = nullptr; //A reference on the last valid frame, so we can properly copy it when encountering a -1 frame!
    for( const MFramePart & pwrap : m_container )
    {
        const fmt::step_t & part = pwrap.getPartData();
        //auto res = part.GetResolution();
        Image* pimg = pspr->getImage(part.getFrameIndex()); // returns null if -1 frame or out of range!
        QImage pix;
        if(!pimg && plast) //check for -1 frames
        {
            Image* plastimg = pspr->getImage(plast->getFrameIndex());
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

    for( const MFramePart & pwrap : m_container )
    {
        const fmt::step_t & part = pwrap.getPartData();

        auto imgres = part.GetResolution();
        int xoff = part.getXOffset();
        int yoff = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset();

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

int MFrame::nodeColumnCount() const {return FramesHeaderNBColumns;}

QVariant MFrame::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    if( column < 0 || column >= static_cast<int>(eFramesColumnsType::NBColumns) )
        return QVariant();

    switch(static_cast<eFramesColumnsType>(column))
    {
    case eFramesColumnsType::Preview:
        {
            if(role == Qt::DisplayRole)
                return QString("frame#%1").arg(nodeIndex());
            if(role != Qt::DecorationRole)
                break;
            return QVariant(AssembleFrame(0,0, QRect()));
        }
//    case eFramesColumnsType::TotalSize:
//        {
//            if(role != Qt::DisplayRole)
//                break;
//            ///#TODO: Write the total resolution of the assembled frame!
//            break;
//        }
    default:
        {
        }
    };

    return QVariant();
}

QVariant MFrame::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const MFramePart *part = static_cast<const MFramePart*>(getItem(index));
    if(part)
        return part->nodeData(index.column(), role);
    return QVariant();
}

QVariant MFrame::frameDataCondensed(int role) const
{
    if(role == Qt::DecorationRole)
    {
        return QPixmap::fromImage(AssembleFrame(0,0,QRect()));
    }
    else if(role == Qt::DisplayRole)
    {
        return QString("FrameID:%1").arg(getFrameUID());
    }
    else if(role == Qt::EditRole)
    {
        return getFrameUID();
    }
    else if(role == Qt::SizeHintRole)
    {
        QFontMetrics fm(QFont("Sergoe UI",9));
        return QSize(fm.horizontalAdvance(frameDataCondensed(role == Qt::DisplayRole).toInt()+32),
                     qMax(fm.height() + 32, calcFrameBounds().height()) );
    }
    return QVariant();
}

QVariant MFrame::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || static_cast<size_t>(section) >= FramesHeaderColumnNames.size() )
        return QVariant();

    if( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Orientation::Horizontal)
            return FramesHeaderColumnNames[section];
    }
    else if(role == Qt::SizeHintRole)
    {
        if( orientation == Qt::Orientation::Horizontal)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.horizontalAdvance(FramesHeaderColumnNames[section])+4, fm.height()+4);
        }
    }
    return QVariant();
}

bool MFrame::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || role != Qt::EditRole)
        return false;

    fmt::step_t *ppart = getPart(index.row());
    Q_ASSERT(ppart);
    bool bok = false;

    switch(static_cast<eFramesColumnsType>(index.column()))
    {
    case eFramesColumnsType::ImgID:
        {
            ppart->setFrameIndex(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::Offset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                ppart->setXOffset(offs.first);
                ppart->setYOffset(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert offset value to a QPair!\n");
            break;
        }
    case eFramesColumnsType::Flip:
        {
            bok = value.canConvert<QPair<bool,bool>>();
            if(bok)
            {
                QPair<bool,bool> offs = value.value<QPair<bool,bool>>();
                ppart->setVFlip(offs.first);
                ppart->setHFlip(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert flip value to a QPair!\n");
            break;
        }
    case eFramesColumnsType::RotNScaling:
        {
            ppart->setRotAndScaling(value.toBool());
            bok = true; //gotta set this to true because toBool doesn't return success or not
            break;
        }
    case eFramesColumnsType::PaletteID:
        {
            ppart->setPalNb(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::Mode:
        {
            ppart->setObjMode(static_cast<fmt::step_t::eObjMode>(value.toInt(&bok)));
            break;
        }
    case eFramesColumnsType::Priority:
        {
            ppart->setPriority(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::TileNum:
        {
            ppart->setTileNum(value.toInt(&bok));
            break;
        }

    //direct access columns
    case eFramesColumnsType::direct_HFlip:
        {
            ppart->setHFlip(value.toBool());
            break;
        }
    case eFramesColumnsType::direct_VFlip:
        {
            ppart->setVFlip(value.toBool());
            break;
        }
    case eFramesColumnsType::direct_XOffset:
        {
            ppart->setXOffset(value.toUInt(&bok));
            break;
        }
    case eFramesColumnsType::direct_YOffset:
        {
            ppart->setYOffset(value.toUInt(&bok));
            break;
        }
        //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
    default:
        return false;
    };

    if(bok && index.model())
    {
        const_cast<QAbstractItemModel*>(index.model())->dataChanged(index, index, QVector<int>{role});
    }
    return bok;
}

int MFrame::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return static_cast<int>(eFramesColumnsType::NBColumns);
}

int MFrame::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return static_cast<TreeElement*>(parent.internalPointer())->nodeChildCount();
    else
        return nodeChildCount();
}

MFrameDelegate &MFrame::itemDelegate()
{
    return *m_pdelegate.data();
}
