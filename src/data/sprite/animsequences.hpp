#ifndef ANIMSEQUENCES_HPP
#define ANIMSEQUENCES_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/animsequence.hpp>

extern const QString ElemName_AnimSequences;


//*******************************************************************
//  AnimSequences
//*******************************************************************
//This tree node is used as a category for holding all the animation sequences in the main treeview!
class AnimSequences : public TreeNodeWithChilds<AnimSequence>
{
public:
//    AnimSequences(TreeNode * parentNode);
//    AnimSequences(const AnimSequences & cp);
//    AnimSequences(AnimSequences && mv);
//    AnimSequences & operator=( const AnimSequences & cp );
//    AnimSequences & operator=( AnimSequences && mv );
    AnimSequences(TreeNode *parent)
        :TreeNodeWithChilds(parent)
    {
    }

    AnimSequences(const AnimSequences &cp)
        :TreeNodeWithChilds(cp)
    {
    }

    AnimSequences(AnimSequences &&mv)
        :TreeNodeWithChilds(mv)
    {
    }

    ~AnimSequences();

    AnimSequences &operator=(const AnimSequences &cp)
    {
        TreeNodeWithChilds::operator=(cp);
        return *this;
    }

    AnimSequences &operator=(AnimSequences && mv)
    {
        TreeNodeWithChilds::operator=(mv);
        return *this;
    }

public:
    void                        importSequences( const fmt::AnimDB::animseqtbl_t & src );
    fmt::AnimDB::animseqtbl_t   exportSequences();

    //Accessors
    AnimSequence        * getSequenceByID( fmt::AnimDB::animseqid_t id );
    const AnimSequence  * getSequenceByID( fmt::AnimDB::animseqid_t id )const;

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

    //QTreeModel
//    virtual QVariant data(const QModelIndex &index, int role)const override;
//    QVariant nodeData(int column, int role)const override;
//    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//    virtual int columnCount(const QModelIndex &parent) override;

    //
//    void                        removeSequence( fmt::AnimDB::animseqid_t id );


    bool nodeIsMutable()const override    {return false;}
    QString nodeDisplayName()const override;

};

#endif // ANIMSEQUENCES_HPP
