#ifndef SPRITEPROPERTIESHANDLER_HPP
#define SPRITEPROPERTIESHANDLER_HPP
#include <QPointer>
#include <src/data/sprite/models/sprite_props_model.hpp>

//*******************************************************************
//  SpritePropertiesHandler
//*******************************************************************
//Links the Sprite's properties and its Model and Delegate!
class SpritePropertiesDelegate;
class SpritePropertiesHandler : public QObject
{
    Q_OBJECT
    Sprite * m_powner{nullptr};
    QScopedPointer<SpritePropertiesDelegate> m_pDelegate; //QPointer because the hierachy handles deleting those!
    QScopedPointer<SpritePropertiesModel>    m_pModel;

public:
    SpritePropertiesHandler( Sprite * owner, QObject * parent = nullptr );

    virtual ~SpritePropertiesHandler();

    void setOwner( Sprite * own );

    SpritePropertiesDelegate * delegate();
    SpritePropertiesModel    * model();

public slots:
    void setSpriteType( fmt::eSpriteType ty );

signals:

};

#endif // SPRITEPROPERTIESHANDLER_HPP
