#ifndef TABEMPTY_HPP
#define TABEMPTY_HPP
#include <QWidget>
#include <src/ui/tabs/basespritetab.hpp>

namespace Ui {
class tabempty;
}

/*
 * Empty tab displayed when there's nothing to display for a given element!
*/
class TabEmpty : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabEmpty(QWidget *parent = nullptr);
    ~TabEmpty();

    // BaseSpriteTab interface
public:
    void PrepareForNewContainer() override;

private:
    Ui::tabempty *ui;
};

#endif // TABEMPTY_HPP
