#ifndef SPRITE_ANIMDB_HPP
#define SPRITE_ANIMDB_HPP
#include <QTableWidget>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>

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
public:
    AnimFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
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

    Sprite * parentSprite();

    virtual QVariant nodeData(int column, int role) const override;

private:
    fmt::animfrm_t          m_data;
    QPixmap                 m_cached;
};

//*******************************************************************
//  AnimSequence
//*******************************************************************
class AnimSequence : public BaseTreeContainerChild<&ElemName_AnimSequence, AnimFrame>
{
public:
    typedef container_t::iterator                       iterator;
    typedef container_t::const_iterator                 const_iterator;
    typedef BaseTreeNodeModel                           model_t;

    AnimSequence( TreeElement * parent )
        :BaseTreeContainerChild(parent), m_model(this)
    {
        setNodeDataTy(eTreeElemDataType::animSequence);
    }

    AnimSequence( const AnimSequence & cp )
        :BaseTreeContainerChild(cp), m_model(this)
    {}

    AnimSequence( AnimSequence && mv )
        :BaseTreeContainerChild(mv), m_model(this)
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

    void importSeq(const fmt::AnimDB::animseq_t & seq)
    {
        getModel().removeRows(0, nodeChildCount());
        getModel().insertRows(0, seq.size());

        auto itseq = seq.begin();
        for( fmt::frmid_t cntid = 0; cntid < static_cast<fmt::frmid_t>(seq.size()); ++cntid, ++itseq )
        {
            m_container[cntid].importFrame(*itseq);
        }
    }

    fmt::AnimDB::animseq_t exportSeq()const
    {
        fmt::AnimDB::animseq_t seq;
        for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
            seq.push_back(std::move(m_container[cntid].exportFrame()));

        return qMove(seq);
    }

    inline int getSeqLength()const {return nodeChildCount();}

    Sprite * parentSprite();

    inline model_t & getModel() {return m_model;}
    inline const model_t & getModel()const {return m_model;}

    inline int nodeColumnCount() const
    {
        return HEADER_COLUMNS.size();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if( role != Qt::DisplayRole )
            return QVariant();

        if( orientation == Qt::Orientation::Vertical )
        {
            return std::move(QVariant( QString("%1").arg(section) ));
        }
        else if( orientation == Qt::Orientation::Horizontal && section < HEADER_COLUMNS.size() )
        {
            return HEADER_COLUMNS[section];
        }
        return QVariant();
    }


    QVariant data(const QModelIndex &index, int role) const
    {        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
            return QVariant();

        return static_cast<TreeElement*>(index.internalPointer())->nodeData(index.column(), role);
    }

private:
    model_t                 m_model;
    static const QList<QVariant> HEADER_COLUMNS;
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

    ~AnimSequences()
    {
        qDebug("AnimSequences::~AnimSequences()\n");
    }

    void clone(const TreeElement *other)
    {
        const AnimSequences * ptr = static_cast<const AnimSequences*>(other);
        if(!ptr)
            throw std::runtime_error("AnimSequences::clone(): other is not a AnimSequences!");
        (*this) = *ptr;
    }

    AnimSequences & operator=( const AnimSequences & cp );
    AnimSequences & operator=( AnimSequences && mv );

    //QTreeModel
    virtual QVariant data(const QModelIndex &index, int role)const override
    {
        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
            return QVariant();

        return static_cast<TreeElement*>(index.internalPointer())->nodeData(index.column(), role);
    }
    QVariant nodeData(int column, int role)const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual int columnCount(const QModelIndex &parent) override
    {
        if (parent.isValid())
            return static_cast<TreeElement*>(parent.internalPointer())->nodeColumnCount();
        else
            return HEADER_COLUMNS.size();
    }

    //
    void                        removeSequence( fmt::AnimDB::animseqid_t id );
    void                        importSequences( const fmt::AnimDB::animseqtbl_t & src );
    fmt::AnimDB::animseqtbl_t   exportSequences();

    //Accessors
    Sprite * parentSprite();
    model_t * getModel();
    AnimSequence * getSequenceByID( fmt::AnimDB::animseqid_t id );

    bool nodeIsMutable()const override    {return false;}

private:
    QScopedPointer<model_t> m_pmodel;
};

