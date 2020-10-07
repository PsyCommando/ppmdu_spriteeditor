#ifndef FRAMESCONTAINER_HPP
#define FRAMESCONTAINER_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/treenodemodel.hpp>
#include <src/data/sprite/frame.hpp>

extern const QString ElemName_FrameCnt;
//*******************************************************************
//  FramesContainer
//*******************************************************************
// Category node for all the meta-frames in the current sprite!
class FramesContainer : public TreeNodeWithChilds<MFrame>
{
public:
    FramesContainer(TreeNode * sprite)
        :TreeNodeWithChilds(sprite)
    {}

    FramesContainer(const FramesContainer & cp)
        :TreeNodeWithChilds(cp)
    {}

    FramesContainer(FramesContainer && mv)
        :TreeNodeWithChilds(mv)
    {}

    FramesContainer &operator=(const FramesContainer &cp)
    {
        TreeNodeWithChilds::operator=(cp);
        return *this;
    }

    FramesContainer &operator=(FramesContainer &&mv)
    {
        TreeNodeWithChilds::operator=(mv);
        return *this;
    }

    ~FramesContainer();
    TreeNode * clone()const override;

    MFrame *        getFrame(fmt::frmid_t id);
    const MFrame *  getFrame(fmt::frmid_t id)const;

    void importFrames( const fmt::ImageDB::frmtbl_t & frms );
    fmt::ImageDB::frmtbl_t exportFrames();
    bool nodeIsMutable()const override {return false;}

    // TreeNode interface
public:
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
};

#endif // FRAMESCONTAINER_HPP
