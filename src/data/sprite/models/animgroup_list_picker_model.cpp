#include "animgroup_list_picker_model.hpp"
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/animtable.hpp>

enum eAnimGroupListPickerColumns : int
{
    GroupID,
    GroupPreview,
};

const QMap<int,QString> AnimGroupListPickerColumnNames
{
    {eAnimGroupListPickerColumns::GroupID,      "ID"},
    {eAnimGroupListPickerColumns::GroupPreview, "Preview"},
};

const int AnimGroupListPickerModel::MAX_NB_PREVIEW_IMGS = 8;
const int AnimGroupListPickerModel::WIDTH_PREVIEW_IMGS  = 256;
const int AnimGroupListPickerModel::HEIGHT_PREVIEW_IMGS = 32;

AnimGroupListPickerModel::AnimGroupListPickerModel(Sprite * owner)
    :TreeNodeModel(nullptr)
{
    m_sprite = owner;
}

AnimGroupListPickerModel::~AnimGroupListPickerModel()
{
}

TreeNodeModel::node_t *AnimGroupListPickerModel::getRootNode()
{
    return &m_sprite->getAnimTable();
}

Sprite *AnimGroupListPickerModel::getOwnerSprite()
{
    return m_sprite;
}

int AnimGroupListPickerModel::columnCount(const QModelIndex &parent) const
{
//    if(!parent.isValid())
//        return 0;
    return AnimGroupListPickerColumnNames.size();
}

QVariant AnimGroupListPickerModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if(index.isValid() && (role == Qt::DecorationRole || role == Qt::DisplayRole))
    {
        const AnimGroup * grp = static_cast<const AnimGroup *>(index.internalPointer());
        if(!grp)
            throw std::runtime_error("AnimGroupPickerModel::data(): Index pointer was null!");
        switch(index.column())
        {
        case eAnimGroupListPickerColumns::GroupID:
            {
                if(role == Qt::DisplayRole)
                    data = grp->getGroupUID();
                break;
            }
        case eAnimGroupListPickerColumns::GroupPreview:
            {
                //Generate preview icons for up to MAX_NB_PREVIEW_IMGS sub-sequences, and merge them into one image of fixed width
                if(role == Qt::DecorationRole)
                    data = grp->MakeGroupPreview(m_sprite, WIDTH_PREVIEW_IMGS, HEIGHT_PREVIEW_IMGS, MAX_NB_PREVIEW_IMGS);
                //If there's more than MAX_NB_PREVIEW_IMGS sub-sequences, add an elipsis to imply there's more
                else if(role == Qt::DisplayRole && grp->nodeChildCount() > MAX_NB_PREVIEW_IMGS)
                    data = "...";
                break;
            }
        };
    }
    return data;
}

QVariant AnimGroupListPickerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if(section < AnimGroupListPickerColumnNames.size() && orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch(section)
        {
        case eAnimGroupListPickerColumns::GroupID:
            {
                data = AnimGroupListPickerColumnNames[eAnimGroupListPickerColumns::GroupID];
                break;
            }
        case eAnimGroupListPickerColumns::GroupPreview:
            {
                data = AnimGroupListPickerColumnNames[eAnimGroupListPickerColumns::GroupPreview];
                break;
            }
        }
    }
    return data;
}
