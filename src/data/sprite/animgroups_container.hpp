#ifndef ANIMGROUPS_HPP
#define ANIMGROUPS_HPP
#include <src/data/sprite/animgroup.hpp>
#include <src/data/treenodewithchilds.hpp>
#include <src/ppmdu/fmts/wa_sprite.hpp>

extern const QString ElemName_AnimGroups;

//Contains all animation groups used in the sprites
class AnimGroups : public TreeNodeWithChilds<AnimGroup>
{
    using parent_t = TreeNodeWithChilds<AnimGroup>;
public:
    AnimGroups(TreeNode *parent);
    AnimGroups(const AnimGroups & cp);
    AnimGroups(AnimGroups && mv);
    AnimGroups& operator=(const AnimGroups & cp);
    AnimGroups& operator=(AnimGroups && mv);
    ~AnimGroups();

    AnimGroup *         getGroup(fmt::animgrpid_t id);
    const AnimGroup *   getGroup(fmt::animgrpid_t id)const;

    void importAnimationGroups(const fmt::AnimDB::animgrptbl_t & grps);
    fmt::AnimDB::animgrptbl_t exportAnimationGroups()const;

    //Resizes all groups to have this exact amount of slots
    void setNbGroupSlots(int nbslots);

    // TreeNode interface
public:
    TreeNode *clone() const override;
    eTreeElemDataType nodeDataTy() const override;
    const QString &nodeDataTypeName() const override;
    QString nodeDisplayName() const override;
};

#endif // ANIMGROUPS_HPP
