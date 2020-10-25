#include "framepart.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>

const QString               ElemName_FramePart = "Frame Part";
const size_t                FramePartHeaderNBColumns = static_cast<unsigned int>(eFramePartColumnsType::HeaderNBColumns);
const std::vector<QString>  FramePartHeaderColumnNames
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

const QStringList FRAME_PART_PRIORITY_NAMES
{
    QString("0- Highest"),
    QString("1- High"),
    QString("2- Low"),
    QString("3- Lowest"),
};

const QStringList FRAME_PART_MODE_NAMES
{
    QString("Normal"),
    QString("Blended"),
    QString("Windowed"),
    QString("Bitmap"),
};

//*******************************************************************
//  MFramePart
//*******************************************************************

MFramePart::MFramePart(TreeNode *mframe)
    :partparent_t(mframe)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
    m_data.attr0 = 0;
    m_data.attr1 = 0;
    m_data.attr2 = 0;
    m_data.setXOffset(255);
    m_data.setYOffset(255);
    m_data.setFrameIndex(-1);
}

MFramePart::MFramePart(TreeNode *mframe, const fmt::step_t &part)
    :partparent_t(mframe), m_data(part)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

MFramePart::MFramePart(const MFramePart &cp)
    : partparent_t(cp.m_parentItem), utils::BaseSequentialIDGen<MFramePart>()
{
    operator=(cp);
}

MFramePart::MFramePart(MFramePart &&mv)
    : partparent_t(mv.m_parentItem), utils::BaseSequentialIDGen<MFramePart>()
{
    operator=(mv);
}

MFramePart::~MFramePart()
{
}

MFramePart &MFramePart::operator=(const MFramePart &cp)
{
    m_flags = cp.m_flags;
    m_data = cp.m_data;
    //we can't copy a unique ID, so don't do it
    return *this;
}

MFramePart &MFramePart::operator=(MFramePart &&mv)
{
    m_parentItem = mv.m_parentItem;
    mv.m_parentItem = nullptr;
    m_flags = mv.m_flags;
    m_data = qMove(mv.m_data);

    //Swap the ids around so we can take its id without having it recycled
    id_t oldid = m_id;
    m_id = mv.m_id;
    mv.m_id = oldid;
    return *this;
}

bool MFramePart::operator==(const MFramePart &other) const
{
    return m_data.attr0 == other.m_data.attr0 &&
           m_data.attr1 == other.m_data.attr1 &&
           m_data.attr2 == other.m_data.attr2;
}

bool MFramePart::operator!=(const MFramePart &other) const
{
    return !operator==(other);
}

TreeNode *MFramePart::clone() const
{
    return new MFramePart(*this);
}

eTreeElemDataType MFramePart::nodeDataTy() const
{
    return eTreeElemDataType::framepart;
}

const QString &MFramePart::nodeDataTypeName() const
{
    return ElemName_FramePart;
}

Qt::ItemFlags MFramePart::nodeFlags(int column) const
{
    if(column == static_cast<int>(eFramePartColumnsType::Preview) )
        return Qt::ItemFlags(m_flags).setFlag(Qt::ItemFlag::ItemIsEditable, false); //The preview is never editable!
    return m_flags;
}

QImage MFramePart::drawPart(Sprite * spr, bool transparencyenabled) const
{
    int imgindex = -1;

    if(m_data.getFrameIndex() < 0)
    {
        qInfo("MFramePart::drawPart(): was asked to draw a -1 frame!!\n");

        if(nodeIndex() > 0)
        {
            const MFrame * frm = static_cast<const MFrame*>(parentNode());
            if(!frm)
            {
                qWarning("MFramePart::drawPart(): Can't access parent frame!!\n");
                return QImage(); //Can't do much here!
            }

            //find previous non -1 frame part!
            for( int cntlkb = nodeIndex(); cntlkb >= 0; --cntlkb )
            {
                const fmt::step_t * p = frm->getPart(cntlkb);
                Q_ASSERT(p);
                if(p->getFrameIndex() >= 0)
                {
                    imgindex = p->getFrameIndex();
                    break;
                }
            }
        }

        //If we didn't find a replacement frame, just return an empty image.
        if(imgindex < 0)
            return QImage();
        //Otherwise continue with the valid index!
    }
    else
    {
        imgindex = m_data.getFrameIndex();
    }

    //auto res = m_data.GetResolution();
    QImage imgo;
    Q_ASSERT(spr);
    const Image * pimg = spr->getImage( imgindex );
    if(!pimg)
    {
        qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
        return QImage();
    }

    if(spr->unk13() == 1)
        qDebug("MFramePart::drawPart(): This part probably won't be drawn correctly, since it appears to be set to 1D mapping!\n");


    if(transparencyenabled)
    {
        QVector<QRgb> newpal = spr->getPalette();
        if(newpal.size() > 0)
            newpal.front() = (~(0xFF << 24)) & newpal.front(); //Format is ARGB
        imgo = pimg->makeImage(newpal);
    }
    else
    {
        imgo = pimg->makeImage(spr->getPalette());
    }

    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
    applyTransforms(imgo);
    return imgo;
}

void MFramePart::importPart(const fmt::step_t &part)
{
    m_data = part;
}

fmt::step_t MFramePart::exportPart() const
{
    return m_data;
}

fmt::step_t &MFramePart::getPartData()
{
    return m_data;
}

const fmt::step_t &MFramePart::getPartData() const
{
    return m_data;
}

void MFramePart::applyTransforms(QImage &srcimg) const
{
    srcimg = srcimg.transformed( QTransform().scale( m_data.isHFlip()? -1 : 1,
                                                    m_data.isVFlip()? -1 : 1) ) ;
}

