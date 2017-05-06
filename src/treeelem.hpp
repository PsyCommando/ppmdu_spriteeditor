#ifndef TREEELEM_HPP
#define TREEELEM_HPP


#include <QVariant>
#include <QList>

enum struct eTreeElemType
{
    Editable,
    Fixed,
};

enum struct eTreeElemDataType
{
    None,
    sprite,
    effectOffsets,
    palette,
    images,
    image,
    frames,
    frame,
    animSequences,
    animSequence,
    animTable,
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
        m_elemty     = eTreeElemType::Fixed;
        m_dataty     = eTreeElemDataType::None;
        m_parentItem = parentItem;
    }

    explicit TreeElement(const TreeElement & cp)
    {
        m_elemty     = cp.m_elemty;
        m_dataty     = cp.m_dataty;
        m_parentItem = cp.m_parentItem;
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
    eTreeElemDataType       getDataTy()const { return m_dataty; }
    void                    setDataTy( eTreeElemDataType ty ) { m_dataty = ty; }

    virtual TreeElement *   parent() { return m_parentItem; }

    virtual void OnClicked(){}

    TreeElement         *m_parentItem;
    eTreeElemType        m_elemty;
    eTreeElemDataType    m_dataty;
};


#endif // TREEELEM_HPP
