#ifndef TABEFFECTS_HPP
#define TABEFFECTS_HPP
#include <src/ui/tabs/basespritetab.hpp>
#include <QWidget>

namespace Ui {
class TabEffects;
}

class TabEffects : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabEffects(QWidget *parent = nullptr);
    ~TabEffects();

    // BaseSpriteTab interface
public:
    void OnShowTab(Sprite *pspr, QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private:
    Ui::TabEffects *ui; 
};

#endif // TABEFFECTS_HPP
