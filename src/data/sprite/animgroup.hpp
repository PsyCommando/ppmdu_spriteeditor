#ifndef ANIMGROUP_HPP
#define ANIMGROUP_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

extern const QString ElemName_AnimGroup;
extern const QString ElemName_AnimSeqRef;

//*******************************************************************
//  AnimSequenceReference
//*******************************************************************
//Helper class to make it easier to display animation sequence references in the model system
class AnimSequenceReference : public TreeNodeTerminal
{
public:
    using seqid_t = fmt::animseqid_t;
    AnimSequenceReference(TreeNode * parent);
    AnimSequenceReference(AnimSequenceReference&& mv);
    AnimSequenceReference(const AnimSequenceReference & cp);
    AnimSequenceReference & operator=(const AnimSequenceReference & cp);
    AnimSequenceReference & operator=(AnimSequenceReference && mv);
    ~AnimSequenceReference();

public:
    void    setSeqRefID(seqid_t id);
    seqid_t getSeqRefID()const;

    // TreeNode interface
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;

private:
    seqid_t m_seqid {0}; //ID reference to an animation sequence within the sprite
};

//*******************************************************************
//  AnimGroup
//*******************************************************************
//Anim group links animation slots to a set of animation sequences
class AnimGroup : public TreeNodeWithChilds<AnimSequenceReference>
{
    typedef TreeNodeWithChilds<AnimSequenceReference> parent_t ;
public:
    using animseqid_t = fmt::animseqid_t;
    enum struct eColumns : int
    {
        GroupID = 0,
        GroupName,
        NbSlots,
        NbColumns,
    };
    static const QStringList ColumnNames;

    AnimGroup(TreeNode * parent);
    AnimGroup(AnimGroup && mv);
    AnimGroup(const AnimGroup & cp);
    AnimGroup & operator=(AnimGroup && mv);
    AnimGroup & operator=(const AnimGroup & cp);
    ~AnimGroup();

    //Import raw group data
    void                    importGroup(const fmt::AnimDB::animgrp_t & grp);
    //Export to raw group data
    fmt::AnimDB::animgrp_t  exportGroup()const;

    bool operator==( const AnimGroup & other)const;
    bool operator!=( const AnimGroup & other)const;

    //Clear references to the specified sequence id. Replace them with -1
    void removeSequenceReferences(animseqid_t id);

    //Quick helper to set the subnodes to the desired value
    void setAnimSlotRef(int slot, animseqid_t id);
    animseqid_t getAnimSlotRef(int slot)const;

    uint16_t getUnk16()const;
    void     setUnk16(uint16_t val);

    //Force the selected amount of slots into this groups, removes extras, adds new empty slots
    void setNbSlots(int newnbslots);

    //Returns the unique id of this group within this sprite
    int getGroupUID()const;

    //Stitches horizontally the first frame of all the sequences in the group together into a single image.
    QPixmap MakeGroupPreview(const Sprite * owner, int maxWidth = 0, int maxHeight = 0, int maxNbImages = 0)const;

// TreeNode interface
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;
    QString             nodeDisplayName() const override;
    bool                nodeShowChildrenOnTreeView()const override {return false;}

private:
    uint16_t m_unk16 {0};

};


#endif // ANIMGROUP_HPP
