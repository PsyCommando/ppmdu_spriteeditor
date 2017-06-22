#ifndef SPRITE_ANIMDB_HPP
#define SPRITE_ANIMDB_HPP
#include <QTableWidget>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>


class Sprite;
class MFrame;
extern const char * ElemName_AnimSequence ;
extern const char * ElemName_AnimSequences;
extern const char * ElemName_AnimTable    ;
extern const char * ElemName_AnimGroup    ;
extern const char * ElemName_AnimFrame    ;

//*******************************************************************
//  AnimFrame
//*******************************************************************
class AnimFrame :  public BaseTreeTerminalChild<&ElemName_AnimFrame>
{
    static QSize calcTextSize(const QString &str);
public:
    static const char *         UProp_AnimFrameID;  //UserProp name used for storing the id of this frame!
    static const QStringList    ColumnNames;        //Name displayed in the column header for each properties of the frame! Is tied to eColumnsType

    //NOTE: Be sure to update ColumnNames when changing this!
    enum struct eColumnsType : int
    {
        Frame = 0,
        Duration,
        Offset,
        ShadowOffset,
        Flags,
        NBColumns,
        //Everything below this is not displayed as header column

        //To acces some of the merged data individually via model data! Since we merged both x/y param entry into a single one for each categories
        Direct_XOffset,
        Direct_YOffset,
        Direct_ShadowXOffset,
        Direct_ShadowYOffset,
    };


public:
    AnimFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags())
    {
        setNodeDataTy(eTreeElemDataType::animFrame);
    }

    void clone(const TreeElement *other)
    {
        const AnimFrame * ptr = static_cast<const AnimFrame*>(other);
        if(!ptr)
            throw std::runtime_error("AnimFrame::clone(): other is not a AnimFrame!");
        (*this) = *ptr;
    }

    inline bool operator==( const AnimFrame & other)const  {return this == &other;}
    inline bool operator!=( const AnimFrame & other)const  {return !operator==(other);}

    void importFrame( const fmt::animfrm_t & frm )
    {
        m_data = frm;
    }

    fmt::animfrm_t exportFrame()const
    {
        return m_data;
    }

    inline uint8_t duration()const {return m_data.duration;}
    inline int16_t frmidx  ()const {return m_data.frmidx;}
    inline uint8_t flags   ()const {return m_data.flag;}
    inline int16_t xoffset ()const {return m_data.xoffs;}
    inline int16_t yoffset ()const {return m_data.yoffs;}
    inline int16_t shadowx ()const {return m_data.shadowxoffs;}
    inline int16_t shadowy ()const {return m_data.shadowyoffs;}

    inline void setDuration(uint8_t val) {m_data.duration = val;}
    inline void setFrmidx  (int16_t val) {m_data.frmidx = val;}
    inline void setFlags   (uint8_t val) {m_data.flag = val;}
    inline void setXoffset (int16_t val) {m_data.xoffs = val;}
    inline void setYoffset (int16_t val) {m_data.yoffs = val;}
    inline void setShadowx (int16_t val) {m_data.shadowxoffs = val;}
    inline void setShadowy (int16_t val) {m_data.shadowyoffs = val;}

    Sprite * parentSprite() override;
    const Sprite * parentSprite() const override
    {
        return const_cast<AnimFrame*>(this)->parentSprite();
    }

    virtual QVariant nodeData(int column, int role) const override;
    bool nodeIsMutable()const override    {return true;}

    QImage makePreview()const;

private:
    fmt::animfrm_t          m_data;
    QPixmap                 m_cached;
};

//*******************************************************************
//  AnimSequenceDelegate
//*******************************************************************
class AnimSequence;
class AnimSequenceDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AnimSequenceDelegate(AnimSequence * seq, QObject *parent = nullptr)
        :QStyledItemDelegate(parent), m_pOwner(seq)
    {}


    AnimSequenceDelegate(AnimSequenceDelegate && mv)
        :QStyledItemDelegate(mv.parent())
    {
        operator=(qMove(mv));
    }

    AnimSequenceDelegate & operator=(AnimSequenceDelegate && mv)
    {
        m_pOwner = mv.m_pOwner;
        return *this;
    }

    //No copies plz
    AnimSequenceDelegate(const AnimSequenceDelegate & cp) = delete;
    AnimSequenceDelegate & operator=(const AnimSequenceDelegate & cp) = delete;
    // QAbstractItemDelegate interface
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    //Returns the stylesheet used for comboboxes this delegate makes
    static const QString & ComboBoxStyleSheet();
    static const QString XOffsSpinBoxName;
    static const QString YOffsSpinBoxName;

    QWidget * makeFrameSelect       (QWidget *parent)const;
    QWidget * makeOffsetWidget      (QWidget *parent)const;
    QWidget * makeShadowOffsetWidget(QWidget *parent)const;

    //Variable:
private:
    AnimSequence * m_pOwner;
};


//*******************************************************************
//  AnimSequence
//*******************************************************************
class AnimSequence : public BaseTreeContainerChild<&ElemName_AnimSequence, AnimFrame>
{
public:
    typedef container_t::iterator                       iterator;
    typedef container_t::const_iterator                 const_iterator;

