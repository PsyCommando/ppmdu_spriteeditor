#include "framescontainer.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/framepart_delegate.hpp>

const QString ElemName_FrameCnt = "Frames";

FramesContainer::~FramesContainer()
{

}

TreeNode * FramesContainer::clone()const
{
    return new FramesContainer(*this);
}

//QVariant FramesContainer::nodeData(int column, int role) const
//{
//    if( column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(ElemName());
//    return QVariant();
//}

QString FramesContainer::nodeDisplayName() const
{
    return nodeDataTypeName();
}

void FramesContainer::importFrames(const fmt::ImageDB::frmtbl_t &frms)
{
//    getModel()->removeRows(0, nodeChildCount());
//    getModel()->insertRows(0, frms.size());

    //Resize container
    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, frms.size());

    for( size_t cntid = 0; cntid < frms.size(); ++cntid )
        m_container[cntid]->importFrame(frms[cntid]);
}

fmt::ImageDB::frmtbl_t FramesContainer::exportFrames()
{
    fmt::ImageDB::frmtbl_t frms(nodeChildCount());
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
    {
        frms[cntid] = m_container[cntid]->exportFrame();
    }
    return frms;
}

MFrame *FramesContainer::getFrame(fmt::frmid_t id)
{
    return static_cast<MFrame*>(nodeChild(id));
}

const MFrame *FramesContainer::getFrame(fmt::frmid_t id) const
{
    return const_cast<FramesContainer*>(this)->getFrame(id);
}

bool FramesContainer::ClearImageReferences(const QModelIndexList &indices, bool bdelete)
{
    bool bfoundref = false;
    for(MFrame * frm : m_container)
    {
        bfoundref |= frm->ClearImageRefs(indices, bdelete);
    }
    return bfoundref;
}

MFrame *FramesContainer::appendNewFrame()
{
    int insertidx = nodeChildCount();
    _insertChildrenNodes(insertidx, 1);
    return m_container[insertidx];
}

