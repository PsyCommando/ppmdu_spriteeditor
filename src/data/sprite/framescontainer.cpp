#include "framescontainer.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/framepart_delegate.hpp>

const QString ElemName_FrameCnt = "Frames";

FramesContainer::FramesContainer(TreeNode *sprite)
    :TreeNodeWithChilds(sprite)
{}

FramesContainer::FramesContainer(const FramesContainer &cp)
    :TreeNodeWithChilds(cp)
{
    operator=(cp);
}

FramesContainer::FramesContainer(FramesContainer &&mv)
    :TreeNodeWithChilds(mv)
{
    operator=(mv);
}

FramesContainer &FramesContainer::operator=(const FramesContainer &cp)
{
    TreeNodeWithChilds::operator=(cp);
    return *this;
}

FramesContainer &FramesContainer::operator=(FramesContainer &&mv)
{
    TreeNodeWithChilds::operator=(mv);
    return *this;
}

FramesContainer::~FramesContainer()
{
}

TreeNode * FramesContainer::clone()const
{
    return new FramesContainer(*this);
}

QString FramesContainer::nodeDisplayName() const
{
    return nodeDataTypeName();
}

void FramesContainer::importFrames(const fmt::ImageDB::frmtbl_t &frms)
{
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

bool FramesContainer::nodeIsMutable() const {return false;}

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

uint16_t FramesContainer::getMaxBlocksUsage() const
{
    uint16_t largest = 0; // the largest amount of tiles used out of all frames
    for(const MFrame * frm : m_container)
    {
        uint16_t curfrmtotalsz = 0; //highest tile number + size used out of all steps
        for(const MFramePart * part : *frm)
        {
            const uint16_t curmax = part->getBlockNum() + part->getBlockLen();
            if(curmax > curfrmtotalsz)
                curfrmtotalsz = curmax;
        }
        if(curfrmtotalsz > largest)
            largest = curfrmtotalsz;
    }
    assert(largest != 0 || m_container.empty());
    return largest;
}

MFrame *FramesContainer::appendNewFrame()
{
    int insertidx = nodeChildCount();
    _insertChildrenNodes(insertidx, 1);
    return m_container[insertidx];
}
