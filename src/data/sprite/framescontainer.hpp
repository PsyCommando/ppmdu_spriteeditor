#ifndef FRAMESCONTAINER_HPP
#define FRAMESCONTAINER_HPP
#include <src/data/treeelem.hpp>
#include <src/data/sprite/frame.hpp>

extern const char * ElemName_FrameCnt;
//*******************************************************************
//  FramesContainer
//*******************************************************************
// Category node for all the meta-frames in the current sprite!
class FramesContainer : public BaseTreeContainerChild<&ElemName_FrameCnt, MFrame>
{
public:

    FramesContainer( TreeElement * parent );
    ~FramesContainer();
    void clone(const TreeElement *other);

    MFrame *        getFrame(fmt::frmid_t id);
    const MFrame *  getFrame(fmt::frmid_t id)const;

    void importFrames( const fmt::ImageDB::frmtbl_t & frms );
    fmt::ImageDB::frmtbl_t exportFrames();

    //Elem data
    QVariant nodeData(int column, int role) const override;
    Sprite * parentSprite();

    //Model data
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool    nodeIsMutable()const override    {return false;}
    int     nodeColumnCount() const override {return FramesHeaderNBColumns;}
};

#endif // FRAMESCONTAINER_HPP
