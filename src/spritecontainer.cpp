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
