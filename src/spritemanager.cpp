#include "spritemanager.hpp"
#include <src/extfmt/riff_palette.hpp>
#include <src/extfmt/text_palette.hpp>
#include <src/extfmt/gpl_palette.hpp>
#include <src/ppmdu/utils/gfileio.hpp>
#include <QSaveFile>


namespace spr_manager
{

    const QVector<QString> PaletteFileFilter
    {
        "Microsoft RIFF palette (*.pal)",
        "Text file hex color list (*.txt)",
        "GIMP GPL palette (*.gpl)",
        "Palette from a PNG image (*.png)",
    };

    SpriteContainer * SpriteManager::OpenContainer(const QString &fname)
    {
        m_container.reset(new SpriteContainer(fname));
        m_container->LoadContainer();
        return m_container.data();
    }

    void SpriteManager::CloseContainer()
    {
        qInfo("SpriteManager::CloseContainer(): Close container called!");
        Reset();
    }

    int SpriteManager::SaveContainer()
    {
        if(!IsContainerLoaded())
            throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
        return SaveContainer(m_container->GetContainerSrcPath());
    }

    int SpriteManager::SaveContainer(const QString &fname)
    {
        if(!IsContainerLoaded())
            throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
        m_container->SetContainerSrcPath(fname);
        return m_container->WriteContainer();
    }

    void SpriteManager::ExportContainer(const QString &fname)
    {
        if(!IsContainerLoaded())
            throw ExNoContainer("SpriteManager::ExportContainer(): No container loaded!");
        m_container->ExportContainer(fname);
    }

    SpriteContainer * SpriteManager::ImportContainer(const QString &fname)
    {
        m_container.reset(new SpriteContainer(fname));
        m_container->ImportContainer(fname);
        return m_container.data();
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
            return m_container->nodeChildCount();
        else
            return 0;
    }

