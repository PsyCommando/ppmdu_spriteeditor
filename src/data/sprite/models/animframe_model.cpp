#include "animframe_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/animframe.hpp>
#include <src/utility/ui_helpers.hpp>

//Name displayed in the column header for each properties of the frame! Is tied to eColumnsType
const std::map<AnimFramesModel::eColumns, QString> AnimFramesModel::ColumnNames
{
    {eColumns::Frame,       "Frame"     },
    {eColumns::Duration,    "Duration"  },
    {eColumns::OffsetX,     "X"         },
    {eColumns::OffsetY,     "Y"         },
    {eColumns::ShadowX,     "Shadow X"  },
    {eColumns::ShadowY,     "Shadow Y"  },
    {eColumns::Flags,       "Flags"     },
};

//***********************************************************************************
//  AnimFramesModel
//***********************************************************************************
AnimFramesModel::AnimFramesModel(AnimSequence *pseq, Sprite *pspr)
    :TreeNodeModel(nullptr)
{
    m_root = pseq;
    m_sprite = pspr;
}

AnimFramesModel::~AnimFramesModel()
{
    m_root = nullptr;
    m_sprite = nullptr;
}

int AnimFramesModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnNames.size();
}

QVariant AnimFramesModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    const AnimFrame * animfrm = static_cast<const AnimFrame*>(getItem(index));

    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::Frame: //Frame
        {
            if( role == Qt::DisplayRole )
                return QVariant(QString("FrameID: %1").arg( static_cast<int>(animfrm->frmidx()) ));
            else if(role == Qt::EditRole)
                return static_cast<int>(animfrm->frmidx());
            else if( role == Qt::DecorationRole)
            {
                const Sprite * pspr = getOwnerSprite();
                const MFrame * pframe = pspr->getFrame(animfrm->frmidx());
                if(pframe)
                    return QVariant(QPixmap::fromImage(pframe->AssembleFrame(0,0, QRect(), nullptr, true, pspr)));
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
                sz.setWidth(sz.width() + 64); //Compensate for thumbnail
                return sz;
            }
            break;
        }
        case eColumns::Duration: //duration
        {
            if( role == Qt::DisplayRole )
                return QString("%1t").arg( static_cast<int>(animfrm->duration()) );
            else if(role == Qt::EditRole)
                return QVariant(static_cast<int>(animfrm->duration()));
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
        case eColumns::OffsetX:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->xoffset()));
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
        case eColumns::OffsetY:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->yoffset()));
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
        case eColumns::ShadowX:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->shadowx()));
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
        case eColumns::ShadowY:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->shadowy()));
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
        case eColumns::Flags: //flag
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(animfrm->flags());
            else if( role == Qt::SizeHintRole )
                return CalculateTextSizeForView(data(index, Qt::DisplayRole).toString());
            break;
        }
    default:
        break;
    };

    return QVariant();
}

bool AnimFramesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || role != Qt::EditRole)
        return false;

    AnimFrame *pfrm = static_cast<AnimFrame*>(getItem(index));//nodeChild(index.row()));
    Q_ASSERT(pfrm);
    bool bok = false;

    switch(static_cast<eColumns>(index.column()))
    {
        case eColumns::Frame:
        {
            pfrm->setFrmidx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::Duration:
        {
            pfrm->setDuration(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::OffsetX:
        {
            pfrm->setXoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::OffsetY:
        {
            pfrm->setYoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::ShadowX:
        {
            pfrm->setShadowx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::ShadowY:
        {
            pfrm->setShadowy(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
        case eColumns::Flags:
        {
            pfrm->setFlags(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }
        default:
            return false; //Return early
    };

    if(bok)
        emit dataChanged(index, index, QVector<int>{role});
    return bok;
}

QVariant AnimFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::SizeHintRole && role != Qt::DecorationRole)
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        const QString rownum = QString("%1").arg(section);
        if(role == Qt::SizeHintRole)
        {
            return TreeNodeModel::headerData(section, orientation, role);
        }
        return QVariant(rownum);
    }
    else if( orientation == Qt::Orientation::Horizontal)
    {
        eColumns col = static_cast<eColumns>(section);
        auto itcol = ColumnNames.find(col);
        if(itcol != ColumnNames.end())
        {
            if(role == Qt::SizeHintRole)
            {
                QSize sz = CalculateTextSizeForView(itcol->second);
                return sz.grownBy(QMargins(8,0,8,0));
            }
            else
                return itcol->second;
        }
    }
    return QVariant();
}

AnimFramesModel::node_t* AnimFramesModel::getRootNode()
{
    return m_root;
}

Sprite *AnimFramesModel::getOwnerSprite()
{
    return m_sprite;
}

const TreeNodeModel::node_t *AnimFramesModel::getRootNode() const
{
    return const_cast<AnimFramesModel*>(this)->getRootNode();
}

const Sprite *AnimFramesModel::getOwnerSprite() const
{
    return const_cast<AnimFramesModel*>(this)->getOwnerSprite();
}

