#include "spritecontainer.h"
#include <QMessageBox>
#include <QString>
#include <cassert>
#include <src/ppmdu/utils/byteutils.hpp>
#include <src/ppmdu/fmts/packfile.hpp>

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

        m_spr.clear();
        //Lets identify the format
        if(m_srcpath.endsWith(PackFileExt))
        {
            fmt::PackFileLoader ldr;
            ldr.Read(data.begin(), data.end());
            m_spr.reserve(ldr.size());

            //Load the raw data into each sprites but don't parse them yet!
            for( size_t cnt = 0; cnt < ldr.size(); ++cnt )
            {
                Sprite newspr(this);
                ldr.CopyEntryData( cnt, std::back_inserter(newspr.m_raw) );
                m_spr.push_back(std::move(newspr));
            }

        }
        else if( m_srcpath.endsWith(WANFileExt) || m_srcpath.endsWith(WATFileExt) )
        {           
            //We load the whole sprite
            m_spr.push_back(Sprite(this));
            std::copy( data.begin(), data.end(), std::back_inserter(m_spr.front().m_raw) );
        }

    }

    void SpriteContainer::WriteContainer()
    {
        //
        QFile container(m_srcpath);

        if( !container.open(QIODevice::ReadWrite) || container.error() != QFileDevice::NoError )
        {
            //Error can't write file!
            qWarning( container.errorString().toLocal8Bit() );
            QMessageBox msgBox;
            msgBox.setText("Failed to write file!");
            msgBox.setInformativeText(container.errorString());
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        if( container.exists())
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
        assert(false);


        //If is a pack file, we gotta load everything first then rebuild the file

        //Write stuff
    }

    Sprite &SpriteContainer::GetSprite(SpriteContainer::sprid_t idx)
    {
        return m_spr[idx];
    }

    SpriteContainer::sprid_t SpriteContainer::AddSprite()
    {
        size_t offset = m_spr.size();
        m_spr.push_back( Sprite(this) );
        return offset;
    }

    void SpriteContainer::FetchToC(QDataStream &fdat)
    {

    }

    void SpriteContainer::LoadEntry(SpriteContainer::sprid_t idx)
    {

    }


};
