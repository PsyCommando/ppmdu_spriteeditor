#include "sprite_anim.hpp"
#include <src/sprite.h>

const QList<QVariant> AnimSequences::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Nb frames"),
};


const QList<QVariant> AnimSequence::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Duration"),
    QString("X"),
    QString("Y"),
    QString("Shadow X"),
    QString("Shadow Y"),
    QString("Flag"),
};

AnimSequences::AnimSequences(TreeElement *parent)
    :BaseTreeContainerChild(parent), m_pmodel(new model_t(this))
{
    setNodeDataTy(eTreeElemDataType::animSequences);
}

AnimSequences::AnimSequences(const AnimSequences &cp)
    :BaseTreeContainerChild(cp), m_pmodel(new model_t(this))
{

}

AnimSequences::AnimSequences(AnimSequences &&mv)
    :BaseTreeContainerChild(mv), m_pmodel(new model_t(this))
{

}

AnimSequences &AnimSequences::operator=(const AnimSequences &cp)
{
    BaseTreeContainerChild::operator=(cp);
    return *this;
}

AnimSequences &AnimSequences::operator=(AnimSequences && mv)
{
    BaseTreeContainerChild::operator=(mv);
    return *this;
}

QVariant AnimSequences::nodeData(int column, int role) const
{
    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
        return QVariant(ElemName());
    return QVariant();
}

QVariant AnimSequences::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
        return qMove(QVariant( QString("%1").arg(section) ));
    else if( orientation == Qt::Orientation::Horizontal && section < HEADER_COLUMNS.size() )
        return HEADER_COLUMNS.at(section);
    return QVariant();
}

AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id)
{
    return static_cast<AnimSequence*>(nodeChild(id));
}

void AnimSequences::removeSequence(fmt::AnimDB::animseqid_t id)
{
    removeChildrenNodes(id,1);
}

void AnimSequences::importSequences(const fmt::AnimDB::animseqtbl_t &src)
{
    removeChildrenNodes(0, nodeChildCount());
    insertChildrenNodes(0, src.size());

    for( fmt::AnimDB::animseqid_t cntid = 0; cntid < src.size(); ++cntid )
        m_container[cntid].importSeq(src.at(cntid));
}

fmt::AnimDB::animseqtbl_t AnimSequences::exportSequences()
{
    fmt::AnimDB::animseqtbl_t seqs;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seqs[cntid] = std::move(m_container[cntid].exportSeq());

    return qMove(seqs);
}

AnimSequences::model_t *AnimSequences::getModel() {return m_pmodel.data();}

Sprite *AnimFrame::parentSprite()
{
    return static_cast<AnimSequence*>(parentNode())->parentSprite();
}

QVariant AnimFrame::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
        return QVariant();

    if( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        switch(column)
        {
        case 0: //preview
            return QVariant(QString("%1#%2").arg(ElemName()).arg(nodeIndex()));
        case 1: //duration
            return QVariant(static_cast<int>(duration()));
        case 2: //x
            return QVariant(static_cast<int>(xoffset()));
        case 3: //y
            return QVariant(static_cast<int>(yoffset()));
        case 4: //shadow x
            return QVariant(static_cast<int>(shadowx()));
        case 5: //shadow y
            return QVariant(static_cast<int>(shadowy()));
        case 6: //flag
            return QVariant(static_cast<int>(flags()));
        };
    }
    else if( role == Qt::DecorationRole && column == 0 )
    {
        MFrame * pframe = const_cast<AnimFrame*>(this)->parentSprite()->getFrame(frmidx());
        if(pframe)
            return QVariant(QPixmap::fromImage(pframe->AssembleFrame(0,0, QRect())));
    }

    return QVariant();
}




Sprite *AnimTable::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

Sprite *AnimGroup::parentSprite()
{
    return static_cast<AnimTable*>(parentNode())->parentSprite();
}

Sprite *AnimSequences::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

Sprite *AnimSequence::parentSprite()
{
    return static_cast<AnimSequences*>(parentNode())->parentSprite();
}
