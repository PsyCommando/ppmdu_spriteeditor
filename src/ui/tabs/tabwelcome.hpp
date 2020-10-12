#ifndef TABWELCOME_HPP
#define TABWELCOME_HPP

#include <QWidget>
#include <src/ui/tabs/basespritetab.hpp>

namespace Ui {
class TabWelcome;
}

class TabWelcome : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabWelcome(QWidget *parent = nullptr);
    ~TabWelcome();

    // BaseSpriteTab interface
public:
    void PrepareForNewContainer() override;

private:
    Ui::TabWelcome *ui;
};

#endif // TABWELCOME_HPP
