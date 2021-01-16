#include "effectoffsetset.hpp"
#include <src/data/sprite/models/effect_set_model.hpp>
#include <src/utility/graphics_util.hpp>

const QString ElemName_EffectOffset = "Effect Offset";
const QString ElemName_EffectOffsetSet = "Effect Offset Set";

const QColor EffectOffset::ColorDefault {255,   0, 0};

//===========================================================
//  EffectOffsetSet
//===========================================================
EffectOffset::EffectOffset(TreeNode * parent, QColor displaycolor)
    :parent_t(parent), m_displayColor(displaycolor)
{
    m_flags |= Qt::ItemFlag::ItemIsEditable;
}

EffectOffset::EffectOffset(const EffectOffset & cp)
    :parent_t(cp)
{}

EffectOffset::EffectOffset(EffectOffset && mv)
    :parent_t(mv)
{}

EffectOffset EffectOffset::operator=(const EffectOffset & cp)
{
    parent_t::operator=(cp);
    m_x = cp.m_x;
    m_y = cp.m_y;
    m_displayColor = cp.m_displayColor;
    return *this;
}

EffectOffset EffectOffset::operator=(EffectOffset && mv)
{
    parent_t::operator=(mv);
    m_x = mv.m_x;
    m_y = mv.m_y;
    m_displayColor = mv.m_displayColor;
    return *this;
}

EffectOffset::~EffectOffset()
{}

int16_t EffectOffset::getX()const
{
    return m_x;
}
int16_t EffectOffset::getY()const
{
    return m_y;
}
void EffectOffset::setX(int16_t x)
{
    m_x = x;
}
void EffectOffset::setY(int16_t y)
{
    m_y = y;
}

TreeNode *EffectOffset::clone() const
{
    return new EffectOffset(*this);
}

eTreeElemDataType EffectOffset::nodeDataTy() const
{
    return eTreeElemDataType::effectOffset;
}

const QString &EffectOffset::nodeDataTypeName() const
{
    return ElemName_EffectOffset;
}

QString EffectOffset::nodeDisplayName() const
{
    switch(static_cast<EffectOffsetSet::eOffsetsPart>(nodeIndex()))
    {
    case EffectOffsetSet::eOffsetsPart::Head:
        return "Head";
    case EffectOffsetSet::eOffsetsPart::RHand:
        return "Right Hand";
    case EffectOffsetSet::eOffsetsPart::LHand:
        return "Left Hand";
    case EffectOffsetSet::eOffsetsPart::Center:
        return "Center";
    default:
        break;
    }
    return parent_t::nodeDisplayName();
}

Qt::ItemFlags EffectOffset::nodeFlags(int column) const
{
    if(column == static_cast<int>(EffectSetModel::eColumns::Name) )
        return Qt::ItemFlags(m_flags).setFlag(Qt::ItemFlag::ItemIsEditable, false); //The name is never editable!
    return m_flags;
}

//===========================================================
//  EffectOffsetSet
//===========================================================


EffectOffsetSet::EffectOffsetSet(TreeNode * parent)
    :parent_t(parent)
{
    QColor displaycol {EffectOffset::ColorDefault};
    for(int i = 0; i < static_cast<int>(eOffsetsPart::NbParts); ++i)
    {
        displaycol.setHsv(RotateHue(displaycol.hue(), 45), displaycol.saturation(), displaycol.value());
        m_offsets.push_back(new EffectOffset(this, displaycol));
    }
}

EffectOffsetSet::EffectOffsetSet(const EffectOffsetSet & cp)
    :parent_t(cp)
{}

EffectOffsetSet::EffectOffsetSet(EffectOffsetSet && mv)
    :parent_t(mv)
{}

EffectOffsetSet EffectOffsetSet::operator=(const EffectOffsetSet & cp)
{
    parent_t::operator=(cp);
    for(int i = 0; i < cp.m_offsets.size(); ++i)
        m_offsets.push_back(cp.m_offsets[i]);
    return *this;
}

EffectOffsetSet EffectOffsetSet::operator=(EffectOffsetSet && mv)
{
    parent_t::operator=(mv);
    m_offsets = qMove(mv.m_offsets);
    for(int i = 0; i < m_offsets.size(); ++i)
        m_offsets[i]->setParentNode(this);
    return *this;
}

EffectOffsetSet::~EffectOffsetSet()
{
    qDeleteAll(m_offsets);
}

