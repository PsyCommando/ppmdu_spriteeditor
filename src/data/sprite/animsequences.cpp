#include "animsequences.hpp"
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/data/sprite/sprite.hpp>

const char * ElemName_AnimSequences = "Anim Sequences";

//********************************************************************************************
//  AnimSequences
//********************************************************************************************
const QList<QVariant> AnimSequences::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Nb frames"),
};

AnimSequences::AnimSequences(TreeElement *parent)
    :BaseTreeContainerChild(parent),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{
    setNodeDataTy(eTreeElemDataType::animSequences);
}

AnimSequences::AnimSequences(const AnimSequences &cp)
    :BaseTreeContainerChild(cp),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{

}

AnimSequences::AnimSequences(AnimSequences &&mv)
    :BaseTreeContainerChild(mv),
      m_pmodel(new model_t(this)),
      m_pickermodel(new AnimSequencesPickerModel(this))
{

}

AnimSequences::~AnimSequences()
{
    qDebug("AnimSequences::~AnimSequences()\n");
}

void AnimSequences::clone(const TreeElement *other)
{
    const AnimSequences * ptr = static_cast<const AnimSequences*>(other);
    if(!ptr)
        throw std::runtime_error("AnimSequences::clone(): other is not a AnimSequences!");
    (*this) = *ptr;
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

QVariant AnimSequences::data(const QModelIndex &index, int role) const
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

int AnimSequences::columnCount(const QModelIndex &parent)
{
    if (parent.isValid())
        return static_cast<TreeElement*>(parent.internalPointer())->nodeColumnCount();
    else
        return HEADER_COLUMNS.size();
}

AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id)
{
    return static_cast<AnimSequence*>(nodeChild(id));
}

const AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id) const
{
    return const_cast<AnimSequences*>(this)->getSequenceByID(id);
}

AnimSequencesPickerModel *AnimSequences::getPickerModel()
{
    return m_pickermodel.data();
}

void AnimSequences::removeSequence(fmt::AnimDB::animseqid_t id)
{
    getModel()->removeRow(id);
}

void AnimSequences::importSequences(const fmt::AnimDB::animseqtbl_t &src)
{
    getModel()->removeRows(0, nodeChildCount());
    getModel()->insertRows(0, src.size());

    for( fmt::AnimDB::animseqid_t cntid = 0; static_cast<size_t>(cntid) < src.size(); ++cntid )
        m_container[cntid].importSeq(src.at(cntid));
}

fmt::AnimDB::animseqtbl_t AnimSequences::exportSequences()
{
    fmt::AnimDB::animseqtbl_t seqs;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seqs[cntid] = std::move(m_container[cntid].exportSeq());

    return seqs;
}

AnimSequences::model_t *AnimSequences::getModel() {return m_pmodel.data();}

Sprite *AnimSequences::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

//*******************************************************************
//  AnimSequencesPickerModel
//*******************************************************************
//Model meant to display all animation sequences so they can be dragged to an animation slot in the animation table.
const QStringList AnimSequencesPickerModel::ColumnNames
{
    "Thubmnail",
    "Nb Frames",
};

AnimSequencesPickerModel::AnimSequencesPickerModel(AnimSequences *pseqs, QObject *parent)
    :QAbstractItemModel(parent), m_pOwner(pseqs)
{}

QModelIndex AnimSequencesPickerModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid() && row < m_pOwner->nodeChildCount())
        return createIndex(row, column, m_pOwner->nodeChild(row));
    return QModelIndex();
}

QModelIndex AnimSequencesPickerModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int AnimSequencesPickerModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_pOwner->nodeChildCount();
    else
        return 0;
}

int AnimSequencesPickerModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return m_pOwner->nodeColumnCount();
    else
        return 0;
}

QVariant AnimSequencesPickerModel::data(const QModelIndex &index, int role) const
{
    //Custom data display for the anim sequence picker!
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole )
        return QVariant();

    switch(static_cast<eColumns>(index.column()))
    {
    case eColumns::Preview:
        {
            if(role == Qt::DecorationRole)
            {
                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QVariant(pseq->makePreview());
            }
            else if(role == Qt::DisplayRole)
            {
                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
                Q_ASSERT(pseq);
                return QString("Sequence#%1 - %2 frames").arg(index.row()).arg(pseq->getSeqLength());
            }
            break;
        }
    case eColumns::NbFrames:
//        {
//            if(role == Qt::DisplayRole)
//            {
//                AnimSequence * pseq = m_pOwner->getSequenceByID(index.row());
//                Q_ASSERT(pseq);
//                return QString("%1 frames").arg(pseq->nodeChildCount());
//            }
//            break;
//        }
    default: break;
    };
    return QVariant();
}

QVariant AnimSequencesPickerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
    {
        return QString("%1").arg(section);
    }
    return QVariant();
}

Qt::ItemFlags AnimSequencesPickerModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}
