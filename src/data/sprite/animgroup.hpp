#ifndef ANIMGROUP_HPP
#define ANIMGROUP_HPP
#include <src/data/treenodeterminal.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

extern const QString ElemName_AnimGroup;

//*******************************************************************
//  AnimGroup
//*******************************************************************
class AnimGroup : public TreeNodeTerminal
{
public:
    enum struct eColumns : int
    {
        GroupID = 0,
        GroupName,
        NbSlots,
        NbColumns,
    };

    static const QStringList ColumnNames;
    typedef QList<fmt::AnimDB::animseqid_t> slots_t;

    AnimGroup(TreeNode * parent);
    AnimGroup(AnimGroup && mv);
    AnimGroup(const AnimGroup & cp);
    AnimGroup & operator=(AnimGroup && mv);
    AnimGroup & operator=(const AnimGroup & cp);
    ~AnimGroup();

    void                    importGroup(const fmt::AnimDB::animgrp_t & grp);
    fmt::AnimDB::animgrp_t  exportGroup();

    bool operator==( const AnimGroup & other)const;
    bool operator!=( const AnimGroup & other)const;


    void removeSequenceReferences( fmt::AnimDB::animseqid_t id );

    uint16_t getUnk16()const;
    void     setUnk16(uint16_t val);

    int getGroupUID()const;

    const slots_t & seqSlots()const;
    slots_t       & seqSlots();

    void InsertRow(int row, fmt::AnimDB::animseqid_t val);
    void RemoveRow(int row);

// TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;

private:
    slots_t     m_seqlist;
    uint16_t    m_unk16 {0};
};


#endif // ANIMGROUP_HPP
