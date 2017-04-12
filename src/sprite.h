#ifndef SPRITE_H
#define SPRITE_H
#include <QByteArray>
#include <QStack>
#include <QHash>
#include <src/treeelem.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

extern const char * ElemName_EffectOffset ;
extern const char * ElemName_Palette      ;
extern const char * ElemName_Images       ;
extern const char * ElemName_Image        ;
extern const char * ElemName_FrameCnt     ;
extern const char * ElemName_Frame        ;
extern const char * ElemName_AnimSequence ;
extern const char * ElemName_AnimSequences;
extern const char * ElemName_AnimTable    ;

//============================================================================================
//
//============================================================================================
/*
 * BaseTreeTerminalChild
 *  Base class for implementing terminal tree nodes elements!
*/
template<const char** _STRELEMNAME>
    class BaseTreeTerminalChild : public TreeElement
{
protected:
    constexpr QString ElemName()
    {
        return QString(*_STRELEMNAME);
    }

public:
    BaseTreeTerminalChild(TreeElement * parent)
        :TreeElement(parent)
    {}

    virtual ~BaseTreeTerminalChild() {}


    TreeElement *child(int) override    {return nullptr;}
    int childCount() const override         {return 0;}
    int childNumber() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOf(const_cast<BaseTreeTerminalChild*>(this));

        return 0;
    }

    int indexOf( TreeElement* )const override  {return 0;}

    //Thos can be re-implemented!
    virtual int columnCount() const                 {return 1;}

    virtual QVariant data(int column) const
    {
        if( column != 0 )
            return QVariant();
        QString sprname = ElemName() + "#" + QString(childNumber());
        return QVariant(sprname);
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

};


/*
 * BaseListContainerChild
*/
    template<const char** _STRELEMNAME, class _CHILD_TY>
        class BaseListContainerChild : public TreeElement
    {
    protected:
        constexpr QString ElemName()
        {
            return QString(*_STRELEMNAME);
        }
        typedef _CHILD_TY               child_t;
        typedef QList<child_t> container_t;

    public:
        BaseListContainerChild(TreeElement * parent)
            :TreeElement(parent)
        {}

        virtual ~BaseListContainerChild() {}


        TreeElement *child(int row) override    {return &m_container[row];}
        int childCount() const override         {return m_container.size();}
        int childNumber() const override
        {
            if (m_parentItem)
                return m_parentItem->indexOf(const_cast<BaseListContainerChild*>(this));

            return 0;
        }

        int indexOf( TreeElement * ptr )const override
        {
            child_t * ptras = static_cast<child_t *>(ptr);
            //Search a matching child in the list!
            if( ptras )
                return m_container.indexOf(*ptras);
            return 0;
        }

        //Thos can be re-implemented!
        virtual int columnCount() const                 {return 1;}

        virtual QVariant data(int column) const
        {
            if( column != 0 )
                return QVariant();
            QString sprname = ElemName() + "#" + QString(childNumber());
            return QVariant(sprname);
        }

        bool insertChildren(int position, int count) override
        {
            int i = 0;
            for( ; i < count; ++i )
                m_container.insert(position, child_t(this) );
            return true;
        }

        bool removeChildren(int position, int count) override
        {
            if( (position + count) >= m_container.size() )
                return false;

            int i = 0;
            for( ; i < count; ++i )
                m_container.removeAt(position);
            return true;
        }

    protected:
        container_t m_container;

    };

//============================================================================================
//
//============================================================================================

//
//
//
class EffectOffsetContainer : public BaseTreeTerminalChild<&ElemName_EffectOffset>
{
public:

    EffectOffsetContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }
};

//
//
//
class PaletteContainer : public BaseTreeTerminalChild<&ElemName_Palette>
{
public:

    PaletteContainer( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }
};

//
//
//

class Image : public BaseTreeTerminalChild<&ElemName_Image>, public utils::BaseSequentialIDGen<Image>
{
public:
    Image(TreeElement * parent)
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {
    }

    inline bool operator==( const Image & other)const  {return getID() == other.getID();}
    inline bool operator!=( const Image & other)const  {return !operator==(other);}

};

class ImageContainer : public BaseListContainerChild<&ElemName_Images, Image>
{
public:

    ImageContainer( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

};

//
//
//
class MFrame : public BaseTreeTerminalChild<&ElemName_Frame>, public utils::BaseSequentialIDGen<MFrame>
{
public:
    MFrame( TreeElement * parent )
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {}

    inline bool operator==( const MFrame & other)const  {return getID() == other.getID();}
    inline bool operator!=( const MFrame & other)const  {return !operator==(other);}

private:
};


//
//
//

class FramesContainer : public BaseListContainerChild<&ElemName_FrameCnt, MFrame>
{
public:

