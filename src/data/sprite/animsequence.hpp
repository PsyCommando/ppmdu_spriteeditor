#ifndef ANIMSEQUENCE_HPP
#define ANIMSEQUENCE_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/animframe.hpp>

class Sprite;
class MFrame;
extern const QString ElemName_AnimSequence;

//*******************************************************************
//  AnimSequence
//*******************************************************************
//Access the contents of an animation sequence
class AnimSequence : public TreeNodeWithChilds<AnimFrame>
{
public:
    AnimSequence(TreeNode *parent)
        :TreeNodeWithChilds(parent)
    {
        //m_flags |= Qt::ItemFlag::ItemIsEditable;
    }

    AnimSequence(const AnimSequence & cp)
        :TreeNodeWithChilds(cp)
    {}

    AnimSequence(AnimSequence && mv)
        :TreeNodeWithChilds(mv)
    {}

    AnimSequence &operator=(const AnimSequence &cp)
    {
        TreeNodeWithChilds::operator=(cp);
        return *this;
    }

    AnimSequence &operator=(AnimSequence &&mv)
    {
        TreeNodeWithChilds::operator=(mv);
        return *this;
    }

    bool operator==(const AnimSequence & other)const;
    bool operator!=(const AnimSequence & other)const;

    //import/export should be handled via model when displayed
    void                    importSeq(const fmt::AnimDB::animseq_t & seq);
    fmt::AnimDB::animseq_t  exportSeq()const;

    //Return an image of the first frame of the animation sequence
    QImage makePreview(const Sprite* owner)const;

    //TreeNode stuff
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;
    QString             nodeDisplayName() const override;
    bool                nodeShowChildrenOnTreeView()const override {return false;}
};

#endif // ANIMSEQUENCE_HPP
