#ifndef SPRITECONTAINER_H
#define SPRITECONTAINER_H
#include <QVector>
#include <QMap>
#include <QFile>
#include <QDataStream>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QList>
#include <QDebug>
#include <cstdint>
#include <QtConcurrent/QtConcurrent>
#include "sprite.h"
#include <src/treeelem.hpp>
/*
*/

namespace spr_manager
{

    /*
     *  SpriteContainer
    */
    class SpriteContainer : public QObject, public TreeElement
    {
        Q_OBJECT
    public:
        typedef QList<Sprite>::iterator         iterator;
        typedef QList<Sprite>::const_iterator   const_iterator;
        typedef uint32_t sprid_t;
        enum struct eContainerType
        {
            NONE,
            WAN,
            WAT,
            PACK,
        };

        static const QList<QString> SpriteContentCategories;

        //
        SpriteContainer(QObject * parent = nullptr);
        SpriteContainer(const QString & str, QObject * parent = nullptr);
        SpriteContainer(const SpriteContainer&) = delete;
        SpriteContainer(SpriteContainer&&) = delete;
        SpriteContainer & operator=(const SpriteContainer&) = delete;
        SpriteContainer & operator=(SpriteContainer&&) = delete;
        virtual ~SpriteContainer();
        void clone(const TreeElement */*other*/);

        //
        bool ContainerLoaded()const;
        bool ContainerIsPackFile()const;
        bool ContainerIsSingleSprite()const;

        inline void           SetContainerType(eContainerType ty){m_cntTy = ty;}
        inline eContainerType GetContainerType()const           {return m_cntTy;}

        inline const QString & GetContainerSrcPath()const             {return m_srcpath;}
        inline void SetContainerSrcPath(const QString & path)   { m_srcpath = path; }

        //
        void LoadContainer();
        int WriteContainer();

        //
        void ImportContainer(const QString & path);
        void ExportContainer(const QString & path)const;

        //
        Sprite & GetSprite( sprid_t idx );
        sprid_t  AddSprite();

        inline iterator         begin()     {return m_spr.begin();}
        inline const_iterator   begin()const{return m_spr.begin();}
        inline iterator         end()       {return m_spr.end();}
        inline const_iterator   end()const  {return m_spr.end();}
        //
    public:

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        QVariant headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role */= Qt::DisplayRole) const;

        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

        TreeElement *getItem(const QModelIndex &index);


        template<class _MANAGERTY>
            QModelIndex index(int row, int column, const QModelIndex &parent, const _MANAGERTY * manager )
        {
            if ( column != 0)
                return QModelIndex();

            TreeElement *parentItem = getItem(parent);
            TreeElement *childItem  = parentItem->nodeChild(row);
            if (childItem)
                return manager->createIndex(row, column, childItem);
            else
                return QModelIndex();
        }

        template<class _MANAGERTY>
            QModelIndex parent(const QModelIndex &index, const _MANAGERTY * manager)
        {
            if (!index.isValid() )
                return QModelIndex();

            if( index.internalPointer() == this || index.internalPointer() == manager )
                return QModelIndex();

            if(m_spr.empty())
                return QModelIndex();

            TreeElement *childItem = getItem(index);
            TreeElement *parentItem = childItem->parentNode();
            Q_ASSERT(parentItem != nullptr);

            if (parentItem == this)
                return QModelIndex();

            return manager->createIndex(parentItem->nodeIndex(), 0, parentItem);
        }


        int columnCount(const QModelIndex &/*parent */= QModelIndex()) const;

        template<class _MANAGERTY>
            bool removeRows(int position, int rows, const QModelIndex &parent, _MANAGERTY * manager)
        {
            TreeElement *parentItem = getItem(parent);
            bool success = true;

            manager->beginRemoveRows(parent, position, position + rows - 1);
            success = parentItem->removeChildrenNodes(position, rows);
            manager->endRemoveRows();

            return success;
        }

        template<class _MANAGERTY>
            bool insertRows(int position, int rows, const QModelIndex &parent, _MANAGERTY * manager)
        {
            TreeElement *parentItem = getItem(parent);
            bool success;

            manager->beginInsertRows(parent, position, position + rows - 1);
            success = parentItem->insertChildrenNodes(position, rows);
            manager->endInsertRows();

            return success;
        }



    public:
        void appendChild(TreeElement *item);
        TreeElement *nodeChild(int row)override;
        int nodeChildCount() const override;
        int nodeIndex() const override;
        int indexOfNode( TreeElement * ptr )const override;
        QModelIndex modelIndexOfNode(TreeElement *ptr) const;
        int nodeColumnCount() const override;
        TreeElement *parentNode() override;
        Sprite *parentSprite()override;
        QVariant nodeData(int column, int role) const override;
        bool insertChildrenNodes(int position, int count)override;
        bool removeChildrenNodes(int position, int count)override;
        bool moveChildrenNodes  (int srcrow, int count, int destrow)override;
        bool nodeIsMutable()const override;

    private:
        void FetchToC( QDataStream & fdat );
        void LoadEntry(sprid_t idx);

        QString getSrcFnameOnly()const;

    private:
        QString                 m_srcpath;      //Original path of the container if applicable!
        QList<Sprite>           m_spr;          //
        eContainerType          m_cntTy;
        //TreeElement           * m_rootelem;     //alias on this!
        QThread                 m_workthread;

    signals:
        void startThread();
        void showProgress(QFuture<void>&);
        void showProgress(QFuture<void>&,QFuture<void>&);
    };

    /*
        ThreadedWriter
            Used so the event loop of the mainwindow can continue looping while we do long operations!
    */
    class ThreadedWriter : public QObject
    {
        Q_OBJECT
    public:
        QScopedPointer<QSaveFile>         savefile;
        SpriteContainer                 * sprdata;
        QMutex                            mtxdata;
        int                               bywritten;
        QFuture<void>                     op1;
        QFuture<void>                     op2;

        explicit ThreadedWriter(QSaveFile * sfile, SpriteContainer * cnt);
        virtual ~ThreadedWriter();

    public slots:
        void WritePack();
        void WriteSprite();
        //void OnFinished();

    signals:
        void finished();
        void finished(int); //int is the nb of bytes written!

    private:
        void _WriteSprite();
        void _WritePack();
    };

};

#endif // SPRITECONTAINER_H
