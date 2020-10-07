#include "animframe_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/animframe.hpp>

//Name displayed in the column header for each properties of the frame! Is tied to eColumnsType
const QStringList AnimFrameColumnNames
{
    "Frame",
    "Duration",
    "Offset",
    "Shadow Offset",
    "Flags",
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

int AnimFramesModel::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return AnimFrameColumnNames.size();//static_cast<int>(eAnimFrameColumnsType::NBColumns);
}

QVariant AnimFramesModel::data(const QModelIndex &index, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    const AnimFrame * animfrm = static_cast<const AnimFrame*>(getItem(index));

    switch(static_cast<eAnimFrameColumnsType>(index.column()))
    {
    case eAnimFrameColumnsType::Frame: //Frame
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
                QSize sz = calcTextSize(data(index, Qt::DisplayRole).toString());
                sz.setWidth( sz.width() + 64 ); //Compensate for thumbnail
                return sz;
            }
            break;
        }
    case eAnimFrameColumnsType::Duration: //duration
        {
            if( role == Qt::DisplayRole )
                return QString("%1t").arg( static_cast<int>(animfrm->duration()) );
            else if(role == Qt::EditRole)
                return QVariant(static_cast<int>(animfrm->duration()));
            else if( role == Qt::SizeHintRole )
                return calcTextSize(data(index, Qt::DisplayRole).toString());
            break;
        }
    case eAnimFrameColumnsType::Offset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(animfrm->xoffset()).arg(animfrm->yoffset());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(animfrm->xoffset(), animfrm->yoffset()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( data(index, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eAnimFrameColumnsType::ShadowOffset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(animfrm->shadowx()).arg(animfrm->shadowy());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(animfrm->shadowx(), animfrm->shadowy()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( data(index, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eAnimFrameColumnsType::Flags: //flag
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(animfrm->flags());
            else if( role == Qt::SizeHintRole )
                return calcTextSize( data(index, Qt::DisplayRole).toString() );
            break;
        }

    // -- Direct acces via nodedate! --
    case eAnimFrameColumnsType::Direct_ShadowXOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->shadowx()));
            break;
        }
    case eAnimFrameColumnsType::Direct_ShadowYOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->shadowy()));
            break;
        }
    case eAnimFrameColumnsType::Direct_XOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->xoffset()));
            break;
        }
    case eAnimFrameColumnsType::Direct_YOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(animfrm->yoffset()));
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

    switch(static_cast<eAnimFrameColumnsType>(index.column()))
    {
    case eAnimFrameColumnsType::Frame:
        {
            pfrm->setFrmidx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case eAnimFrameColumnsType::Duration:
        {
            pfrm->setDuration(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }
    case eAnimFrameColumnsType::Offset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                pfrm->setXoffset(offs.first);
                pfrm->setYoffset(offs.second);
            }
            else
                qDebug("AnimSequence::setData(): Couldn't convert offset value to a QPair!\n");
            break;
        }
    case eAnimFrameColumnsType::ShadowOffset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                pfrm->setShadowx(offs.first);
                pfrm->setShadowy(offs.second);
            }
            else
                qDebug("AnimSequence::setData(): Couldn't convert shadow offset value to a QPair!\n");
            break;
        }
    case eAnimFrameColumnsType::Flags:
        {
            pfrm->setFlags(static_cast<uint8_t>(value.toInt(&bok)));
            break;
        }

    //direct access columns
    case eAnimFrameColumnsType::Direct_XOffset:
        {
            pfrm->setXoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case eAnimFrameColumnsType::Direct_YOffset:
        {
            pfrm->setYoffset(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case eAnimFrameColumnsType::Direct_ShadowXOffset:
        {
            pfrm->setShadowx(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    case eAnimFrameColumnsType::Direct_ShadowYOffset:
        {
            pfrm->setShadowy(static_cast<int16_t>(value.toInt(&bok)));
            break;
        }
    //Undefined cases
    default:
        return false;
    };

    if(bok && index.model())
    {
        QVector<int> roles{role};
        QAbstractItemModel* pmod = const_cast<QAbstractItemModel*>(index.model());
        if(!pmod)
            qFatal("Model is null!");

        pmod->dataChanged(index, index, roles);
    }
    return bok;
}

QVariant AnimFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return QVariant( QString("%1").arg(section) );
    }
    else if( orientation == Qt::Orientation::Horizontal && section < AnimFrameColumnNames.size() )
    {
        return AnimFrameColumnNames[section];
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

QSize AnimFramesModel::calcTextSize(const QString &text)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.horizontalAdvance(text), fm.height());
}
