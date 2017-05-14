#include "sprite_anim.hpp"
#include <src/sprite.h>

const QList<QVariant> AnimSequences::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Nb frames"),
};


const QList<QVariant> AnimSequence::HEADER_COLUMNS
{
    QString("Preview"),
    QString("Duration"),
    QString("X"),
    QString("Y"),
    QString("Shadow X"),
    QString("Shadow Y"),
    QString("Flag"),
};

AnimSequences::AnimSequences(TreeElement *parent)
    :BaseListContainerChild(parent), m_model(this)
{}

AnimSequences::AnimSequences(const AnimSequences &cp)
    :BaseListContainerChild(cp), m_model(this)
{

}

AnimSequences::AnimSequences(AnimSequences &&mv)
    :BaseListContainerChild(mv), m_model(this)
{

}

AnimSequences &AnimSequences::operator=(const AnimSequences &cp)
{
    BaseListContainerChild::operator=(cp);
    return *this;
}

AnimSequences &AnimSequences::operator=(AnimSequences && mv)
{
    BaseListContainerChild::operator=(mv);
    return *this;
}

QVariant AnimSequences::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    return static_cast<TreeElement*>(index.internalPointer())->data(index.column(), role);
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

int AnimSequences::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeElement*>(parent.internalPointer())->columnCount();
    else
        return HEADER_COLUMNS.size();
}

AnimSequence *AnimSequences::getSequenceByID(fmt::AnimDB::animseqid_t id)
{
    return static_cast<AnimSequence*>(child(id));
}

void AnimSequences::removeSequence(fmt::AnimDB::animseqid_t id)
{
    removeChildren(id,1);
}

void AnimSequences::importSequences(const fmt::AnimDB::animseqtbl_t &src)
{
    removeChildren(0, childCount());
    insertChildren(0, src.size());

    for( fmt::AnimDB::animseqid_t cntid = 0; cntid < src.size(); ++cntid )
        m_container[cntid].importSeq(src.at(cntid));
}

fmt::AnimDB::animseqtbl_t AnimSequences::exportSequences()
{
    fmt::AnimDB::animseqtbl_t seqs(childCount());
    for( int cntid = 0; cntid < childCount(); ++cntid )
        seqs[cntid] = std::move(m_container[cntid].exportSeq());

    return qMove(seqs);
}

AnimSequences::model_t &AnimSequences::getModel() {return m_model;}

Sprite *AnimFrame::parentSprite()
{
    return static_cast<AnimSequence*>(parent())->parentSprite();
}

QVariant AnimFrame::data(int column, int role) const
{
    if( role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole )
        return QVariant();

    if( role == Qt::DisplayRole || role == Qt::EditRole )
    {
        switch(column)
        {
        case 0: //preview
            return QVariant(QString("%1#%2").arg(ElemName()).arg(childNumber()));
        case 1: //duration
            return QVariant(static_cast<int>(duration()));
        case 2: //x
            return QVariant(static_cast<int>(xoffset()));
        case 3: //y
            return QVariant(static_cast<int>(yoffset()));
        case 4: //shadow x
            return QVariant(static_cast<int>(shadowx()));
        case 5: //shadow y
            return QVariant(static_cast<int>(shadowy()));
        case 6: //flag
            return QVariant(static_cast<int>(flags()));
        };
    }
    else if( role == Qt::DecorationRole && column == 0 )
    {
        MFrame * pframe = const_cast<AnimFrame*>(this)->parentSprite()->getFrame(frmidx());
        if(pframe)
        {
            QRect area;
            return QVariant(QPixmap::fromImage(pframe->AssembleFrame(0,0, area)));
        }
//        MFrame * pframe = const_cast<AnimFrame*>(this)->parentSprite()->getFrame(frmidx());
//        if(pframe && m_cached.isNull() && m_mtxcache.tryLock(1))
//        {
//            m_futimg = QtConcurrent::run( this, &AnimFrame::BuildCachedImage, pframe);
//            connect( &m_futimgwatch, SIGNAL(finished()), &m_updatenotifier, SLOT(imagecached()) );
//            m_futimgwatch.setFuture(m_futimg);
//            m_mtxcache.unlock();
//        }
//        else if(pframe && !m_cached.isNull() && m_mtxcache.tryLock(1))
//        {
//            m_mtxcache.unlock();
//            QMutexLocker lk(&m_mtxcache);
//            return QVariant(m_cached);
//        }
    }

    return QVariant();
}

//void AnimFrame::update()
//{
//    MFrame * pframe = const_cast<AnimFrame*>(this)->parentSprite()->getFrame(frmidx());
//    if(!pframe)
//    {
//        qDebug("AnimFrame::update(): Updated invalid frame..\n");
//        return;
//    }
//    QMutexLocker lk(&const_cast<AnimFrame*>(this)->m_mtxcache);

//    m_futimg = QtConcurrent::run( this, &AnimFrame::BuildCachedImage, pframe);
//    connect( &m_futimgwatch, SIGNAL(finished()), &m_updatenotifier, SLOT(imagecached()) );
//    m_futimgwatch.setFuture(m_futimg);
//}

//QPixmap AnimFrame::BuildCachedImage( MFrame * pframe ) const
//{
//    //**************FIXME!! This is not thread safe!!!!!*******************
//    //Frames are not mutex-protected and their state can change if the user changes them while we're
//    //drawing the image!!
//    //Ideally we might want some external class that renders images for us instead!


//    QMutexLocker lk(&const_cast<AnimFrame*>(this)->m_mtxcache);
//    QRect area;
//    return qMove(QPixmap::fromImage(pframe->AssembleFrame(0,0, area)));
//}