void EffectOffsetSet::ImportOffsetSets(fmt::frameoffsets_t offsets)
{
    setHead(offsets.head);
    setRHand(offsets.rhand);
    setLHand(offsets.lhand);
    setCenter(offsets.center);
}

fmt::frameoffsets_t EffectOffsetSet::ExportOffsetSets() const
{
    fmt::frameoffsets_t ofs;
    ofs.head = getHead();
    ofs.rhand = getRHand();
    ofs.lhand = getLHand();
    ofs.center = getCenter();
    return ofs;
}

fmt::offset_t EffectOffsetSet::getHead()const
{
    const EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::Head));
    return fmt::offset_t{ef->getX(), ef->getY()};
}
fmt::offset_t EffectOffsetSet::getRHand()const
{
    const EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::RHand));
    return fmt::offset_t{ef->getX(), ef->getY()};
}
fmt::offset_t EffectOffsetSet::getLHand()const
{
    const EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::LHand));
    return fmt::offset_t{ef->getX(), ef->getY()};
}
fmt::offset_t EffectOffsetSet::getCenter()const
{
    const EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::Center));
    return fmt::offset_t{ef->getX(), ef->getY()};
}

void EffectOffsetSet::setHead(fmt::offset_t offs)
{
    EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::Head));
    ef->setX(offs.xoff);
    ef->setY(offs.yoff);
}
void EffectOffsetSet::setRHand(fmt::offset_t offs)
{
    EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::RHand));
    ef->setX(offs.xoff);
    ef->setY(offs.yoff);
}
void EffectOffsetSet::setLHand(fmt::offset_t offs)
{
    EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::LHand));
    ef->setX(offs.xoff);
    ef->setY(offs.yoff);
}
void EffectOffsetSet::setCenter(fmt::offset_t offs)
{
    EffectOffset * ef = m_offsets.at(static_cast<int>(eOffsetsPart::Center));
    ef->setX(offs.xoff);
    ef->setY(offs.yoff);
}

TreeNode *EffectOffsetSet::clone() const
{
    return new EffectOffsetSet(*this);
}

eTreeElemDataType EffectOffsetSet::nodeDataTy() const
{
    return eTreeElemDataType::effectOffsetSet;
}

const QString &EffectOffsetSet::nodeDataTypeName() const
{
    return ElemName_EffectOffsetSet;
}

QString EffectOffsetSet::nodeDisplayName() const
{
    return QString("%1#%2").arg(nodeDataTypeName()).arg(nodeIndex());
}

TreeNode *EffectOffsetSet::nodeChild(int row)
{
    if(row >= 0 && row < m_offsets.size())
        return m_offsets.at(row);
    return nullptr;
}

int EffectOffsetSet::nodeChildCount() const
{
    return m_offsets.size();
}

int EffectOffsetSet::indexOfChild(const TreeNode * node) const
{
    EffectOffset * ptras = static_cast<EffectOffset*>(const_cast<TreeNode*>(node));
    //Search a matching child in the list!
    if( ptras )
        return m_offsets.indexOf(ptras);
    return -1;
}

bool EffectOffsetSet::_insertChildrenNode(TreeNode *, int )
{
    return false;
}

bool EffectOffsetSet::_insertChildrenNodes(int , int )
{
    return false;
}

bool EffectOffsetSet::_insertChildrenNodes(const QList<TreeNode *> &, int )
{
    return false;
}

bool EffectOffsetSet::_removeChildrenNode(TreeNode *)
{
    return false;
}

bool EffectOffsetSet::_removeChildrenNodes(int , int )
{
    return false;
}

bool EffectOffsetSet::_removeChildrenNodes(const QList<TreeNode *> &)
{
    return false;
}

bool EffectOffsetSet::_deleteChildrenNode(TreeNode *)
{
    return false;
}

bool EffectOffsetSet::_deleteChildrenNodes(int , int )
{
    return false;
}

bool EffectOffsetSet::_deleteChildrenNodes(const QList<TreeNode *> &)
{
    return false;
}

bool EffectOffsetSet::_moveChildrenNodes(int , int , int , TreeNode *)
{
    return false;
}

bool EffectOffsetSet::_moveChildrenNodes(QModelIndexList &, int , QModelIndex )
{
    return false;
}

bool EffectOffsetSet::_moveChildrenNodes(const QList<TreeNode *> &, int , QModelIndex )
{
    return false;
}