    FramesContainer( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

private:
};

//
//
//

class AnimSequence : public BaseTreeTerminalChild<&ElemName_AnimSequence>, public utils::BaseSequentialIDGen<AnimSequence>
{
public:
    AnimSequence( TreeElement * parent )
        :BaseTreeTerminalChild(parent),BaseSequentialIDGen()
    {}

    inline bool operator==( const AnimSequence & other)const  {return getID() == other.getID();}
    inline bool operator!=( const AnimSequence & other)const  {return !operator==(other);}
};

//
//
//
class AnimSequences : public BaseListContainerChild<&ElemName_AnimSequences, AnimSequence>
{
public:
    AnimSequences( TreeElement * parent )
        :BaseListContainerChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }

private:
};

//
//
//
class AnimTable : public BaseTreeTerminalChild<&ElemName_AnimTable>
{
public:

    AnimTable( TreeElement * parent )
        :BaseTreeTerminalChild(parent)
    {}

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        return QVariant(ElemName());
    }
};

//============================================================================================
//
//============================================================================================

//
//
//
class Sprite : public TreeElement, public utils::BaseSequentialIDGen<Sprite>
{
    //static QHash<unsigned long long,Sprite> spriteIDs;
//    static unsigned long long               spriteCnt;
//    static QStack<unsigned long long>       spriteIDRecycler;
//    unsigned long long                      m_id;

//    static void AddSprite( Sprite * spr )
//    {
//        unsigned long long id = spriteCnt;
//        if( spriteIDRecycler.empty() )
//            ++spriteCnt;
//        else
//        {
//            id = spriteIDRecycler.front();
//            spriteIDRecycler.pop_front();
//        }
//        //spriteIDs.insert(id,*spr);
//        spr->m_id = id;
//    }

//    static void RemSprite(Sprite * spr)
//    {
//        spriteIDRecycler.push_back(spr->m_id);
//    }

public:
    Sprite( TreeElement * parent )
        :TreeElement(parent),
          BaseSequentialIDGen(),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)
    {
        //AddSprite(this);
        InitElemTypes();
    }

    Sprite( TreeElement * parent, QByteArray && raw )
        :TreeElement(parent),
          BaseSequentialIDGen(),
          m_raw(raw),
          m_efxcnt(this),
          m_palcnt(this),
          m_imgcnt(this),
          m_frmcnt(this),
          m_seqcnt(this),
          m_anmtbl(this)
    {
        //AddSprite(this);
        InitElemTypes();
    }

    ~Sprite()
    {
    }

    void InitElemTypes()
    {
        m_efxcnt.setElemTy(eTreeElemType::Fixed);
        m_palcnt.setElemTy(eTreeElemType::Fixed);
        m_imgcnt.setElemTy(eTreeElemType::Fixed);
        m_frmcnt.setElemTy(eTreeElemType::Fixed);
        m_seqcnt.setElemTy(eTreeElemType::Fixed);
        m_anmtbl.setElemTy(eTreeElemType::Fixed);
    }

    //Raw data buffer
    QByteArray m_raw;

public:

    TreeElement *child(int row) override
    {
        return ElemPtr(row);
    }

    int childCount() const override
    {
        return NBChilds;
    }

    int childNumber() const override
    {
        if (m_parentItem)
            return m_parentItem->indexOf(const_cast<Sprite*>(this));

        return 0;
    }

    int indexOf( TreeElement * ptr )const override
    {
        //Search a matching child in the list!
        for( int idx = 1; idx <= NBChilds; ++idx )
        {
            if(ElemPtr(idx) == ptr)
                return idx;
        }
        return 0;
    }

    int columnCount() const override
    {
        return 1; //Always just 1 column
    }

    TreeElement *parent() override
    {
        return m_parentItem;
    }

    QVariant data(int column) const override
    {
        if( column != 0 )
            return QVariant();
        QString sprname = "Sprite#" + QString(childNumber());
        return QVariant(sprname);
    }

    //You don't!!
    bool insertChildren(int, int) override {return false;}
    bool removeChildren(int, int) override {return false;}

    inline bool operator==( const Sprite & other)const  {return getID() == other.getID();}
    inline bool operator!=( const Sprite & other)const  {return !operator==(other);}

private:

    TreeElement * ElemPtr( int idx )
    {
        switch(idx)
        {
        case 1:
            return &m_efxcnt;
        case 2:
            return &m_palcnt;
        case 3:
            return &m_imgcnt;
        case 4:
            return &m_frmcnt;
        case 5:
            return &m_seqcnt;
        case 6:
            return &m_anmtbl;
        };
        return nullptr;
    }

    const TreeElement * ElemPtr( int idx )const
    {
        return const_cast<Sprite*>(this)->ElemPtr(idx);
    }

    static const int        NBChilds = 6;
    EffectOffsetContainer   m_efxcnt;
    PaletteContainer        m_palcnt;
    ImageContainer          m_imgcnt;
    FramesContainer         m_frmcnt;
    AnimSequences           m_seqcnt;
    AnimTable               m_anmtbl;
};

#endif // SPRITE_H
