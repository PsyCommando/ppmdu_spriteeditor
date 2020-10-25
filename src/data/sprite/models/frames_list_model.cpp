#include "frames_list_model.hpp"
#include <src/data/sprite/framescontainer.hpp>

const QString ElemName_Frame = "Frame";

const QList<QString> FramesListHeaderColumnNames
{
    "preview",
    "id",
};

FramesListModel::FramesListModel(FramesContainer * parent, Sprite *parentsprite)
    :TreeNodeModel(nullptr)
{
    m_root = parent;
    m_sprite = parentsprite;
}

FramesListModel::~FramesListModel()
{
}

int FramesListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant FramesListModel::data(const QModelIndex &index, int role) const
{
//    if (!index.isValid())
//        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const MFrame *frm = static_cast<const MFrame*>(getItem(index));
    switch(role)
    {
    case Qt::DecorationRole:
        return QPixmap::fromImage(frm->AssembleFrame(0,0,QRect(), nullptr, true, const_cast<Sprite*>(getOwnerSprite())));
    case Qt::DisplayRole:
        return QString("FrameID:%1").arg(frm->getFrameUID());
    case Qt::EditRole:
        return frm->getFrameUID();
    case Qt::SizeHintRole:
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.horizontalAdvance(data(index, role == Qt::DisplayRole).toInt()+32),
                         qMax(fm.height() + 32, frm->calcFrameBounds().height()) );
        }
    default:
        return QVariant();
    }
}

bool FramesListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return false;
}

QVariant FramesListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return QVariant(QString("%1").arg(section));
    }
    else if( orientation == Qt::Orientation::Horizontal &&
             section < FramesListHeaderColumnNames.size() )
    {
        return FramesListHeaderColumnNames[section];
    }
    return QVariant();
}

TreeNodeModel::node_t *FramesListModel::getRootNode()
{
    return m_root;
}

Sprite *FramesListModel::getOwnerSprite()
{
    return m_sprite;
}

eTreeElemDataType FramesContainer::nodeDataTy() const
{
    return eTreeElemDataType::frames;
}

const QString &FramesContainer::nodeDataTypeName() const
{
    return ElemName_FrameCnt;
}
