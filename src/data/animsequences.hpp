#ifndef ANIMSEQUENCES_HPP
#define ANIMSEQUENCES_HPP
#include <src/treeelem.hpp>
#include <src/data/animsequence.hpp>

extern const char * ElemName_AnimSequences;
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
    const AnimSequence * getSequenceByID( fmt::AnimDB::animseqid_t id )const;

    AnimSequencesPickerModel * getPickerModel();

    bool nodeIsMutable()const override    {return false;}

private:
    QScopedPointer<model_t> m_pmodel;
    QScopedPointer<AnimSequencesPickerModel> m_pickermodel;
};

#endif // ANIMSEQUENCES_HPP
