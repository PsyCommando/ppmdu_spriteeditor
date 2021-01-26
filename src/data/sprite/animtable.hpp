#ifndef ANIMTABLE_HPP
#define ANIMTABLE_HPP
#include <memory>
#include <QHash>
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/animgroup.hpp>

class Sprite;
extern const QString ElemName_AnimGroupRef;
extern const QString ElemName_AnimTable;

//*******************************************************************
// AnimTableSlot
//*******************************************************************
//Represent a slot in the animation table. Contains a reference to an animation group or not
class AnimTableSlot : public TreeNodeTerminal
{
public:
    using parent_t = TreeNodeTerminal;
    AnimTableSlot(TreeNode * parent);
    AnimTableSlot(const AnimTableSlot & cp);
    AnimTableSlot(AnimTableSlot && mv);
    AnimTableSlot & operator=(const AnimTableSlot & cp);
    AnimTableSlot & operator=(AnimTableSlot && mv);
    ~AnimTableSlot();

public:
    //Set/get the animation group this slot refers to
    void setGroupRef(const QModelIndex & ref);
    void setGroupRef(fmt::animgrpid_t ref);
    fmt::animgrpid_t getGroupRef()const;

    //Set/get the human readable name of this animation slot
    void setSlotName(const QString & name);
    const QString &getSlotName()const;

    //True if the slot refers to no animation groups, aka a "null" slot in the original file
    bool isNull()const;

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;

private:
    fmt::animgrpid_t    m_grpId {fmt::NullGrpIndex};    //ID of the group this refers to
    QString             m_slotName;                     //Name of the slot
};


//*******************************************************************
//  AnimTable
//*******************************************************************
class AnimGroups;
class AnimTable : public TreeNodeWithChilds<AnimTableSlot>
{
public:
    using parent_t = TreeNodeWithChilds<AnimTableSlot>;
    typedef int animtblidx_t;   //Indice in the animation table
    typedef QPair<fmt::animgrpid_t, QString> animtblentry_t; //Second is the animation name assigned to the slot, first is the animation group assigned!

    AnimTable(TreeNode *parentsprite);
    AnimTable(const AnimTable &cp);
    AnimTable(AnimTable &&mv);
    AnimTable &operator=(const AnimTable &cp);
    AnimTable &operator=(AnimTable &&mv);
    ~AnimTable();

    //Load the animation table
    void importAnimationTable(const fmt::AnimDB::animtbl_t & atbl, const AnimGroups & groups);

    //Transforms the data model's QModelIndex into AnimGroup indices in the AnimGroupContainer, and null references are replaced by -1.
    fmt::AnimDB::animtbl_t exportAnimationTable(const AnimGroups & groups)const;

    //Clears any references to a group from the animation table!
    void DeleteGroupRefs( fmt::animgrpid_t id );
    void DeleteGroupChild( fmt::animgrpid_t id );

    //Handling for animation slot naming in the UI
    inline int  getNbNamedSlots()const;
    QString     getSlotName (animtblidx_t entry)const;
    void        setSlotName (animtblidx_t entry, const QString & name);

    //Finds the first reference to a given group in the table
    animtblidx_t findFirstGroupRef(const AnimGroup * grp)const;

    //Stylesheet for editing groups refs
    static const QString &ComboBoxStyleSheet();

    // TreeNode interface
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;
    QString             nodeDisplayName() const override;
    bool                nodeIsMutable()const override    {return true;}
    bool                nodeShowChildrenOnTreeView()const override {return false;} //Don't show the references in the main treeview!
};

#endif // ANIMTABLE_HPP
