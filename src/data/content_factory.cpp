#include "content_factory.hpp"
#include <QDebug>

ContentContainerFactory::ContentContainerFactory()
{
}

ContentContainerFactory & ContentContainerFactory::instance()
{
    static ContentContainerFactory _instance;
    return _instance;
}

ContentContainerFactory::~ContentContainerFactory()
{
}

BaseContainer* ContentContainerFactory::MakeContainer(const QString & filepath)
{
    for(const auto & entry : m_registeredtypes)
    {
        if(entry.second.cntFileTypePredicate(filepath))
            return MakeContainerFromType(entry.first);
    }
    return nullptr;
}

BaseContainer* ContentContainerFactory::MakeContainerFromType(const QString & containertype)
{
    auto func = m_registeredtypes.at(containertype).cntInstantiator;
    if(!func)
    {
        throw std::runtime_error("ContentContainerFactory::MakeContainerFromType(): Bad instantiator function!");
    }
    return func();
}

void ContentContainerFactory::RegisterContainerType(ContainerType &&newtype)
{
    qDebug()<<"ContentContainerFactory::RegisterContainerType(): Registered container type \"" <<newtype.cntTypeName <<"\"!";
    m_registeredtypes.insert_or_assign(newtype.cntTypeName, newtype);
}

const ContentContainerFactory::regtypeslst_t &ContentContainerFactory::RegisteredContainerTypes() const
{
    return m_registeredtypes;
}


