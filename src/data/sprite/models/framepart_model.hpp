#ifndef FRAMEPARTSMODEL_HPP
#define FRAMEPARTSMODEL_HPP
#include <src/data/treenodemodel.hpp>

//*******************************************************************
//  MFramePartModel
//*******************************************************************
//Model for representing the MFramePart that make up a MFrame on the mframe editor tab!
class MFrame;
class MFramePart;
class MFramePartModel : public TreeNodeModel
{
    Q_OBJECT

public:
    explicit MFramePartModel(MFrame* pmfrm, Sprite* pspr);
    ~MFramePartModel();

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // TreeNodeModel interface
public:
    node_t *getRootNode() override;
    Sprite *getOwnerSprite() override;

private:
    QVariant dataImgPreview     (const MFramePart * part, int role)const;
    QVariant dataImgId          (const MFramePart * part, int role)const;
    QVariant dataImgIsRef       (const MFramePart * part, int role)const;
    QVariant dataBlockRange      (const MFramePart * part, int role)const;
    QVariant dataImgSize        (const MFramePart * part, int role)const;
    QVariant dataUnk0           (const MFramePart * part, int role)const;
    QVariant dataXOffset        (const MFramePart * part, int role)const;
    QVariant dataYOffset        (const MFramePart * part, int role)const;
    QVariant dataVFlip          (const MFramePart * part, int role)const;
    QVariant dataHFlip          (const MFramePart * part, int role)const;
    QVariant dataRnS            (const MFramePart * part, int role)const;
    QVariant dataRnSParam       (const MFramePart * part, int role)const;
    QVariant dataRotCanvas      (const MFramePart * part, int role)const;
    QVariant dataPaletteID      (const MFramePart * part, int role)const;
    QVariant dataPriority       (const MFramePart * part, int role)const;
    QVariant dataBlockNum       (const MFramePart * part, int role)const;
    QVariant dataMosaic         (const MFramePart * part, int role)const;
    QVariant dataMode           (const MFramePart * part, int role)const;

    static QSize calcTextSize(const QString &text);

private:
    Sprite* m_sprite{nullptr};
    MFrame* m_root  {nullptr};
};

#endif // FRAMEPARTSMODEL_HPP
