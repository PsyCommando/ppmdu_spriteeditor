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
    typedef TreeNodeWithChilds<MFramePart> parent_t;

public:
    MFrame(TreeNode * framescnt);
    MFrame(const MFrame &cp);
    MFrame(MFrame &&mv);
    ~MFrame();

    MFrame & operator=(const MFrame & cp);
    MFrame & operator=(MFrame && mv);

    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;
    bool                nodeShowChildrenOnTreeView()const override;

    bool operator==( const MFrame & other)const;
    bool operator!=( const MFrame & other)const;

    //appends a new framepart to the list and returns the pointer
    MFramePart * appendNewFramePart();

    void importFrame(const fmt::ImageDB::frm_t & frms);
    fmt::ImageDB::frm_t exportFrame()const;

    fmt::step_t         *getPart(int id);
    const fmt::step_t   *getPart(int id)const;
    bool ClearImageRefs(const QModelIndexList & indices, bool bdelete = false);

    QPixmap AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect * out_area/*=nullptr*/, const Sprite* parentsprite) const;
    //cropto: A rectangle whose width and height will be used to crop the assembled frame. Mainly useful to get even-sized frames.
    //        A default QRect is ignored and the image will be cropped to our discretion.
    QImage  AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect *out_area/*=nullptr*/, bool makebgtransparent/*=true*/, const Sprite* parentsprite)const;
    QRect   calcFrameBounds()const;

    //For now UID is index!
    int getFrameUID()const;

    //Generates a filed tile buffer based on this frame's sub-parts
    // Adds up tiles up to "uptopartidx". If its -1, adds all frames parts
    //QVector<uint8_t> generateTilesBuffer(const Sprite * spr, int uptopartidx = -1)const;

    int calcCharBlocksLen()const;

    //Function to assign tilenums in order to all the framparts
    void optimizeCharBlocksUsage();

    //Recurse through the hierarchy to find the parent sprite
    const Sprite * findParentSprite()const;

    //Obtain a part via tile number. Used internally for handling reference frame parts
    MFramePart *        getPartForCharBlockNum(int tilenum);
    const MFramePart *  getPartForCharBlockNum(int tilenum)const;

//    // TreeNode interface
public:
//    bool _insertChildrenNode(TreeNode *node, int destrow) override;
//    bool _insertChildrenNodes(int row, int count) override;
//    bool _insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow) override;
//    bool _removeChildrenNode(TreeNode *node) override;
//    bool _removeChildrenNodes(int row, int count) override;
//    bool _removeChildrenNodes(const QList<TreeNode *> &nodes) override;
//    bool _deleteChildrenNode(TreeNode *node) override;
//    bool _deleteChildrenNodes(int row, int count) override;
//    bool _deleteChildrenNodes(const QList<TreeNode *> &nodes) override;
//    bool _moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode) override;
//    bool _moveChildrenNodes(const QModelIndexList &indices, int destrow, QModelIndex destparent) override;
//    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent) override;

//    bool _insertChildrenNode(TreeNode *node, int destrow, bool doupdate = true);
//    bool _insertChildrenNodes(int row, int count, bool doupdate = true);
//    bool _insertChildrenNodes(const QList<TreeNode *> &nodes, int destrow, bool doupdate = true);
//    bool _removeChildrenNode(TreeNode *node, bool doupdate = true);
//    bool _removeChildrenNodes(int row, int count, bool doupdate = true);
//    bool _removeChildrenNodes(const QList<TreeNode *> &nodes, bool doupdate = true);
//    bool _deleteChildrenNode(TreeNode *node, bool doupdate = true);
//    bool _deleteChildrenNodes(int row, int count, bool doupdate = true);
//    bool _deleteChildrenNodes(const QList<TreeNode *> &nodes, bool doupdate = true);
//    bool _moveChildrenNodes(int row, int count, int destrow, TreeNode *destnode, bool doupdate = true);
//    bool _moveChildrenNodes(const QModelIndexList &indices, int destrow, QModelIndex destparent, bool doupdate = true);
//    bool _moveChildrenNodes(const QList<TreeNode *> &nodes, int destrow, QModelIndex destparent, bool doupdate = true);
};

#endif // FRAME_HPP