//*******************************************************************
//  AnimGroup
//*******************************************************************
class AnimGroup : public BaseTreeTerminalChild<&ElemName_AnimGroup>
{
public:
    AnimGroup( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {setNodeDataTy(eTreeElemDataType::animGroup);}

    void clone(const TreeElement *other)
    {
        const AnimGroup * ptr = static_cast<const AnimGroup*>(other);
        if(!ptr)
            throw std::runtime_error("AnimGroup::clone(): other is not a AnimGroup!");
        (*this) = *ptr;
    }

    int nodeColumnCount()const override {return 1;}

    QVariant nodeData(int column, int role) const override
    {
        if( role != Qt::DisplayRole && role != Qt::EditRole)
            return QVariant();

        if( column == 0 )
            return QVariant( QString("%1 %2").arg(ElemName()).arg(nodeIndex()) );
        else if(column == 1)
            return QVariant(QString("%1").arg(unk16));
        else if(column == 2)
            return QVariant(QString("%1").arg(m_seqlist.size()));
        else
            return QVariant();
    }

    void importGroup(const fmt::AnimDB::animgrp_t & grp)
    {
        m_seqlist.reserve(grp.seqs.size());
        for( const auto & seq : grp.seqs )
            m_seqlist.push_back(seq);

        unk16 = grp.unk16;
    }

    fmt::AnimDB::animgrp_t exportGroup()
    {
        fmt::AnimDB::animgrp_t dest;
        dest.seqs.resize(m_seqlist.size());
        std::copy(m_seqlist.begin(), m_seqlist.end(), dest.seqs.begin());
        dest.unk16 = unk16;
        return std::move(dest);
    }

    inline bool operator==( const AnimGroup & other)const  {return this == &other;}
    inline bool operator!=( const AnimGroup & other)const  {return !operator==(other);}

    void fillTableWidget( QTableWidget * tbl, AnimSequences & seqs )
    {
        tbl->setRowCount(m_seqlist.size());

        int cntrow = 0;
        for( auto seq : m_seqlist )
        {
            tbl->setItem(cntrow, 0, new QTableWidgetItem(QString("Sequence ID ").arg(seq)) );
            AnimSequence * pseq = static_cast<AnimSequence*>(seqs.nodeChild(seq));
            if( pseq )
                tbl->setItem(cntrow, 1, new QTableWidgetItem(QString("%1").arg(pseq->getSeqLength())) );
            ++cntrow;
        }
    }


    void removeSequenceReferences( fmt::AnimDB::animseqid_t id )
    {
        for( auto & seq : m_seqlist )
        {
            if(seq == id)
                seq = -1;
        }
    }

    Sprite * parentSprite();

private:
    QList<fmt::AnimDB::animseqid_t> m_seqlist;
    uint16_t                        unk16;
};

//*******************************************************************
//  AnimTable
//*******************************************************************
class AnimTable : public BaseTreeContainerChild<&ElemName_AnimTable, AnimGroup>
{
public:

    AnimTable( TreeElement * parent )
        :BaseTreeContainerChild(parent)
    {
        setNodeDataTy(eTreeElemDataType::animTable);
    }

    void clone(const TreeElement *other)
    {
        const AnimTable * ptr = static_cast<const AnimTable*>(other);
        if(!ptr)
            throw std::runtime_error("AnimTable::clone(): other is not a AnimTable!");
        (*this) = *ptr;
    }

    QVariant nodeData(int column, int role) const override
    {
        if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
            return QVariant(ElemName());
        return QVariant();
    }

    //Load the animation table
    void importAnimationTable( const fmt::AnimDB::animtbl_t & orig )
    {
        for(auto id : orig)
            m_animtbl.push_back(id);
    }

    fmt::AnimDB::animtbl_t exportAnimationTable()
    {
        fmt::AnimDB::animtbl_t dest;
        dest.reserve(m_animtbl.size());
        for( auto id : m_animtbl )
            dest.push_back(id);
        return std::move(dest);
    }

    void importAnimationGroups( fmt::AnimDB::animgrptbl_t & animgrps )
    {
        m_container.reserve(animgrps.size());
        getModel()->removeRows(0, nodeChildCount());
        getModel()->insertRows(0, animgrps.size());

        for( fmt::AnimDB::animgrpid_t cntgrp = 0; cntgrp < static_cast<fmt::AnimDB::animgrpid_t>(animgrps.size());
             ++cntgrp )
            m_container[cntgrp].importGroup(animgrps[cntgrp]);
    }

    fmt::AnimDB::animgrptbl_t exportAnimationGroups()
    {
        fmt::AnimDB::animgrptbl_t grps;
        for( int cntgrp = 0; cntgrp < nodeChildCount(); ++cntgrp )
        {
            grps[cntgrp] = std::move(m_container[cntgrp].exportGroup());
        }
        return std::move(grps);
    }

    //Clears any references to a group from the animation table!
    void DeleteGroupRefs( fmt::AnimDB::animgrpid_t id )
    {
        int idx = -1;
        do
        {
            idx = m_animtbl.indexOf(id);
            if( idx != -1 )
                m_animtbl[idx] = -1;
        }while( idx != -1 );
    }

    void DeleteGroupChild( fmt::AnimDB::animgrpid_t id )
    {
        DeleteGroupRefs(id);
//        for( size_t cntchild = 0; cntchild < childCount(); ++cntchild )
//        {
//            AnimGroup   * pchild = static_cast<AnimGroup*>(child(cntchild));
//            if( pchild && pchild->getGrpId() == id )
                getModel()->removeRow(id);
//        }
    }

    inline int getAnimTableSize()const                              {return m_animtbl.size();}
    inline fmt::AnimDB::animgrpid_t & getAnimTableEntry(int entry)  {return m_animtbl[entry];}

    Sprite * parentSprite();

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole &&
                role != Qt::DecorationRole &&
                role != Qt::SizeHintRole &&
                role != Qt::EditRole)
            return QVariant();

        const AnimGroup *grp = static_cast<const AnimGroup*>(getItem(index));
        return grp->nodeData(index.column(), role);
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
//        section;
//        orientation;
//        role;
//        if( role != Qt::DisplayRole )
//            return QVariant();

//        if( orientation == Qt::Orientation::Vertical )
//        {
//            return std::move(QVariant( QString("%1").arg(section) ));
//        }
//        else if( orientation == Qt::Orientation::Horizontal )
//        {
//            switch(section)
//            {
//            case 0:
//                return std::move(QVariant( QString("Preview") ));
//            case 1:
//                return std::move(QVariant( QString("Bit Depth") ));
//            case 2:
//                return std::move(QVariant( QString("Resolution") ));
//            };
//        }
        Q_ASSERT(false);
        return QVariant();
    }

    bool nodeIsMutable()const override    {return false;}

private:
    QList<fmt::AnimDB::animgrpid_t> m_animtbl;
};


#endif // SPRITE_ANIMDB_HPP
