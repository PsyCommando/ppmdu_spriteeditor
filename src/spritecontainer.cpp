#include "spritecontainer.h"
#include <QMessageBox>
#include <QString>
#include <QSaveFile>
#include <QtConcurrent>
#include <QtConcurrentMap>
#include <QFuture>
#include <QFutureWatcher>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/packfile.hpp>
#include <src/spritemanager.h>
#include <src/sprite.h>
#include <dialogprogressbar.hpp>

namespace spr_manager
{
    const QString PackFileExt = "bin";
    const QString WANFileExt  = "wan";
    const QString WATFileExt  = "wat";


    const QList<QString> SpriteContainer::SpriteContentCategories=
    {
        "Images",
        "Frames",
        "Sequences",
        "Animations",
        "Palette",
        "Effects Offsets",
    };

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
                ldr.CopyEntryData( cnt, std::back_inserter(newspr.m_raw) );
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
            std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front().m_raw) );
            manager.endInsertRows();
            m_cntTy = eContainerType::WAN;
        }
        else if( m_srcpath.endsWith(WATFileExt) )
        {
            //We load the whole sprite
            manager.beginInsertRows( QModelIndex(), 0, 0);
            m_spr.push_back(Sprite(this));
            std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front().m_raw) );
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
        emit showProgress(pmthw->curop);
        qDebug("SpriteContainer::WriteContainer(): progress dialog displayed!\n");
        return 0;
    }

    void SpriteContainer::ImportContainer(const QString &path)
    {
        Q_ASSERT(false); //Need to be done!
    }

    void SpriteContainer::ExportContainer(const QString &path) const
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

    void SpriteContainer::FetchToC(QDataStream &fdat)
    {

    }

    void SpriteContainer::LoadEntry(SpriteContainer::sprid_t idx)
    {

    }

    ThreadedWriter::ThreadedWriter(QSaveFile *sfile, SpriteContainer *cnt)
        :QObject(nullptr),savefile(sfile), sprdata(cnt), bywritten(0)
    {
        connect(this, SIGNAL(finished()), this, SLOT(OnFinished()));
    }

    ThreadedWriter::~ThreadedWriter()
    {qDebug("ThreadedWriter::~ThreadedWriter()\n");}

    void ThreadedWriter::WritePack()
    {
        Q_ASSERT(sprdata->hasChildren());
        fmt::PackFileWriter writer;
        bywritten = 0;

        curop = QtConcurrent::map( sprdata->begin(),
                                   sprdata->end(),
                                   [&](Sprite & curspr)
        {
            try
            {
                if(curspr.wasParsed())
                    curspr.CommitSpriteData();
                QMutexLocker lk(&mtxdata);
                writer.AppendSubFile(curspr.getRawData().begin(), curspr.getRawData().end());
            }
            catch(const std::exception & e)
            {
                qWarning(e.what());
            }
        });

        QFutureSynchronizer<void> futsync;
        futsync.addFuture(curop);
        futsync.waitForFinished();

        QDataStream outstr(savefile.data());
        QByteArray  out;
        writer.Write(std::back_inserter(out));
        bywritten = outstr.writeRawData( out.data(), out.size() );
        savefile->commit();
        emit finished(bywritten);
        emit finished();
        qDebug("ThreadedWriter::WritePack(): Finished!\n");
    }

    void ThreadedWriter::WriteSprite()
    {
        Q_ASSERT(sprdata->hasChildren());
        QDataStream     outstr(savefile.data());
        Sprite          &curspr = sprdata->GetSprite(0);
        bywritten   = 0;

        curop = QtConcurrent::run( [&]()
        {
            curspr.CommitSpriteData();
            bywritten = outstr.writeRawData( (const char *)(curspr.getRawData().data()), curspr.getRawData().size() );
            savefile->commit();
        });
        QFutureSynchronizer<void> futsync;
        futsync.addFuture(curop);
        futsync.waitForFinished();
        emit finished(bywritten);
        emit finished();
        qDebug("ThreadedWriter::WritePack(): WriteSprite!\n");
    }

    void ThreadedWriter::OnFinished()
    {
    }


};
