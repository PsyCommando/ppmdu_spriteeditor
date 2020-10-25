#ifndef ANIMTABLE_HPP
#define ANIMTABLE_HPP
#include <src/data/treenodewithchilds.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/animgroup.hpp>

class Sprite;
extern const QString ElemName_AnimTable;

//*******************************************************************
//  AnimTable
//*******************************************************************
class AnimTable : public TreeNodeWithChilds<AnimGroup>
{
public:
    typedef QPair<fmt::AnimDB::animgrpid_t, QString> animtblentry_t; //Second is the animation name assigned to the slot, first is the animation group assigned!

    AnimTable(TreeNode *parentsprite)
        :TreeNodeWithChilds(parentsprite)
    {
    }

    AnimTable(const AnimTable &cp)
        :TreeNodeWithChilds(cp)
    {
        operator=(cp);
    }

    AnimTable(AnimTable &&mv)
        :TreeNodeWithChilds(qMove(mv))
    {
        operator=(qMove(mv));
    }

    AnimTable &operator=(const AnimTable &cp)
    {
        TreeNodeWithChilds::operator=(cp);
        m_slotNames = cp.m_slotNames;
        return *this;
    }

    AnimTable &operator=(AnimTable &&mv)
    {
        TreeNodeWithChilds::operator=(mv);
        m_slotNames = qMove(mv.m_slotNames);
        return *this;
    }

    ~AnimTable();

    //Load the animation table
    void                    importAnimationTable( const fmt::AnimDB::animtbl_t & orig );
    fmt::AnimDB::animtbl_t  exportAnimationTable();

    void                        importAnimationGroups( fmt::AnimDB::animgrptbl_t & animgrps );
    fmt::AnimDB::animgrptbl_t   exportAnimationGroups();

    //Clears any references to a group from the animation table!
    void DeleteGroupRefs( fmt::AnimDB::animgrpid_t id );
    void DeleteGroupChild( fmt::AnimDB::animgrpid_t id );

    inline int getSlotsTableSize()const {return m_slotNames.size();}

    int getNbGroupSequenceSlots()const;

    //Returns the associated animation name for a slot, or an empty string if no anim is associated
    QString getSlotName (fmt::AnimDB::animgrpid_t entry)const;
    void setSlotName (fmt::AnimDB::animgrpid_t entry, const QString & name);

//    AnimTableDelegate       * getDelegate();
//    const AnimTableDelegate *getDelegate()const;

    // TreeNode interface
public:
    TreeNode *          clone() const override;
    eTreeElemDataType   nodeDataTy() const override;
    const QString &     nodeDataTypeName() const override;
    QString             nodeDisplayName() const override;
    bool                nodeIsMutable()const override    {return true;}

private:
    QHash<fmt::AnimDB::animgrpid_t, QString> m_slotNames; //List of all the named slots indices and their name
    //QList<animtblentry_t> m_animtbl;      //This list is meant to tell which anim group index correspond to what animation entry index
    //The actual AnimGroup objects are stored in the base class
    //QScopedPointer<AnimTableDelegate>   m_delegate;
};

#endif // ANIMTABLE_HPP
