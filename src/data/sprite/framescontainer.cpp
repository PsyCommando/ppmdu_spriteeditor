#include "framescontainer.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/framesdelegate.hpp>

const char * ElemName_FrameCnt = "Frames";

FramesContainer::FramesContainer(TreeElement *parent)
    :BaseTreeContainerChild(parent)
{
    setNodeDataTy(eTreeElemDataType::frames);
}

FramesContainer::~FramesContainer()
{
    qDebug("FrameContainer::~FrameContainer()\n");
}

void FramesContainer::clone(const TreeElement *other)
{
    const FramesContainer * ptr = static_cast<const FramesContainer*>(other);
    if(!ptr)
        throw std::runtime_error("FramesContainer::clone(): other is not a FramesContainer!");
    (*this) = *ptr;
}

QVariant FramesContainer::nodeData(int column, int role) const
{
    if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

void FramesContainer::importFrames(const fmt::ImageDB::frmtbl_t &frms)
{
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, frms.size());

    for( size_t cntid = 0; cntid < frms.size(); ++cntid )
        m_container[cntid].importFrame(frms[cntid]);
}

fmt::ImageDB::frmtbl_t FramesContainer::exportFrames()
{
    fmt::ImageDB::frmtbl_t frms(nodeChildCount());
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
    {
        frms[cntid] = std::move(m_container[cntid].exportFrame());
    }
    return std::move(frms);
}

MFrame *FramesContainer::getFrame(fmt::frmid_t id)
{
    return static_cast<MFrame*>(nodeChild(id));
}

const MFrame *FramesContainer::getFrame(fmt::frmid_t id) const
{
    return const_cast<FramesContainer*>(this)->getFrame(id);
}

QVariant FramesContainer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const MFrame *frm = static_cast<const MFrame*>(getItem(index));
    return frm->nodeData(index.column(), role);
}

QVariant FramesContainer::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             static_cast<size_t>(section) < FramesHeaderNBColumns )
    {
        return FramesHeaderColumnNames[section];
    }
    return QVariant();
}

Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}
