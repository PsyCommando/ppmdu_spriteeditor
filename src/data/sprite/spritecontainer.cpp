#include "spritecontainer.hpp"
#include <QMessageBox>
#include <QString>
#include <QSaveFile>
#include <QtConcurrent>
#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/packfile.hpp>
#include <src/data/sprite/spritemanager.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/ui/dialogprogressbar.hpp>

namespace spr_manager
{
    const QString PackFileExt = "bin";
    const QString WANFileExt  = "wan";
    const QString WATFileExt  = "wat";

//=================================================================================================
//  SpriteContainer
//=================================================================================================
    const QList<QString> SpriteContainer::SpriteContentCategories=
    {
        "Images",
        "Frames",
        "Sequences",
        "Animations",
        "Palette",
        "Effects Offsets",
    };

    SpriteContainer::SpriteContainer(QObject *parent)
        :QObject(parent), TreeElement(nullptr), m_cntTy(eContainerType::NONE)
    {
    }

    SpriteContainer::SpriteContainer(const QString &str, QObject *parent)
        :QObject(parent), TreeElement(nullptr), m_srcpath(str), m_cntTy(eContainerType::NONE)//, m_rootelem(this)
    {}

    SpriteContainer::~SpriteContainer()
    {
        //m_rootelem = nullptr;
        qDebug("SpriteContainer::~SpriteContainer(): Deleting sprite container!\n");
        if(m_workthread.isRunning())
        {
            m_workthread.terminate();
            qDebug("SpriteContainer::~SpriteContainer(): Waiting on thread..\n");
            m_workthread.wait();
        }
        qDebug("SpriteContainer::~SpriteContainer(): Done!\n");
    }

    void SpriteContainer::clone(const TreeElement *)
    {
        throw std::runtime_error("SpriteContainer::clone(): Copy is deactivated!");
    }

    bool SpriteContainer::ContainerLoaded() const
    {
        return !m_spr.empty() || (m_cntTy != eContainerType::NONE);
    }

    bool SpriteContainer::ContainerIsPackFile() const
    {
        return ContainerLoaded() && m_cntTy == eContainerType::PACK;
    }

    bool SpriteContainer::ContainerIsSingleSprite() const
    {
        return !ContainerIsPackFile();
    }

