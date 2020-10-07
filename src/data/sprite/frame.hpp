#ifndef FRAME_HPP
#define FRAME_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/framepart.hpp>

class Sprite;
extern const QString ElemName_Frame;

//*******************************************************************
//  MFrame
//*******************************************************************
// Represents an assembly of images.
class FramesContainer;
class MFrame : public TreeNodeWithChilds<MFramePart>
{
    friend class MFrameDelegate;
    friend class MFramePartDelegate;
    //Dynamic property for edit controls, so we can keep track of which part they edit!
    static const char * PropPartID;
    typedef TreeNodeWithChilds<MFramePart> paren_t;

public:
//    MFrame(FramesContainer * parent);
//    MFrame(const MFrame &cp);
//    MFrame(MFrame &&mv);

//    MFrame & operator=(const MFrame & cp);
//    MFrame & operator=(MFrame && mv);

    MFrame(TreeNode * framescnt)
        :TreeNodeWithChilds(framescnt)
    {}

    MFrame(const MFrame &cp)
        :TreeNodeWithChilds(cp)
    {}

    MFrame(MFrame &&mv)
        :TreeNodeWithChilds(mv)
    {}

    ~MFrame();

    MFrame & operator=(const MFrame & cp)
    {
        TreeNodeWithChilds::operator=(cp);
        return *this;
    }

    MFrame & operator=(MFrame && mv)
    {
        TreeNodeWithChilds::operator=(mv);
        return *this;
    }

    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

    bool operator==( const MFrame & other)const;
    bool operator!=( const MFrame & other)const;

    void importFrame(const fmt::ImageDB::frm_t & frms);
    fmt::ImageDB::frm_t exportFrame()const;

    fmt::step_t         *getPart(int id);
    const fmt::step_t   *getPart(int id)const;

    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect * out_area/*=nullptr*/, const Sprite* parentsprite) const;
    //cropto: A rectangle whose width and height will be used to crop the assembled frame. Mainly useful to get even-sized frames.
    //        A default QRect is ignored and the image will be cropped to our discretion.
    QImage  AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect *out_area/*=nullptr*/, bool makebgtransparent/*=true*/, const Sprite* parentsprite)const;
    QRect   calcFrameBounds()const;

    //For now UID is index!
    inline int getFrameUID()const {return nodeIndex();}
};

#endif // FRAME_HPP
