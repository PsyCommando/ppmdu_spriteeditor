#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H
/*
*/
#include "spritecontainer.h"
#include <QScopedPointer>
//#include <memory>
#include <cstdint>
#include <qstring.h>
#include <QTreeView>
#include <QDebug>
#include <QAbstractItemModel>
#include <src/treeelem.hpp>

namespace spr_manager
{

    extern const QVector<QString> PaletteFileFilter; //list of file filters matching the ePaletteDumpType values below!
    enum struct ePaletteDumpType : unsigned int
    {
        RIFF_Pal = 0,
        TEXT_Pal,
        GIMP_PAL,
        INVALID,
    };
    inline const QString & GetPaletteFileFilterString( ePaletteDumpType ty )
    {
        static const QString InvalidString;
        if(ty >= ePaletteDumpType::INVALID)
            return InvalidString;
        return PaletteFileFilter[static_cast<int>(ty)];
    }


    /*
     * SpriteManager
    */

    class SpriteManager : public QAbstractItemModel
    {
    public:
        Qt::ItemFlags flags(const QModelIndex &index  = QModelIndex()) const
        {
            if (!index.isValid() || !IsContainerLoaded())
                return QAbstractItemModel::flags(index);

            TreeElement * pnode = getItem(index);
            if(!pnode)
                return QAbstractItemModel::flags(index);

            return pnode->nodeFlags();

//            switch(getItem(index)->getElemTy())
//            {
//            case eTreeElemType::Editable:
//                return Qt::ItemIsSelectable | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
//            case eTreeElemType::Fixed:
//                return Qt::ItemIsSelectable | QAbstractItemModel::flags(index);
//            };

            //return QAbstractItemModel::flags(index);
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            if (!IsContainerLoaded())
                return QVariant();
            return m_container->data(index, role);
        }

        QVariant headerData(int /*section*/, Qt::Orientation /*orientation*/, int role = Qt::DisplayRole) const override
        {
            //nothing really
            return QVariant();
        }

        int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            if (!IsContainerLoaded())
                return 0;

            TreeElement *parentItem = getItem(parent);
            return parentItem->nodeChildCount();
        }

        bool hasChildren(const QModelIndex &parent = QModelIndex()) const override
        {
            if(!IsContainerLoaded())
                return false;
            return m_container->hasChildren(parent);
        }

        TreeElement *getItem(const QModelIndex &index) const
        {
            if (!IsContainerLoaded())
                return nullptr;
            return m_container->getItem(index);
        }

        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {
            if (!IsContainerLoaded())
                return QModelIndex();
            return m_container->index(row,column,parent,this);
        }

        QModelIndex parent(const QModelIndex &index = QModelIndex()) const override
        {
            if (!IsContainerLoaded())
                return QModelIndex();
            return m_container->parent(index, this);
        }


        int columnCount(const QModelIndex &parent = QModelIndex()) const override
        {
            return 1;
        }

        bool removeRows(int position, int rows, const QModelIndex &parent)
        {
            if (!IsContainerLoaded())
                return false;
            return m_container->removeRows(position, rows, parent, this);
        }

        bool insertRows(int position, int rows, const QModelIndex &parent)
        {
            if (!IsContainerLoaded())
                return false;
            return m_container->insertRows(position, rows, parent, this);
        }

    public:
        using QAbstractItemModel::createIndex;
        using QAbstractItemModel::beginRemoveRows;
        using QAbstractItemModel::beginInsertRows;
        using QAbstractItemModel::endRemoveRows;
        using QAbstractItemModel::endInsertRows;

    public:

        SpriteManager()
            :QAbstractItemModel(nullptr), m_container(nullptr)
        {
        }

        ~SpriteManager()
        {
            qDebug("SpriteManager::~SpriteManager(): Deleting sprite manager!\n");
            //Since the class will be deleted later by the model, release ownership here instead of deleting!
//            delete m_container.take();
        }

        static SpriteManager & Instance()
        {
            static SpriteManager s_manager;
            return s_manager;
        }

        //Info
        inline bool IsContainerLoaded()const { return (m_container != nullptr); }
        bool ContainerIsPackFile()const;
        bool ContainerIsSingleSprite()const;

        //IO
        SpriteContainer * OpenContainer(const QString & fname);
        void CloseContainer();

        int SaveContainer();
        int SaveContainer(const QString & fname);

        void ExportContainer(const QString & fname);
        SpriteContainer *ImportContainer(const QString & fname);

        SpriteContainer * NewContainer( SpriteContainer::eContainerType type );


        //
        //Empty the sprite list and etc.
        void Reset();

        void AddSpriteToContainer  ( Sprite && spr );
        void RemSpriteFromContainer( QModelIndex index );

        bool DumpSprite(const QModelIndex &index, const QString &path);
        void DumpPalette(const Sprite * sprite, const QString &path, ePaletteDumpType type);
        void DumpPalette(const QModelIndex & sprite, const QString &path, ePaletteDumpType type);
        void ImportPalette(Sprite * sprite, const QString &path, ePaletteDumpType type);

        //Access
        int             GetNbSpritesInContainer()const;
        QString GetContainerSrcFile()const;

        SpriteContainer::eContainerType GetType()const;

        //Returns a short string to indicate in the status bar the kind of container working on!
        QString getContentShortName()const;

        //Tool for retrieving animation table templates names
        inline QStringList       & getAnimSlotNames() {return m_animslotnames;}
        inline const QStringList & getAnimSlotNames()const {return m_animslotnames;}

        inline SpriteContainer * getContainer() {return m_container.data();}

    private:
        QScopedPointer<SpriteContainer> m_container; //<-- MAKE THIS A QPOINTER And destroy manually or via the item model!!!!!
        QStringList m_animslotnames;

    };



};

#endif // SPRITEMANAGER_H
