#include "animsequences.hpp"
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/data/sprite/sprite.hpp>

const QString ElemName_AnimSequences = "Anim Sequences";

//********************************************************************************************
//  AnimSequences
//********************************************************************************************
AnimSequences::~AnimSequences()
{
    //qDebug("AnimSequences::~AnimSequences()\n");
}

TreeNode *AnimSequences::clone() const
{
    return new AnimSequences(*this);
}

eTreeElemDataType AnimSequences::nodeDataTy() const
{
    return eTreeElemDataType::animSequences;
}

const QString &AnimSequences::nodeDataTypeName() const
{
    return ElemName_AnimSequences;
}

//QVariant AnimSequences::data(const QModelIndex &index, int role) const
//{
//    if (!index.isValid())
//        return QVariant("root");

//    if (role != Qt::DisplayRole &&
//        role != Qt::DecorationRole &&
//        role != Qt::SizeHintRole &&
//        role != Qt::EditRole)
//        return QVariant();

//    return static_cast<TreeNode*>(index.internalPointer())->nodeData(index.column(), role);
//}

//QVariant AnimSequences::nodeData(int column, int role) const
//{
//    if(column == 0 && (role == Qt::DisplayRole || role == Qt::EditRole))
//        return QVariant(ElemName());
//    return QVariant();
//}

//QVariant AnimSequences::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if( role != Qt::DisplayRole )
//        return QVariant();

//    if( orientation == Qt::Orientation::Vertical )
//        return qMove(QVariant( QString("%1").arg(section) ));
//    else if( orientation == Qt::Orientation::Horizontal && section < HEADER_COLUMNS.size() )
//        return HEADER_COLUMNS.at(section);
//    return QVariant();
//}

//int AnimSequences::columnCount(const QModelIndex &parent)
//{
//    if (parent.isValid())
//        return static_cast<TreeNode*>(parent.internalPointer())->nodeColumnCount();
//    else
//        return HEADER_COLUMNS.size();
//}

AnimSequence *AnimSequences::getSequenceByID(fmt::animseqid_t id)
{
    return static_cast<AnimSequence*>(nodeChild(id));
}

const AnimSequence *AnimSequences::getSequenceByID(fmt::animseqid_t id) const
{
    return const_cast<AnimSequences*>(this)->getSequenceByID(id);
}

QString AnimSequences::nodeDisplayName() const
{
    return nodeDataTypeName();
}

//AnimSequencesPickerModel *AnimSequences::getPickerModel()
//{
//    return m_pickermodel.data();
//}

//void AnimSequences::removeSequence(fmt::AnimDB::animseqid_t id)
//{
//    getModel()->removeRow(id);
//}

void AnimSequences::importSequences(const fmt::AnimDB::animseqtbl_t &src)
{
//    getModel()->removeRows(0, nodeChildCount());
//    getModel()->insertRows(0, src.size());

    _removeChildrenNodes(0, nodeChildCount());
    _insertChildrenNodes(0, src.size());

    for( fmt::animseqid_t cntid = 0; static_cast<size_t>(cntid) < src.size(); ++cntid )
        m_container[cntid]->importSeq(src.at(cntid));
}

fmt::AnimDB::animseqtbl_t AnimSequences::exportSequences()
{
    fmt::AnimDB::animseqtbl_t seqs;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seqs[cntid] = m_container[cntid]->exportSeq();

    return seqs;
}

AnimSequence *AnimSequences::appendNewSequence()
{
    int newidx = nodeChildCount();
    _insertChildrenNodes(newidx, 1);
    return m_container[newidx];
}

//AnimSequences::model_t *AnimSequences::getModel() {return m_pmodel.data();}

//Sprite *AnimSequences::parentSprite()
//{
//    return static_cast<Sprite*>(parentNode());
//}