    void SpriteContainer::LoadContainer()
    {
        QFile container(m_srcpath);

        if( !container.open(QIODevice::ReadOnly) || !container.exists() || container.error() != QFileDevice::NoError )
        {
            //Error can't load file!
            qWarning( container.errorString().toLocal8Bit().data() );
            QMessageBox msgBox;
            msgBox.setText("Failed to load file!");
            msgBox.setInformativeText(container.errorString());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        QByteArray data = container.readAll();
        SpriteManager & manager = SpriteManager::Instance();

        m_spr.clear();
        //Lets identify the format
        if(m_srcpath.endsWith(PackFileExt))
        {
            fmt::PackFileLoader ldr;
            ldr.Read(data.begin(), data.end());
            m_spr.reserve(ldr.size());

            //Load the raw data into each sprites but don't parse them yet!
            manager.beginInsertRows( QModelIndex(), 0, ldr.size() - 1);

            for( size_t cnt = 0; cnt < ldr.size(); ++cnt )
            {
                Sprite newspr(this);
                ldr.CopyEntryData( cnt, std::back_inserter(newspr.getRawData()) );
                m_spr.push_back(std::move(newspr));
            }
            manager.endInsertRows();

            m_cntTy = eContainerType::PACK;
        }
        else if( m_srcpath.endsWith(WANFileExt) )
        {           
            //We load the whole sprite
            manager.beginInsertRows( QModelIndex(), 0, 0);
            m_spr.push_back(Sprite(this));
            std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front().getRawData()) );
            manager.endInsertRows();
            m_cntTy = eContainerType::WAN;
        }
        else if( m_srcpath.endsWith(WATFileExt) )
        {
            //We load the whole sprite
            manager.beginInsertRows( QModelIndex(), 0, 0);
            m_spr.push_back(Sprite(this));
            std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front().getRawData()) );
            manager.endInsertRows();
            m_cntTy = eContainerType::WAT;
        }

    }

    int SpriteContainer::WriteContainer()
    {
        QScopedPointer<QSaveFile> pcontainer( new  QSaveFile(m_srcpath));

        if( !pcontainer->open(QIODevice::WriteOnly) || pcontainer->error() != QFileDevice::NoError )
        {
            //Error can't write file!
            qWarning( pcontainer->errorString().toLocal8Bit() );
            QMessageBox msgBox;
            msgBox.setText("Failed to write file!");
            msgBox.setInformativeText(pcontainer->errorString());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return 0;
        }

        ThreadedWriter * pmthw = new ThreadedWriter(pcontainer.take(), spr_manager::SpriteManager::Instance().getContainer());
        connect( &m_workthread, SIGNAL(finished()), pmthw, SLOT(deleteLater()) );
        connect(pmthw, SIGNAL(finished()), &m_workthread, SLOT(quit()));
        connect(pmthw, SIGNAL(finished()), pmthw, SLOT(deleteLater()));

        //
        switch(m_cntTy)
        {
        case eContainerType::PACK:
            {
                connect(&m_workthread, SIGNAL(started()), pmthw, SLOT(WritePack()));
                pmthw->moveToThread(&m_workthread);
                break;
            }
        case eContainerType::WAN:
        case eContainerType::WAT:
            {
                connect(&m_workthread, SIGNAL(started()), pmthw, SLOT(WriteSprite()));
                pmthw->moveToThread(&m_workthread);
                break;
            }
        default:
            {
                Q_ASSERT(false);
                qFatal("SpriteContainer::WriteContainer(): Tried to write unknown filetype!!");
                break;
            }
        };
        m_workthread.start();
        emit showProgress(pmthw->op1, pmthw->op2);
        qDebug("SpriteContainer::WriteContainer(): progress dialog displayed!\n");
        return 0;
    }

    void SpriteContainer::ImportContainer(const QString &/*path*/)
    {
        Q_ASSERT(false); //Need to be done!
    }

    void SpriteContainer::ExportContainer(const QString &/*path*/) const
    {
        Q_ASSERT(false); //Need to be done!
    }

    Sprite &SpriteContainer::GetSprite(SpriteContainer::sprid_t idx)
    {
        return m_spr[idx];
    }

    SpriteContainer::sprid_t SpriteContainer::AddSprite()
    {
        SpriteManager & manager = SpriteManager::Instance();
        size_t offset = m_spr.size();

        manager.beginInsertRows( QModelIndex(), offset, offset );
        m_spr.push_back( Sprite(this) );
        manager.endInsertRows();

        return offset;
    }

    QVariant SpriteContainer::data(const QModelIndex &index, int role) const
    {
        if(!ContainerLoaded() || m_spr.empty())
            return QVariant();

        if (!index.isValid())
            return QVariant("root");

        if (role != Qt::DisplayRole && role != Qt::EditRole)
            return QVariant();

        TreeElement *item = const_cast<SpriteContainer*>(this)->getItem(index);
        return item->nodeData(index.column(), role);
    }

    QVariant SpriteContainer::headerData(int, Qt::Orientation, int) const
    {
        //nothing really
        return QVariant();
    }

    int SpriteContainer::rowCount(const QModelIndex &parent) const
    {
        if (!ContainerLoaded() || m_spr.empty())
            return 0;

        TreeElement *parentItem = const_cast<SpriteContainer*>(this)->getItem(parent);
        Q_ASSERT(parentItem);

        //Exclude some items from being displayed as having childrens!
        if(parentItem->getNodeDataTy() == eTreeElemDataType::animSequence ||
           //parentItem->getNodeDataTy() == eTreeElemDataType::animTable    ||
           parentItem->getNodeDataTy() == eTreeElemDataType::frame)
            return 0;

        return parentItem->nodeChildCount();
    }

    bool SpriteContainer::hasChildren(const QModelIndex &parent) const
    {
        if(!ContainerLoaded() || m_spr.empty())
            return false;

        return rowCount(parent) > 0;
//        TreeElement * parentItem = const_cast<SpriteContainer*>(this)->getItem(parent);

//        if(parentItem)
//            return parentItem->nodeChildCount() > 0;
//        else
//            return false;
    }

    TreeElement *SpriteContainer::getItem(const QModelIndex &index)
    {
        if (index.isValid())
        {
            TreeElement *item = reinterpret_cast<TreeElement*>(index.internalPointer());
            if (item)
                return item;
        }
        return this;
    }

    int SpriteContainer::columnCount(const QModelIndex &) const
    {
        //            if (parent.isValid())
        //                return static_cast<TreeElement*>(parent.internalPointer())->columnCount();
        //            else
        return 1;
    }

    void SpriteContainer::appendChild(TreeElement *item)
    {
        QMutexLocker lk(&getMutex());
        Sprite * spritem = nullptr;
        spritem = dynamic_cast<Sprite*>(item);

        if(spritem)
            m_spr.append(*spritem);
    }

    TreeElement *SpriteContainer::nodeChild(int row)
    {
        return &m_spr[row];
    }

    int SpriteContainer::nodeChildCount() const
    {
        return m_spr.count();
    }

    int SpriteContainer::nodeIndex() const
    {
        return 0; //Always first!
    }

    int SpriteContainer::indexOfNode(const TreeElement *ptr) const
    {
        QMutexLocker lk(&const_cast<SpriteContainer*>(this)->getMutex());
        const Sprite * ptrspr = dynamic_cast<const Sprite *>(ptr);

        if( ptrspr )
            return m_spr.indexOf(*ptrspr);
        return 0;
    }

    QModelIndex SpriteContainer::modelIndexOfNode(const TreeElement *ptr) const
    {
        //#TODO: Its pretty useless if its limited to only sprites......
        QMutexLocker lk(&const_cast<SpriteContainer*>(this)->getMutex());

        if(ptr->getNodeDataTy() != eTreeElemDataType::sprite)
        {
            qFatal(QString("SpriteContainer::modelIndexOfNode(): Got non-sprite node! Type is %1").arg(static_cast<int>(ptr->getNodeDataTy())).toStdString().c_str());
            return QModelIndex();
        }

        const Sprite * ptrspr = dynamic_cast<const Sprite *>(ptr);
        Q_ASSERT(ptrspr);

        int idx = m_spr.indexOf(*ptrspr);
        if(idx == -1)
        {
            Q_ASSERT(false);
            throw std::runtime_error("SpriteContainer::modelIndexOfNode(): Couldn't find node in table! Node is possibly not a sprite!");
        }

        return spr_manager::SpriteManager::Instance().index(idx, 0);
    }

    QModelIndex spr_manager::SpriteContainer::modelIndex() const
    {
        return QModelIndex();
    }

    QModelIndex spr_manager::SpriteContainer::modelChildIndex(int row, int column) const
    {
        return spr_manager::SpriteManager::Instance().index(row, column);
    }

    QModelIndex spr_manager::SpriteContainer::modelParentIndex() const
    {
        return spr_manager::SpriteManager::Instance().parent();
    }

    int SpriteContainer::nodeColumnCount() const
    {
        return 1;
    }

    TreeElement *SpriteContainer::parentNode()
    {
        return m_parentItem;
    }

    Sprite *SpriteContainer::parentSprite()
    {
        return nullptr;
    }

    QVariant SpriteContainer::nodeData(int column, int role) const
    {
        if( (role == Qt::DisplayRole || role == Qt::EditRole) && column != 0)
            return QVariant(getSrcFnameOnly());
        return QVariant();
    }

    bool SpriteContainer::insertChildrenNodes(int position, int count)
    {
        QMutexLocker lk(&getMutex());
        if(position > m_spr.size())
            return false;

        for( int i = 0; i < count; ++i )
            m_spr.insert(position, Sprite(this));
        return true;
    }

    bool SpriteContainer::removeChildrenNodes(int position, int count)
    {
        QMutexLocker lk(&getMutex());
        if(position + (count-1) > m_spr.size())
            return false;
        for(int i = 0; i < count; ++i)
            m_spr.removeAt(position);
        return true;
    }

    bool SpriteContainer::removeChildrenNodes(QModelIndexList indices)
    {
        bool success = true;
        for(QModelIndex idx : indices)
        {
            if(!idx.isValid())
                continue;
            TreeElement * node = static_cast<TreeElement*>(idx.internalPointer());
            Q_ASSERT(node);
            success = (removeChildrenNodes(node->nodeIndex(), 1) && success);
        }
        return success;
    }

    bool SpriteContainer::moveChildrenNodes(int srcrow, int count, int destrow)
    {
        QMutexLocker lk(&getMutex());

        if( srcrow + count > m_spr.size() || destrow > m_spr.size() )
        {
            Q_ASSERT(false);
            return false;
        }

        if(destrow > srcrow)
        {
            for( int i = 0; i < count; ++i )
                m_spr.move(srcrow, destrow);
        }
        else
        {
            for( int i = 0; i < count; ++i )
                m_spr.move(srcrow, destrow + i);
        }

        return true;
    }

    bool SpriteContainer::nodeIsMutable() const {return false;}

    void SpriteContainer::FetchToC(QDataStream &/*fdat*/)
    {

    }

    void SpriteContainer::LoadEntry(SpriteContainer::sprid_t /*idx*/)
    {

    }

    QString SpriteContainer::getSrcFnameOnly() const
    {
        return m_srcpath.mid( m_srcpath.lastIndexOf('/') );
    }

    //Incremental load
    void SpriteContainer::fetchMore(const QModelIndex &parent)
    {
        if (!parent.isValid() || !ContainerLoaded())
            return;
        TreeElement * pte = getItem(parent);
        if(!pte || (pte && pte == this))
            return;
        pte->fetchMore(parent);
    }

    bool SpriteContainer::canFetchMore(const QModelIndex &parent) const
    {
        if (!parent.isValid() || !ContainerLoaded())
            return false;
        TreeElement * pte = const_cast<SpriteContainer*>(this)->getItem(parent);
        if(!pte || (pte && pte == this))
            return false;
        return pte->canFetchMore(parent);
    }

    //=================================================================================================
    //  ThreadedWriter
    //=================================================================================================
    ThreadedWriter::ThreadedWriter(QSaveFile *sfile, SpriteContainer *cnt)
        :QObject(nullptr),savefile(sfile), sprdata(cnt), bywritten(0)
    {
        //connect(this, SIGNAL(finished()), this, SLOT(OnFinished()));
    }

    ThreadedWriter::~ThreadedWriter()
    {qDebug("ThreadedWriter::~ThreadedWriter()\n");}

    void ThreadedWriter::WritePack()
    {
        try
        {
            _WritePack();
        }
        catch(const std::exception & e)
        {
            qCritical(e.what());
        }
    }

    void ThreadedWriter::WriteSprite()
    {
        try
        {
            _WriteSprite();
        }
        catch(const std::exception & e)
        {
            qCritical(e.what());
        }
    }

    void ThreadedWriter::_WriteSprite()
    {
        Q_ASSERT(sprdata && sprdata->hasChildren());
        QDataStream     outstr(savefile.data());
        Sprite          &curspr = sprdata->GetSprite(0);
        bywritten   = 0;

        //Setup the functions
        op1 = QtConcurrent::run( [&]()
        {
            try
            {
                curspr.CommitSpriteData();
                bywritten = outstr.writeRawData( (const char *)(curspr.getRawData().data()), curspr.getRawData().size() );
                savefile->commit();
            }
            catch(const std::exception & e )
            {
                qCritical(e.what());
            }
        });
        op2 = QtConcurrent::run( [](){} ); //dummy op

        QFutureSynchronizer<void> futsync;
        futsync.addFuture(op1);
        futsync.waitForFinished();


        op2.waitForFinished();
        emit finished(bywritten);
        emit finished();
        qDebug("ThreadedWriter::WritePack(): WriteSprite!\n");
    }

    void ThreadedWriter::_WritePack()
    {
        Q_ASSERT(sprdata && sprdata->hasChildren());
        bywritten = 0;

        //Commit all sprites that need it!
        op1 = QtConcurrent::map( sprdata->begin(),
                                   sprdata->end(),
                                   [&](Sprite & curspr)
        {
            try
            {
                if(curspr.wasParsed())
                    curspr.CommitSpriteData();
            }
            catch(const std::exception & e)
            {
                qCritical(e.what());
            }
        });
        op2 = QtConcurrent::run([&]()
        {
            op1.waitForFinished();
            //Fill up the packfilewriter
            fmt::PackFileWriter writer;
            for( Sprite & spr : *sprdata )
                writer.AppendSubFile( spr.getRawData().begin(), spr.getRawData().end() );

            //Build the packfile
            QDataStream outstr(savefile.data());
            QByteArray  out;
            writer.Write(std::back_inserter(out));

            //Write it to file!
            bywritten = outstr.writeRawData( out.data(), out.size() );
            savefile->commit();
        });

        //Wait for it to finish
        QFutureSynchronizer<void> futsync;
        futsync.addFuture(op1);
        futsync.addFuture(op2);
        futsync.waitForFinished();

        emit finished(bywritten);
        emit finished();
        qDebug("ThreadedWriter::WritePack(): Finished!\n");
    }

};
