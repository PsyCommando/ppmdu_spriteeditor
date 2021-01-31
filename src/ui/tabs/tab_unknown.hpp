#ifndef TAB_UNKNOWN_HPP
#define TAB_UNKNOWN_HPP
#include <QModelIndex>
#include <QWidget>
#include <src/ui/tabs/basespritetab.hpp>

namespace Ui {
class TabUnknown;
}

class TabUnknown : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabUnknown(QWidget *parent = nullptr);
    ~TabUnknown();

    // BaseSpriteTab interface
public:
    void OnShowTab(QPersistentModelIndex element) override;
    void OnHideTab() override;
    void PrepareForNewContainer() override;

private:
    Ui::TabUnknown *ui;
};

#endif // TAB_UNKNOWN_HPP
