#ifndef SPRITEPROPERTIESMODEL_HPP
#define SPRITEPROPERTIESMODEL_HPP
#include <QAbstractItemModel>
#include <src/ppmdu/fmts/wa_sprite.hpp>

enum struct eSpriteProperties : int
{
    SpriteType = 0,
    Compression,

    ColorMode,
    Unk7,
    Unk8,
    Unk9,
    Unk10,
    Unk11,
    Unk12,
    Unk13,

    NbProperties [[maybe_unused]],
};
enum struct eSpritePropColumns : int
{
    Value= 0,
    Description,
    NbColumns [[maybe_unused]],
};

extern const QString     ElemName_SpriteProperty;
extern const int         SpritePropertiesNbCols;
extern const QStringList SpritePropertiesColNames;
extern const QStringList SpritePropertiesNames;
extern const QStringList SpritePropertiesDescriptions;
extern const int         SpriteNbProperties;

//*******************************************************************
//  SpritePropertiesModel
//*******************************************************************
//Model for displaying the editable sprite properties on the property tab
class Sprite;
class SpritePropertiesModel : public QAbstractItemModel
{
    Q_OBJECT
    Sprite * m_spr{nullptr};
public:

    SpritePropertiesModel(Sprite * parentspr, QObject * parent = nullptr);
    virtual ~SpritePropertiesModel();

    int getNbProperties()const;

    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const override;
    virtual QModelIndex     index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex     parent(const QModelIndex &/*child*/) const override;

    virtual int     rowCount(const QModelIndex &parent) const override;
    virtual int     columnCount(const QModelIndex &/*parent*/) const override;
    virtual bool    hasChildren(const QModelIndex &parent) const override;

    virtual QVariant    data(const QModelIndex &index, int role) const override;
    virtual bool        setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant    headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QVariant dataDisplay(int propid, int column)const;
    QVariant getNameForProperty(int propid)const;
    QVariant getDataForProperty(int propid, int role)const;
    QVariant getDescForProperty(int propid)const;
    void     setDataForProperty( eSpriteProperties propid, const QVariant & data );

signals:
    void spriteTypeSet(fmt::eSpriteType);
};

#endif // SPRITEPROPERTIESMODEL_HPP
