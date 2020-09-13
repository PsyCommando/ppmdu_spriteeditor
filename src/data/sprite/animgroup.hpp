#ifndef ANIMGROUP_HPP
#define ANIMGROUP_HPP
#include <src/data/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

extern const char * ElemName_AnimGroup;
//*******************************************************************
//  AnimGroupModel
//*******************************************************************
class AnimGroup;
class AnimGroupModel : public QAbstractItemModel
{
    AnimGroup * m_pOwner;
public:
    AnimGroupModel(AnimGroup * pgrp, QObject * parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &/*child*/) const override;
    virtual int         rowCount(const QModelIndex &parent) const override;
    virtual int         columnCount(const QModelIndex &parent) const override;
    virtual bool        hasChildren(const QModelIndex &parent) const override;
    virtual QVariant    data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::DropActions supportedDragActions() const override;

    // QAbstractItemModel interface
public:
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override
    {
        return QAbstractItemModel::dropMimeData(data,action,row,column,parent);
    }
};

//*******************************************************************
//  AnimGroupDelegate
//*******************************************************************
class AnimGroupDelegate;

//*******************************************************************
//  AnimGroup
//*******************************************************************
class AnimGroup : public BaseTreeTerminalChild<&ElemName_AnimGroup>
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

    AnimGroup(TreeElement * parent);
    AnimGroup(AnimGroup && mv);
    AnimGroup(const AnimGroup & cp);
    AnimGroup & operator=(AnimGroup && mv);
    AnimGroup & operator=(const AnimGroup & cp);
    ~AnimGroup();

    void clone(const TreeElement *other);


    int             nodeColumnCount()const override {return ColumnNames.size();}
    QVariant        nodeData(int column, int role) const override;


    void                    importGroup(const fmt::AnimDB::animgrp_t & grp);
    fmt::AnimDB::animgrp_t  exportGroup();

    inline bool operator==( const AnimGroup & other)const  {return this == &other;}
    inline bool operator!=( const AnimGroup & other)const  {return !operator==(other);}


    void removeSequenceReferences( fmt::AnimDB::animseqid_t id );

    Sprite * parentSprite();

    inline uint16_t getUnk16()const         {return m_unk16;}
    inline void     setUnk16(uint16_t val)  {m_unk16 = val;}

    int getGroupUID()const {return nodeIndex();}

    inline const slots_t & seqSlots()const {return m_seqlist;}
    inline slots_t       & seqSlots()      {return m_seqlist;}

    void InsertRow(int row, fmt::AnimDB::animseqid_t val)
    {
        m_seqlist.insert(row, val);
    }

    void RemoveRow(int row)
    {
        m_seqlist.removeAt(row);
    }

     inline AnimGroupModel * getModel() {return &m_model;}
     inline const AnimGroupModel * getModel()const {return &m_model;}

private:
    slots_t                             m_seqlist;
    uint16_t                            m_unk16{0};
    AnimGroupModel                      m_model;
    QScopedPointer<AnimGroupDelegate>   m_delegate;
};


#endif // ANIMGROUP_HPP
