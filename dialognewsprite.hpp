#ifndef DIALOGNEWSPRITE_HPP
#define DIALOGNEWSPRITE_HPP

#include <QDialog>

namespace Ui {
class DialogNewSprite;
}

class DialogNewSprite : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewSprite(QWidget *parent = 0);
    ~DialogNewSprite();

private:
    Ui::DialogNewSprite *ui;
};

#endif // DIALOGNEWSPRITE_HPP
