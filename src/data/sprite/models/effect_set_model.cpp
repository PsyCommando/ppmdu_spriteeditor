#include "effect_set_model.hpp"
#include <src/data/sprite/effectoffsetcontainer.hpp>
#include <src/data/sprite/sprite.hpp>

const QList<QString> EffectSetModel::ColumnNames
{
    {"Name"},
    {"X"},
    {"Y"},
};

EffectSetModel::EffectSetModel(EffectOffsetSet *poffsets, Sprite *powner)
    :TreeNodeModel()
{
    m_root = poffsets;
    m_sprite = powner;
}

const EffectOffset *EffectSetModel::getHead() const
{
    return static_cast<const EffectOffset *>(m_root->nodeChild(static_cast<int>(EffectOffsetSet::eOffsetsPart::Head)));
}

const EffectOffset *EffectSetModel::getRHand() const
{
    return static_cast<const EffectOffset *>(m_root->nodeChild(static_cast<int>(EffectOffsetSet::eOffsetsPart::RHand)));
}

const EffectOffset *EffectSetModel::getLHand() const
{
    return static_cast<const EffectOffset *>(m_root->nodeChild(static_cast<int>(EffectOffsetSet::eOffsetsPart::LHand)));
}

const EffectOffset *EffectSetModel::getCenter() const
{
    return static_cast<const EffectOffset *>(m_root->nodeChild(static_cast<int>(EffectOffsetSet::eOffsetsPart::Center)));
}

int EffectSetModel::columnCount(const QModelIndex &) const
{
    return ColumnNames.size(); //Extra for aligning things
}

QVariant EffectSetModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::SizeHintRole)
        return QVariant();
    const EffectOffset * poff = static_cast<const EffectOffset *>(getItem(index));
    Q_ASSERT(poff);

    //Set bg color to display color
    if(role == Qt::BackgroundRole)
        return QBrush(poff->getDisplayColor());

    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::Name:
        {
            if(role == Qt::DisplayRole)
                return poff->nodeDisplayName();
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI",9));
                return QSize(128, fm.height());
            }
            break;
        }
        case eColumns::XOffset:
        {
            if(role == Qt::DisplayRole)
                return poff->getX();
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI",9));
                return QSize(fm.horizontalAdvance(data(index, Qt::DisplayRole).toString()), fm.height());
            }
            break;
        }
        case eColumns::YOffset:
        {
            if(role == Qt::DisplayRole)
                return poff->getY();
            else if(role == Qt::SizeHintRole)
            {
                QFontMetrics fm(QFont("Sergoe UI",9));
                return QSize(fm.horizontalAdvance(data(index, Qt::DisplayRole).toString()), fm.height());
            }
            break;
        }
    }
    return QVariant();
}

bool EffectSetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole)
        return false;
    EffectOffset * poff = static_cast<EffectOffset *>(getItem(index));
    Q_ASSERT(poff);
    bool success = false;
    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::XOffset:
        {
            int x = value.toInt(&success);
            if(success)
                poff->setX(x);
            break;
        }
        case eColumns::YOffset:
        {
            int y = value.toInt(&success);
            if(success)
                poff->setY(y);
            break;
        }
        case eColumns::Name:
            break;
    }
    if(success)
        emit dataChanged(index, index, QVector<int>{role} );
    return success;
}

QVariant EffectSetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section >= ColumnNames.size())
        return QVariant();
    if( role != Qt::DisplayRole &&
        role != Qt::SizeHintRole)
        return QVariant();
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::SizeHintRole)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(96, fm.height());
        }
        else if(role == Qt::DisplayRole)
            return ColumnNames[section];
    }
    return QVariant();
}

TreeNodeModel::node_t *EffectSetModel::getRootNode()
{
    return m_root;
}

Sprite *EffectSetModel::getOwnerSprite()
{
    return m_sprite;
}

const Sprite *EffectSetModel::getOwnerSprite() const
{
    return const_cast<EffectSetModel*>(this)->getOwnerSprite();
}
