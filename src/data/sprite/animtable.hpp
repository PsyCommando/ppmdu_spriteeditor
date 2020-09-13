#ifndef ANIMTABLE_HPP
#define ANIMTABLE_HPP
#include <src/data/treeelem.hpp>
#include <src/data/sprite/animsequences.hpp>

class Sprite;
class AnimGroup;
extern const char * ElemName_AnimTable;

//*******************************************************************
//  AnimTableDelegate
//*******************************************************************
class AnimTableDelegate;

//*******************************************************************
//  AnimTable
//*******************************************************************
class AnimTable : public BaseTreeContainerChild<&ElemName_AnimTable, AnimGroup>
{
public:
    typedef QPair<fmt::AnimDB::animgrpid_t, QString> animtblentry_t; //Second is the animation name assigned to the slot, first is the animation group assigned!

    AnimTable(TreeElement * parent);
    AnimTable(const AnimTable & cp);
    AnimTable(AnimTable && mv);
    AnimTable & operator=(const AnimTable & cp);
    AnimTable & operator=(AnimTable && mv);
    void clone(const TreeElement *other);

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

    //Returns the associated animation name for a slot, or an empty string if no anim is associated
    QString getSlotName (fmt::AnimDB::animgrpid_t entry)const;
    void setSlotName (fmt::AnimDB::animgrpid_t entry, const QString & name);

    Sprite * parentSprite();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant nodeData(int column, int role) const override;
    bool     nodeIsMutable()const override    {return true;}
    virtual int nodeColumnCount() const       {return 1;}

    AnimTableDelegate       * getDelegate();
    const AnimTableDelegate *getDelegate()const;

private:
    QHash<fmt::AnimDB::animgrpid_t, QString> m_slotNames; //List of all the named slots indices and their name
    //QList<animtblentry_t> m_animtbl;      //This list is meant to tell which anim group index correspond to what animation entry index
    //The actual AnimGroup objects are stored in the base class
    QScopedPointer<AnimTableDelegate>   m_delegate;
};



#endif // ANIMTABLE_HPP
