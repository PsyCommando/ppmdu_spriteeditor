#include "animframe.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animsequence.hpp>

const char * ElemName_AnimFrame     = "Anim Frame";

//***********************************************************************************
//  AnimFrame
//***********************************************************************************
const char * AnimFrame::UProp_AnimFrameID = "AnimFrameID";

const QStringList AnimFrame::ColumnNames
{
    "Frame",
    "Duration",
    "Offset",
    "Shadow Offset",
    "Flags",
};

QSize AnimFrame::calcTextSize(const QString &str)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.horizontalAdvance(str), fm.height());
}

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

    switch(static_cast<eColumnsType>(column))
    {
    case eColumnsType::Frame: //Frame
        {
            if( role == Qt::DisplayRole )
                return QVariant(QString("FrameID: %1").arg( static_cast<int>(frmidx()) ));
            else if(role == Qt::EditRole)
                return static_cast<int>(frmidx());
            else if( role == Qt::DecorationRole)
            {
                const MFrame * pframe = parentSprite()->getFrame(frmidx());
                if(pframe)
                    return QVariant(QPixmap::fromImage(pframe->AssembleFrame(0,0, QRect())));
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( sz.width() + 64 ); //Compensate for thumbnail
                return sz;
            }
            break;
        }
    case eColumnsType::Duration: //duration
        {
            if( role == Qt::DisplayRole )
                return QString("%1t").arg( static_cast<int>(duration()) );
            else if(role == Qt::EditRole)
                return QVariant(static_cast<int>(duration()));
            else if( role == Qt::SizeHintRole )
                return calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
            break;
        }
    case eColumnsType::Offset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(xoffset()).arg(yoffset());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(xoffset(), yoffset()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eColumnsType::ShadowOffset:
        {
            if(role == Qt::DisplayRole)
                return QString("(%1, %2)").arg(shadowx()).arg(shadowy());
            else if(role == Qt::EditRole)
            {
                QVariant var;
                var.setValue<QPair<int,int>>( qMakePair(shadowx(), shadowy()) );
                return var;
            }
            else if( role == Qt::SizeHintRole )
            {
                QSize sz = calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
                sz.setWidth( qMax( 100, sz.width() ) );
                return sz;
            }
            break;
        }
    case eColumnsType::Flags: //flag
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant( flags() );
            else if( role == Qt::SizeHintRole )
                return calcTextSize( nodeData(column, Qt::DisplayRole).toString() );
            break;
        }

    // -- Direct acces via nodedate! --
    case eColumnsType::Direct_ShadowXOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(shadowx()));
            break;
        }
    case eColumnsType::Direct_ShadowYOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(shadowy()));
            break;
        }
    case eColumnsType::Direct_XOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(xoffset()));
            break;
        }
    case eColumnsType::Direct_YOffset:
        {
            if( role == Qt::DisplayRole || role == Qt::EditRole )
                return QVariant(static_cast<int>(yoffset()));
            break;
        }
    default:
        break;
    };

    return QVariant();
}

QImage AnimFrame::makePreview() const
{
    //#TODO: split this from ui code!
    const MFrame * pframe = parentSprite()->getFrame(frmidx());
    if(pframe)
        return pframe->AssembleFrame(0,0, QRect());
    return QImage();
}
