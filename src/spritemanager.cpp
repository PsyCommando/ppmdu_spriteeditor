#include "spritemanager.h"


namespace spr_manager
{
    const QString ShortNameNone     = "NONE";
    const QString ShortNamePackFile = "PACK";
    const QString ShortNameWAN      = "WAN";
    const QString ShortNameWAT      = "WAT";

    SpriteContainer * SpriteManager::OpenContainer(const QString &fname)
    {
        m_container.reset(new SpriteContainer(fname));
        m_container->LoadContainer();
        return m_container.data();
    }

    void SpriteManager::CloseContainer()
    {
        Reset();
    }

    void SpriteManager::SaveContainer()
    {
        if(!m_container)
        {
            qWarning("No container loaded!");
            return;
        }
        m_container->WriteContainer();
    }

    void SpriteManager::SaveContainer(const QString &fname)
    {
        if(!m_container)
        {
            qWarning("No container loaded!");
            return;
        }
        m_container->SetContainerSrcPath(fname);
        m_container->WriteContainer();
    }

    void SpriteManager::ExportContainer(const QString &fname)
    {
        if(!m_container)
        {
            qWarning("No container loaded!");
            return;
        }
    }

    void SpriteManager::ImportContainer(const QString &fname)
    {
        Reset();
    }

    SpriteContainer * SpriteManager::NewContainer(SpriteContainer::eContainerType type)
    {
        m_container.reset(new SpriteContainer());
        m_container->SetContainerType(type);

        if(m_container->ContainerIsSingleSprite())
        {
            //We want to add a single empty sprite for a single sprite!
            m_container->AddSprite();
        }
        return m_container.data();
    }

    int SpriteManager::GetNbSpritesInContainer() const
    {
        if(m_container)
            return m_container->childCount();
        else
            return 0;
    }

    const QString &SpriteManager::GetContainerSrcFile() const
    {
        if(m_container)
            return m_container->GetContainerSrcPath();
        else
            return QString();
    }

    SpriteContainer::eContainerType SpriteManager::GetType() const
    {
        if(!m_container)
            return SpriteContainer::eContainerType::NONE;
        return m_container->GetContainerType();
    }

    void SpriteManager::Reset()
    {
        m_container.reset();
    }

    void SpriteManager::AddSpriteToContainer(Sprite &&spr)
    {
        if( ContainerIsPackFile() && m_container )
        {
            SpriteContainer::sprid_t id = m_container->AddSprite();
            qDebug() << "SpriteManager::AddSpriteToContainer(): Adding new sprite to slot#" <<id <<"!\n";
            spr.setParent(m_container.data());
            m_container->GetSprite(id) = qMove(spr);
        }
        else
            qCritical() << "SpriteManager::AddSpriteToContainer() : Tried to add a sprite to a single sprite!!\n";

    }

    void SpriteManager::RemSpriteFromContainer(QModelIndex index)
    {
        if(!m_container)
        {
            qWarning("No container loaded!");
            return;
        }
    }

    bool SpriteManager::ContainerIsPackFile()const
    {
        if(m_container)
            return m_container->ContainerIsPackFile();
        return false;
    }

    bool SpriteManager::ContainerIsSingleSprite()const
    {
        if(m_container)
            return m_container->ContainerIsSingleSprite();
        return false;
    }

    QString SpriteManager::getContentShortName()const
    {
        if(m_container)
        {
            switch( m_container->GetContainerType() )
            {
            case SpriteContainer::eContainerType::PACK:
                return ShortNamePackFile;
            case SpriteContainer::eContainerType::WAN:
                return ShortNameWAN;
            case SpriteContainer::eContainerType::WAT:
                return ShortNameWAT;
            };
        }
        return ShortNameNone;
    }

};
