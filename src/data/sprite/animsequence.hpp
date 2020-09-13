#ifndef ANIMSEQUENCE_HPP
#define ANIMSEQUENCE_HPP
#include <QTableWidget>
#include <QList>
#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#include <src/data/treeelem.hpp>
//#include <src/ppmdu/fmts/wa_sprite.hpp>
//#include <src/ppmdu/utils/imgutils.hpp>
#include <src/data/sprite/animframe.hpp>

class Sprite;
class MFrame;
extern const char * ElemName_AnimSequence;

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

    Sprite * parentSprite()override;

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
    AnimSequenceDelegate m_delegate;
};


#endif // ANIMSEQUENCE_HPP
