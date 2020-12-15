#ifndef CONTENT_FACTORY_HPP
#define CONTENT_FACTORY_HPP
#include <src/data/base_container.hpp>
#include <QString>
#include <unordered_map>
#include <functional>
#include <type_traits>

/*
 * Contains details to instantiate the correct kind of container
*/
struct ContainerType
{
    typedef std::function<BaseContainer*()>     funInstantiator_t;
    typedef std::function<bool(const QString&)> funFileTypePredicate_t;

    QString                 cntTypeName;            //Key to identify this type as
    funInstantiator_t       cntInstantiator;        //Function for creating an instance of the type
    funFileTypePredicate_t  cntFileTypePredicate;   //Function for matching a type to a file indicated by the specified filepath
    QString                 cntDisplayName;         //Clean user-readable name for the type of content
    //**If you add things here, don't forget to update the "ContentTypeInitializer" struct's constructor to init the new vars, and also modify the define macro**
};

/*
 * Handles creating the appropriate container for a given file path/container type
*/
class ContentContainerFactory
{
    ContentContainerFactory();
public:
    typedef std::unordered_map<QString, ContainerType> regtypeslst_t;
    static ContentContainerFactory & instance();
    ~ContentContainerFactory();

    BaseContainer* MakeContainer(const QString & filepath);
    BaseContainer* MakeContainerFromType(const QString & containertype);

    void RegisterContainerType(ContainerType && newtype);
    const regtypeslst_t & RegisteredContainerTypes()const;

private:
    regtypeslst_t m_registeredtypes;
};

/*
 * Helper to instantiate types automatically
*/
template<class CNT_TY> struct ContentTypeInitializer
{
    ContentTypeInitializer()
    {
        ContentContainerFactory::instance().RegisterContainerType(
                    ContainerType
                    {
                        BaseContainer::GetContentName<CNT_TY>(),
                        std::function([](){return new CNT_TY();}),
                        std::function([](const QString & filepath){return BaseContainer::DoesFileMatchContainer<CNT_TY>(filepath);}),
                        BaseContainer::GetContentDisplayName<CNT_TY>(),
                    });
    }
    static ContentTypeInitializer<CNT_TY> s_instance; //When specializing this template, instantiate the constant
};

//Helper macro to define a type to be handled by the system
//TYPE : Type of the container
//TNAME: Name of the items from the container
//FUN_FILEMATCH : Function with the signature "bool(const QString&)" that returns if a given filt path can be opened with the container!
//DNAME: User readable display name to be used for the container type.
#define DEFINE_CONTAINER(TYPE, TNAME, FUN_FILEMATCH, DNAME)\
    template<> ContentTypeInitializer<TYPE> ContentTypeInitializer<TYPE>::s_instance{};\
    template<> const QString& BaseContainer::GetContentName<TYPE>(){static const QString NULLNAME = TNAME; return NULLNAME;}\
    template<> bool BaseContainer::DoesFileMatchContainer<TYPE>(const QString & filepath){return FUN_FILEMATCH(filepath);}\
    template<> const QString& BaseContainer::GetContentDisplayName<TYPE>(){static const QString NULLNAME = DNAME; return NULLNAME;}

#endif // CONTENT_FACTORY_HPP
