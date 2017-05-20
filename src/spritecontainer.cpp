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

    void WritePack(QSaveFile * pcontainer, spr_manager::SpriteContainer * cnt)
    {
        fmt::PackFileWriter writer;
        QMutex              mtxdata;
        QVector<QVector<uint8_t>> tmpdata;
        QScopedPointer<QSaveFile> container(pcontainer);

        QFuture<void> savequeue = QtConcurrent::map( cnt->begin(),
                                                     cnt->end(),
                                                     [&writer,&mtxdata,&tmpdata](Sprite & curspr)
        {
            if(curspr.wasParsed())
                curspr.CommitSpriteData();

            QMutexLocker lk(&mtxdata);
            QVector<uint8_t> curdata;
            std::copy( curspr.getRawData().begin(), curspr.getRawData().end(), std::back_inserter(curdata) );
            tmpdata.push_back(curdata);
            //writer.AppendSubFile(curspr.getRawData().begin(), curspr.getRawData().end());
        });


        DialogProgressBar prgbar(savequeue);
        prgbar.setModal(true);
        prgbar.show();

        QFutureSynchronizer<void> futsync;
        futsync.addFuture(savequeue);
        futsync.waitForFinished();

        QDataStream outstr(container.data());
        QByteArray  out;
        writer.Write(std::back_inserter(out));
        outstr.writeRawData( out.data(), out.size() );
        container->commit();
    }


    void SpriteContainer::WriteContainer()
    {
        if( QFile(m_srcpath).exists() )
        {
            //Warn overwrite
            QMessageBox msgBox;
            msgBox.setText("Overwrite?");
            msgBox.setInformativeText( "The file already exists.\nOverwrite?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            if(msgBox.exec() != QMessageBox::Yes)
                return;
        }

        //
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
            return;
        }

        //
        switch(m_cntTy)
        {
        case eContainerType::PACK:
            {
                WritePack(pcontainer.take(), spr_manager::SpriteManager::Instance().getContainer());
                return;
            }
        case eContainerType::WAN:
        case eContainerType::WAT:
            {
                break;
            }
        default:
            {
                Q_ASSERT(false);
                qFatal("SpriteContainer::WriteContainer(): Tried to write unknown filetype!!");
            }
        };
        //If is a pack file, we gotta load everything first then rebuild the file

        //Write stuff
//        if(!container.commit())
//        {
//            //Error during commit!
//            qWarning( container.errorString().toLocal8Bit() );
//            QMessageBox msgBox;
//            msgBox.setText("Failed to commit to file!");
//            msgBox.setInformativeText(container.errorString());
//            msgBox.setStandardButtons(QMessageBox::Ok);
//            msgBox.setDefaultButton(QMessageBox::Ok);
//            msgBox.exec();
//            return;
//        }
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


};
