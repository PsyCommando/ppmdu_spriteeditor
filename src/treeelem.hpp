#ifndef TREEELEM_HPP
#define TREEELEM_HPP


#include <QVariant>
#include <QList>

enum struct eTreeElemType
{
    Editable,
    Fixed,
};

/*
 * TreeElement
 * Helper base class for displaying the data content of a sprite!
*/
class TreeElement
{
public:
    explicit TreeElement(TreeElement *parentItem = 0)
    {
        m_elemty     = eTreeElemType::Editable;
        m_parentItem = parentItem;
    }

    virtual ~TreeElement()
    {
    }

    virtual TreeElement *   child(int row)=0;
    virtual int             childCount() const=0;
    virtual int             childNumber() const=0;
    virtual int             columnCount() const=0;
    virtual QVariant        data(int column) const = 0;
    virtual bool            insertChildren(int position, int count)=0;
    virtual bool            removeChildren(int position, int count)=0;
    virtual int             indexOf( TreeElement * ptr )const = 0;

    eTreeElemType           getElemTy()const { return m_elemty; }
    void                    setElemTy( eTreeElemType ty ) { m_elemty = ty; }
    virtual TreeElement *   parent() { return m_parentItem; }

    TreeElement         *m_parentItem;
    eTreeElemType        m_elemty;
};


#endif // TREEELEM_HPP
