#include "animgroup.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequences.hpp>

const QString ElemName_AnimGroup = "Anim Group";
const QString ElemName_AnimSeqRef = "Anim Sequence Reference";

//===================================================
//  AnimSequenceReference
//===================================================
AnimSequenceReference::AnimSequenceReference(TreeNode * parent)
    :TreeNodeTerminal(parent)
{
}
AnimSequenceReference::AnimSequenceReference(AnimSequenceReference&& mv)
    :TreeNodeTerminal(mv)
{
}

AnimSequenceReference::AnimSequenceReference(const AnimSequenceReference & cp)
    :TreeNodeTerminal(cp)
{
}

AnimSequenceReference & AnimSequenceReference::operator=(const AnimSequenceReference & cp)
{
    m_seqid = cp.m_seqid;
    TreeNodeTerminal::operator=(cp);
    return *this;
}

AnimSequenceReference & AnimSequenceReference::operator=(AnimSequenceReference && mv)
{
    m_seqid = mv.m_seqid;
    TreeNodeTerminal::operator=(mv);
    return *this;
}

AnimSequenceReference::~AnimSequenceReference()
{
}

void AnimSequenceReference::setSeqRefID(AnimSequenceReference::seqid_t id)
{
    m_seqid = id;
}

AnimSequenceReference::seqid_t AnimSequenceReference::getSeqRefID() const
{
    return m_seqid;
}

TreeNode *AnimSequenceReference::clone() const
{
    return new AnimSequenceReference(*this);
}

eTreeElemDataType AnimSequenceReference::nodeDataTy() const
{
    return eTreeElemDataType::animSequenceRef;
}

const QString &AnimSequenceReference::nodeDataTypeName() const
{
    return ElemName_AnimSeqRef;
}


//**********************************************************************************
//  AnimGroup
//**********************************************************************************
const QStringList AnimGroup::ColumnNames
{
    "Group ID",
    "Group Name",
    "Nb Slots",
};

AnimGroup::AnimGroup(TreeNode *parent)
    :parent_t(parent)
{
}

AnimGroup::AnimGroup(AnimGroup &&mv)
    :parent_t(mv)
{
}

AnimGroup::AnimGroup(const AnimGroup &cp)
    :parent_t(cp)
{
}

AnimGroup & AnimGroup::operator=(AnimGroup &&mv)
{
    m_unk16 = mv.m_unk16;
    parent_t::operator=(mv);
    return *this;
}

AnimGroup & AnimGroup::operator=(const AnimGroup &cp)
{
    m_unk16 = cp.m_unk16;
    parent_t::operator=(cp);
    return *this;
}

AnimGroup::~AnimGroup()
{
}

//QVariant AnimGroup::nodeData(int column, int role) const
//{
//    if( role != Qt::DisplayRole  &&
//        role != Qt::EditRole     &&
//        role != Qt::SizeHintRole &&
//        role != Qt::DecorationRole)
//        return QVariant();

//    if(role == Qt::SizeHintRole)
//    {
//        QFontMetrics fm(QFont("Sergoe UI", 9));
//        QString str(nodeData(column,Qt::DisplayRole).toString());
//        return QSize(fm.horizontalAdvance(str), fm.height());
//    }

//    switch(static_cast<eColumns>(column))
//    {
//    case eColumns::GroupID:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return getGroupUID();
//            break;
//        }
//    case eColumns::GroupName:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//            {
//                const AnimTable * tbl = static_cast<const AnimTable*>(parentNode());
//                Q_ASSERT(tbl);
//                return tbl->data( tbl->getModel()->index(nodeIndex(), column, QModelIndex()), role);
//            }
//            break;
//        }
//    case eColumns::NbSlots:
//        {
//            if(role == Qt::DisplayRole || role == Qt::EditRole)
//                return m_seqlist.size();
//            break;
//        }
//    default:
//        {
//            break;
//        }
//    };
//    return QVariant();
//}

void AnimGroup::importGroup(const fmt::AnimDB::animgrp_t &grp)
{
    _insertChildrenNodes(0, grp.seqs.size());
    for(int i = 0; i < m_container.size(); ++i)
        m_container[i]->setSeqRefID(grp.seqs[i]);
    m_unk16 = grp.unk16;
}

fmt::AnimDB::animgrp_t AnimGroup::exportGroup()const
{
    fmt::AnimDB::animgrp_t dest;
    const int nbrefs = m_container.size();
    dest.seqs.resize(nbrefs);
    for(int i = 0; i < nbrefs; ++i)
        dest.seqs[i] = m_container[i]->getSeqRefID();
    dest.unk16 = m_unk16;
    return dest;
}

bool AnimGroup::operator==(const AnimGroup &other) const
{
    return this == &other;
}

bool AnimGroup::operator!=(const AnimGroup &other) const
{
    return !operator==(other);
}

void AnimGroup::removeSequenceReferences(AnimGroup::animseqid_t id)
{
    for( auto & seq : m_container )
    {
        if(seq->getSeqRefID() == id)
            seq->setSeqRefID(-1);
    }
}

void AnimGroup::setAnimSlotRef(int slot, AnimGroup::animseqid_t id)
{
    AnimSequenceReference * ref =  m_container[slot];
    ref->setSeqRefID(id);
}

AnimGroup::animseqid_t AnimGroup::getAnimSlotRef(int slot) const
{
    const AnimSequenceReference * ref =  m_container[slot];
    return ref->getSeqRefID();
}

uint16_t AnimGroup::getUnk16() const
{
    return m_unk16;
}

void AnimGroup::setUnk16(uint16_t val)
{
    m_unk16 = val;
}

int AnimGroup::getGroupUID() const
{
    return nodeIndex();
}

QPixmap AnimGroup::MakeGroupPreview(const Sprite* owner, int maxWidth, int maxHeight, int maxNbImages) const
{
    if(!owner)
        throw std::runtime_error("AnimGroup::MakeGroupPreview(): Got null owner sprite pointer!");
    const int MAX_NB_IMGS       = maxNbImages > 0   ? maxNbImages : nodeChildCount();
    const int WIDTH_PER_IMAGE   = maxWidth > 0      ? (maxWidth/MAX_NB_IMGS) : 0;
    const int HEIGTH_PER_IMAGE  = maxHeight;
    QPixmap target(maxWidth, maxHeight);
    QPainter stitcher;

    int curX = 0; //Keep track of the top corner position to place the next image at

    stitcher.begin(&target);
    stitcher.setBackground(QBrush(owner->getPalette().front()));
    for(const auto node : m_container)
    {
        const AnimSequence * seq =  owner->getAnimSequence(node->getSeqRefID());
        QImage preview(seq->makePreview(owner));
        stitcher.drawImage(curX, 0, preview.scaled(WIDTH_PER_IMAGE, HEIGTH_PER_IMAGE, Qt::KeepAspectRatio));
        curX += WIDTH_PER_IMAGE;
    }
    stitcher.end();
    //target.save("D:/Users/Guill/Documents/CodingProjects/ppmdu_spriteeditor/workdir/test.png", "png");
    return target;
}

TreeNode *AnimGroup::clone() const
{
    return new AnimGroup(*this);
}

eTreeElemDataType AnimGroup::nodeDataTy() const
{
    return eTreeElemDataType::animGroup;
}

const QString &AnimGroup::nodeDataTypeName() const
{
    return ElemName_AnimGroup;
}

QString AnimGroup::nodeDisplayName() const
{
    //#TODO: anim groups should be named by the animation they're for maybe?
    return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());
}
