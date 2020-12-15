#ifndef BASE_CONTAINER_HPP
#define BASE_CONTAINER_HPP
#include <src/data/treenode.hpp>
/*
 * BaseContainer
 * Base class for containers to be displayed by the SpriteManager model.
 */
class BaseContainer : public QObject, public TreeNode
{
    Q_OBJECT
public:
    BaseContainer(QObject * parent = nullptr):QObject(parent),TreeNode(){}
    BaseContainer(const BaseContainer & cp) :QObject(parent()), TreeNode(cp){}
    BaseContainer(BaseContainer && mv)      :QObject(parent()), TreeNode(mv){}

    BaseContainer& operator=(const BaseContainer & cp)  {TreeNode::operator=(cp); return *this;}
    BaseContainer& operator=(BaseContainer && mv)       {TreeNode::operator=(mv); return *this;}
    virtual ~BaseContainer(){}

    //***************************
    //Container State Access
    //***************************
public:
    //Loading/Saving, Importing from another filetype/exporting to another filetype
    virtual void    LoadContainer()                             = 0;
    virtual void    LoadContainer(const QString & path)         {SetContainerSrcPath(path); LoadContainer();}
    virtual int     WriteContainer()const                       = 0;
    virtual int     WriteContainer(const QString & path)        {SetContainerSrcPath(path); return WriteContainer();}

    //Import/Export
    virtual void    ImportContainer(const QString & path)       = 0;
    virtual void    ExportContainer(const QString & path, const QString & exportype)const= 0;
    virtual bool    isContainerLoaded()const                    = 0;

    //Origin/destination file associated with the container for save/loading from/to disk
    virtual QString         GetContainerSrcFnameOnly()const         = 0;
    virtual const QString & GetContainerSrcPath()const              = 0;
    virtual void            SetContainerSrcPath(const QString& path)= 0;

    //Returns a unique string to identify the type of container this is
    virtual const QString& GetContainerType()const = 0;
    virtual void SetContainerType(const QString & newtype) = 0;

    //Returns the type of item the container containers (Must be specialized!! The Macro DEFINE_CONTAINER does it for you)
    template<class CONTAINER_TYPE>
        static const QString& GetContentName()
    {
        static const QString NULLNAME = "NULL";
        return NULLNAME;
    }

    //Returns a clean user displayable content type name
    template<class CONTAINER_TYPE>
        static const QString& GetContentDisplayName()
    {
        static const QString NULLNAME = "NULL";
        return NULLNAME;
    }

    //If a given file match the specified file (Must be specialized!! The Macro DEFINE_CONTAINER does it for you)
    template<class CONTAINER_TYPE>
        static bool DoesFileMatchContainer(const QString & filepath)
    {
        Q_UNUSED(filepath);
        return false;
    }

    //Returns whether the container can contain several top level nodes, or a single one
    virtual bool isMultiItemContainer()const = 0;

    //Returns the name of the topmost node in the container, like sprite for example
    virtual const QString& GetTopNodeName()const = 0;

    //Initialize the container so its ready for its first use.
    virtual void Initialize() = 0;

    //Returns the top level node owning what is referred by the index
    virtual TreeNode*         getOwnerNode(const QModelIndex & index) = 0;
    virtual const TreeNode*   getOwnerNode(const QModelIndex & index)const = 0;

    //***************************
    //Container Model Interface
    //***************************
public:
    virtual int GetNbDataColumns()const = 0;
    virtual int GetNbDataRows()const {return nodeChildCount();}
    virtual QVariant GetContentData(const QModelIndex & index, int role)const = 0;
    virtual QVariant GetContentHeaderData(int section, Qt::Orientation orientation, int role)const = 0;
    virtual bool canFetchMore(const QModelIndex & index)const = 0;
    virtual void fetchMore(const QModelIndex & index) = 0;
};
#endif // BASE_CONTAINER_HPP
