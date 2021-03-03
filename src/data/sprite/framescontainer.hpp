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
    FramesContainer(TreeNode * sprite);
    FramesContainer(const FramesContainer & cp);
    FramesContainer(FramesContainer && mv);
    FramesContainer &operator=(const FramesContainer &cp);
    FramesContainer &operator=(FramesContainer &&mv);

    ~FramesContainer();
    TreeNode * clone()const override;

    MFrame *        getFrame(fmt::frmid_t id);
    const MFrame *  getFrame(fmt::frmid_t id)const;
    bool ClearImageReferences(const QModelIndexList & indices, bool bdelete = false);

    //Returns the highest amount of blocks used by a single frame out of all of them
    uint16_t getMaxBlocksUsage()const;

    //Create and append a new MFrame, return pointer to it
    MFrame * appendNewFrame();

    void importFrames( const fmt::ImageDB::frmtbl_t & frms );
    fmt::ImageDB::frmtbl_t exportFrames();
    bool nodeIsMutable()const override;

    // TreeNode interface
public:
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;
};

#endif // FRAMESCONTAINER_HPP
