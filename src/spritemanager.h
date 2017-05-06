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
#include <QAbstractItemModel>
#include <src/treeelem.hpp>

namespace spr_manager
{




    /*
     * SpriteManager
    */

    class SpriteManager : public QAbstractItemModel
    {
    public:
        Qt::ItemFlags flags(const QModelIndex &index  = QModelIndex()) const
        {
            if (!index.isValid() || !IsContainerLoaded())
                return 0;

            switch(getItem(index)->getElemTy())
            {
            case eTreeElemType::Editable:
                return Qt::ItemIsSelectable | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
            case eTreeElemType::Fixed:
                return Qt::ItemIsSelectable | QAbstractItemModel::flags(index);
            };

            return QAbstractItemModel::flags(index);
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
            return parentItem->childCount();
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

        //Returns a short string to indicate in the status bar the kind of container working on!
        QString getContentShortName()const;

    public:
        using QAbstractItemModel::createIndex;
        using QAbstractItemModel::beginRemoveRows;
        using QAbstractItemModel::beginInsertRows;
        using QAbstractItemModel::endRemoveRows;
        using QAbstractItemModel::endInsertRows;

    public:

        SpriteManager()
            :QAbstractItemModel(nullptr)//, TreeElement(nullptr)
        {}

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
        void OpenContainer(const QString & fname);
        void CloseContainer();

        void SaveContainer();
        void SaveContainer(const QString & fname);

        void ExportContainer(const QString & fname);
        void ImportContainer(const QString & fname);

        void NewContainer( SpriteContainer::eContainerType type );

        //
        //Empty the sprite list and etc.
        void Reset();

        //Access
        int             GetNbSpritesInContainer()const;
        const QString & GetContainerSrcFile()const;

        SpriteContainer::eContainerType GetType()const;

    private:
        QScopedPointer<SpriteContainer> m_container;

    };



};

#endif // SPRITEMANAGER_H