    AnimSequence( TreeElement * parent )
        :BaseTreeContainerChild(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags()), m_delegate(this)
    {
        setNodeDataTy(eTreeElemDataType::animSequence);
    }

    AnimSequence( const AnimSequence & cp )
        :BaseTreeContainerChild(cp), m_delegate(this)
    {}

    AnimSequence( AnimSequence && mv )
        :BaseTreeContainerChild(mv), m_delegate(this)
    {}

    AnimSequence & operator=( const AnimSequence & cp )
    {
        BaseTreeContainerChild::operator=(cp);
        return *this;
    }

    AnimSequence & operator=( AnimSequence && mv )
    {
        BaseTreeContainerChild::operator=(mv);
        return *this;
    }

    void clone(const TreeElement *other)
    {
        const AnimSequence * ptr = static_cast<const AnimSequence*>(other);
        if(!ptr)
            throw std::runtime_error("AnimFrame::clone(): other is not a AnimFrame!");
        (*this) = *ptr;
    }

    inline bool operator==( const AnimSequence & other)const  {return this == &other;}
    inline bool operator!=( const AnimSequence & other)const  {return !operator==(other);}

    inline iterator         begin()     {return m_container.begin();}
    inline const_iterator   begin()const{return m_container.begin();}
    inline iterator         end()       {return m_container.end();}
    inline const_iterator   end()const  {return m_container.end();}
    inline size_t           size()const {return m_container.size();}
    inline bool             empty()const{return m_container.empty();}

    void importSeq(const fmt::AnimDB::animseq_t & seq);

    fmt::AnimDB::animseq_t exportSeq()const;

    inline int getSeqLength()const {return nodeChildCount();}

    Sprite              * parentSprite()override;

    inline int nodeColumnCount() const override
    {
        return AnimFrame::ColumnNames.size();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    QVariant data(const QModelIndex &index, int role) const override;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    inline AnimSequenceDelegate         * getDelegate()     {return &m_delegate;}
    inline const AnimSequenceDelegate   * getDelegate()const{return &m_delegate;}

    QImage makePreview()const;

private:
    AnimSequenceDelegate    m_delegate;
};

//*******************************************************************
//  AnimSequencesPickerModel
//*******************************************************************
//Model meant to display all animation sequences so they can be dragged to an animation slot in the animation table.
class AnimSequences;
class AnimSequencesPickerModel : public QAbstractItemModel
{
    AnimSequences *  m_pOwner;
public:

    enum struct eColumns : int
    {
        Preview = 0,
        NbFrames,
        NbColumns,
    };

    static const QStringList ColumnNames;

public:
    AnimSequencesPickerModel(AnimSequences * pseqs, QObject * parent = nullptr);



    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    virtual QModelIndex parent(const QModelIndex &child) const override;

    virtual int rowCount(const QModelIndex &parent) const override;

    virtual int columnCount(const QModelIndex &parent) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
};

//*******************************************************************
//  AnimSequences
//*******************************************************************
class AnimSequences : public BaseTreeContainerChild<&ElemName_AnimSequences, AnimSequence>
{
public:
    typedef BaseTreeNodeModel model_t;

    static const QList<QVariant> HEADER_COLUMNS;

    AnimSequences( TreeElement * parentNode );
    AnimSequences( const AnimSequences & cp );
    AnimSequences( AnimSequences && mv );

    ~AnimSequences();

    void clone(const TreeElement *other);

    AnimSequences & operator=( const AnimSequences & cp );
    AnimSequences & operator=( AnimSequences && mv );

    //QTreeModel
    virtual QVariant data(const QModelIndex &index, int role)const override;
    QVariant nodeData(int column, int role)const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual int columnCount(const QModelIndex &parent) override;

    //
    void                        removeSequence( fmt::AnimDB::animseqid_t id );
    void                        importSequences( const fmt::AnimDB::animseqtbl_t & src );
    fmt::AnimDB::animseqtbl_t   exportSequences();

    //Accessors
    Sprite * parentSprite();
    model_t * getModel();
    AnimSequence * getSequenceByID( fmt::AnimDB::animseqid_t id );

    AnimSequencesPickerModel * getPickerModel();

    bool nodeIsMutable()const override    {return false;}

private:
    QScopedPointer<model_t> m_pmodel;
    QScopedPointer<AnimSequencesPickerModel> m_pickermodel;
};

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

     inline AnimGroupModel * getModel() {return &m_model;}
     inline const AnimGroupModel * getModel()const {return &m_model;}

private:
    slots_t                             m_seqlist;
    uint16_t                            m_unk16;
    AnimGroupModel                      m_model;
    QScopedPointer<AnimGroupDelegate>   m_delegate;
};

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
    inline const QString getSlotName (fmt::AnimDB::animgrpid_t entry)const
    {
        if(entry >= 0 && entry < m_slotNames.size() )
        {
            auto itf = m_slotNames.find(entry);
            if(itf != m_slotNames.end())
                return (*itf);
        }
        return QString();
    }
    inline void setSlotName (fmt::AnimDB::animgrpid_t entry, const QString & name)
    {
        if(entry >= 0 && entry < m_slotNames.size())
            m_slotNames[entry] = name;
    }

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


#endif // SPRITE_ANIMDB_HPP
