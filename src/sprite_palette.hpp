#ifndef SPRITE_PALETTE_HPP
#define SPRITE_PALETTE_HPP
#include <QVector>
#include <QList>
#include <QColor>
#include <QImage>
#include <QPixmap>
#include <QGraphicsScene>
#include <QPointer>
#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>

#include <algorithm>

#include <src/treeelem.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>
#include <src/ppmdu/utils/imgutils.hpp>
#include <src/ppmdu/utils/sequentialgenerator.hpp>

class Sprite;
extern const char * ElemName_Palette;

//====================================================================
//  PaletteModel
//====================================================================
class PaletteModel : public QAbstractItemModel
{
    Q_OBJECT
    QVector<QRgb> * m_pal;

 public:
    PaletteModel(QVector<QRgb> * pal, QObject * parent = nullptr);


    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual bool     setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool moveRows(const QModelIndex &sourceParent,
                          int sourceRow,
                          int count,
                          const QModelIndex &destinationParent,
                          int destinationChild) override;

    virtual Qt::ItemFlags flags(const QModelIndex &/*index*/) const override;
};

//*******************************************************************
//  PaletteContainer
//*******************************************************************
class PaletteContainer : public BaseTreeTerminalChild<&ElemName_Palette>
{
public:

    PaletteContainer( TreeElement * parent );
    PaletteContainer( PaletteContainer && mv );
    PaletteContainer( const PaletteContainer & cp );
    PaletteContainer & operator=( const PaletteContainer & cp );
    PaletteContainer & operator=( PaletteContainer && mv );
    ~PaletteContainer();

    void clone(const TreeElement *other)
    {
        const PaletteContainer * ptr = static_cast<const PaletteContainer*>(other);
        if(!ptr)
            throw std::runtime_error("PaletteContainer::clone(): other is not a PaletteContainer!");
        (*this) = *ptr;
    }

public:
    QVariant nodeData(int column, int role) const override;

    Sprite                      * parentSprite();
    inline bool                   nodeIsMutable()const override {return false;}
    inline PaletteModel         * getModel()                    {return m_model.data();}
    inline const PaletteModel   * getModel()const               {return m_model.data();}

    inline QVector<QRgb>        &getPalette()                           {return m_pal;}
    inline const QVector<QRgb>  &getPalette()const                      {return m_pal;}
    inline void                 setPalette(QVector<QRgb>        & pal)  {m_pal = pal;}
    inline void                 setPalette(const QVector<QRgb>  & pal)  {m_pal = pal;}
    inline void                 setPalette(QVector<QRgb>        && pal) {m_pal = qMove(pal);}

private:
    QScopedPointer<PaletteModel>    m_model;
    QVector<QRgb>                   m_pal;
};

#endif // SPRITE_PALETTE_HPP
