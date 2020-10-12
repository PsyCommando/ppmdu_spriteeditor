#include "animsequence.hpp"
#include <src/data/sprite/animframe.hpp>
#include <src/data/sprite/animsequences.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/models/animframe_model.hpp>
#include <QComboBox>
#include <QSpinBox>
#include <QVBoxLayout>

const QString ElemName_AnimSequence  = "Anim Sequence";
//********************************************************************************************
//  AnimSequence
//********************************************************************************************
//const QList<QVariant> AnimSequence::HEADER_COLUMNS
//{
//    QString("Frame"),
//    QString("Duration"),
//    QString("X"),
//    QString("Y"),
//    QString("Shadow X"),
//    QString("Shadow Y"),
//    QString("Flag"),
//};

void AnimSequence::importSeq(const fmt::AnimDB::animseq_t &seq)
{
    TreeNodeWithChilds<AnimFrame>::_removeChildrenNodes(0, nodeChildCount());
    TreeNodeWithChilds<AnimFrame>::_insertChildrenNodes(0, seq.size());
//    getModel()->removeRows(0, nodeChildCount());
//    getModel()->insertRows(0, seq.size());

    auto itseq = seq.begin();
    for( fmt::frmid_t cntid = 0; cntid < static_cast<fmt::frmid_t>(seq.size()); ++cntid, ++itseq )
        m_container[cntid]->importFrame(*itseq);
}

fmt::AnimDB::animseq_t AnimSequence::exportSeq() const
{
    fmt::AnimDB::animseq_t seq;
    for( int cntid = 0; cntid < nodeChildCount(); ++cntid )
        seq.push_back(m_container[cntid]->exportFrame());
    return seq;
}

QImage AnimSequence::makePreview(const Sprite* owner) const
{
    const_iterator itbeg = begin();
    if( itbeg != end() )
        return (*itbeg)->makePreview(owner);
    return QImage();
}



eTreeElemDataType AnimSequence::nodeDataTy() const
{
    return eTreeElemDataType::animSequence;
}

const QString &AnimSequence::nodeDataTypeName() const
{
    return ElemName_AnimSequence;
}

QString AnimSequence::nodeDisplayName() const
{
    return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());
}

TreeNode *AnimSequence::clone() const
{
    return new AnimSequence(*this);
}

bool AnimSequence::operator==(const AnimSequence &other) const
{
    return this == &other;
}

bool AnimSequence::operator!=(const AnimSequence &other) const
{
    return !operator==(other);
}

AnimSequence::iterator AnimSequence::begin()
{
    return m_container.begin();
}

AnimSequence::const_iterator AnimSequence::begin() const
{
    return m_container.begin();
}

AnimSequence::iterator AnimSequence::end()
{
    return m_container.end();
}

AnimSequence::const_iterator AnimSequence::end() const
{
    return m_container.end();
}

size_t AnimSequence::size() const
{
    return m_container.size();
}

bool AnimSequence::empty() const
{
    return m_container.empty();
}
