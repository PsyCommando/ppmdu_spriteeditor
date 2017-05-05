#include "spritemanager.h"


namespace spr_manager
{

    void SpriteManager::OpenContainer(const QString &fname)
    {
        m_container.reset( new SpriteContainer(fname) );
        m_container->LoadContainer();
    }

    void SpriteManager::CloseContainer()
    {

    }

    void SpriteManager::SaveContainer()
    {

    }

    void SpriteManager::SaveContainer(const QString &fname)
    {

    }

    void SpriteManager::ExportContainer(const QString &fname)
    {

    }

    void SpriteManager::ImportContainer(const QString &fname)
    {

    }

    void SpriteManager::NewContainer(SpriteContainer::eContainerType type)
    {
        m_container.reset( new SpriteContainer() );
        m_container->SetContainerType(type);

        if(m_container->ContainerIsSingleSprite())
        {
            //We want to add a single empty sprite for a single sprite!
            m_container->AddSprite();
        }
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

};