    QString SpriteManager::GetContainerSrcFile() const
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
            spr.setParentNode(m_container.data());
            m_container->GetSprite(id) = qMove(spr);
        }
        else
            qCritical() << "SpriteManager::AddSpriteToContainer() : Tried to add a sprite to a single sprite!!\n";

    }

    void SpriteManager::RemSpriteFromContainer(QModelIndex index)
    {
        if(!m_container)
        {
            qWarning("No container loaded!\n");
            return;
        }
        m_container->removeRows( index.row(), 1, QModelIndex(), this );
    }

    bool SpriteManager::DumpSprite(const QModelIndex &index, const QString & path)
    {
        if(IsContainerLoaded())
        {
            Sprite * spr = static_cast<Sprite*>(m_container->getItem(index));
            if(!spr)
            {
                Q_ASSERT(false);
                qWarning("SpriteManager::DumpSprite(): Index isn't a sprite! \n");
                return false;
            }

            qDebug() <<"SpriteManager::DumpSprite(): Dumping sprite #" <<index.row() <<"!\n";
            spr->CommitSpriteData();
            QSaveFile save(path);
            save.write( (char*)(spr->getRawData().data()), spr->getRawData().size());
            return save.commit();
        }
        else
            qCritical() << "SpriteManager::DumpSprite() : Tried to dump a sprite while the container was not loaded!\n";
        return false;
    }

    void SpriteManager::DumpPalette(const Sprite * spr, const QString &path, ePaletteDumpType type)
    {
        if(!spr)
            throw std::range_error("SpriteManager::DumpPalette(): Invalid sprite index!!");

        std::vector<uint8_t> fdata;
        std::vector<uint32_t> pal;
        for(const QRgb & col : spr->getPalette())
        {
            pal.push_back(static_cast<uint32_t>(col));
        }

        switch(type)
        {
        case ePaletteDumpType::RIFF_Pal:
            {
                qDebug("Exporting RIFF Palette\n");
                fdata = utils::ExportTo_RIFF_Palette(pal, utils::ARGBToComponents); //sice QRgb is ARGB, we use this decoder!
                qDebug("Exporting RIFF Palette, conversion complete!\n");
                break;
            }
        case ePaletteDumpType::TEXT_Pal:
            {
                fdata = utils::ExportPaletteAsTextPalette(pal, utils::ARGBToComponents);
                break;
            }
        case ePaletteDumpType::GIMP_PAL:
            {
                fdata = utils::ExportGimpPalette(pal, utils::ARGBToComponents);
                break;
            }
        default:
            throw std::invalid_argument("SpriteManager::DumpPalette(): Invalid palette destination type!");
        };

        qDebug("Exporting RIFF Palette, Writing to file!\n");
        QSaveFile sf(path);
        if(!sf.open( QSaveFile::WriteOnly ))
            throw std::runtime_error(QString("SpriteManager::DumpPalette(): Couldn't open file \"%1\" for writing!\n").arg(path).toStdString());

        if( sf.write( (char*)fdata.data(), fdata.size() ) < static_cast<qint64>(fdata.size()) )
            qWarning("SpriteManager::DumpPalette(): The amount of bytes written to file differs from the expected filesize!\n");

        qDebug("Exporting RIFF Palette, written! Now commiting\n");

        if(!sf.commit())
            throw std::runtime_error(QString("SpriteManager::DumpPalette(): Commit to \"%1\" failed!\n").arg(path).toStdString());
        qDebug("Exporting RIFF Palette, commited!\n");
    }

    void SpriteManager::DumpPalette(const QModelIndex &sprite, const QString &path, ePaletteDumpType type)
    {
        if(!sprite.isValid())
            throw std::invalid_argument("SpriteManager::DumpPalette(): Invalid sprite index!!");

        Sprite * spr = static_cast<Sprite*>(m_container->getItem(sprite));
        DumpPalette(spr, path, type);
    }

    void SpriteManager::ImportPalette(Sprite *spr, const QString &path, ePaletteDumpType type)
    {
        if(!spr)
            throw std::range_error("SpriteManager::ImportPalette(): Invalid sprite index!!");

        std::vector<uint8_t> fdata = utils::ReadFileToByteVector(path.toStdString());

        switch(type)
        {
        case ePaletteDumpType::RIFF_Pal:
            {
                //can't use move here since we need the implicit convertion on copy
                std::vector<uint32_t> imported = utils::ImportFrom_RIFF_Palette(fdata, utils::RGBToARGB);
                spr->setPalette(QVector<QRgb>(imported.begin(), imported.end())); //since QRgb is ARGB, we use this encoder!
                break;
            }
        case ePaletteDumpType::TEXT_Pal:
            {
                //can't use move here since we need the implicit convertion on copy
                std::vector<uint32_t> imported = utils::ImportPaletteAsTextPalette(fdata, utils::RGBToARGB);
                spr->setPalette( QVector<QRgb>(imported.begin(), imported.end()) );
                break;
            }
        case ePaletteDumpType::GIMP_PAL:
            {
                //can't use move here since we need the implicit convertion on copy
                std::vector<uint32_t> imported = utils::ImportGimpPalette(fdata, utils::RGBToARGB);
                spr->setPalette( QVector<QRgb>(imported.begin(), imported.end()) );
                break;
            }
        case ePaletteDumpType::PNG_PAL:
            {
                //can't use move here since we need the implicit convertion on copy
                QImage png(path, "png");
                spr->setPalette(png.colorTable());
                break;
            }
        default:
            throw std::invalid_argument("SpriteManager::ImportPalette(): Invalid palette type!");
        };

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
                return tr("PACK");
            case SpriteContainer::eContainerType::WAN:
                return tr("WAN");
            case SpriteContainer::eContainerType::WAT:
                return tr("WAT");
            default:
                return QString("NULL");
            };
        }
        return tr("NONE");
    }

    //Incremental load
    void SpriteManager::fetchMore(const QModelIndex &parent)
    {
        if(m_container)
            m_container->fetchMore(parent);
    }

    bool SpriteManager::canFetchMore(const QModelIndex &parent) const
    {
        if(m_container)
            return m_container->canFetchMore(parent);
        return false;
    }

};


