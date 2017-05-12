#include "dialognewsprite.hpp"
#include "ui_dialognewsprite.h"

DialogNewSprite::DialogNewSprite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewSprite)
{
    ui->setupUi(this);
}

DialogNewSprite::~DialogNewSprite()
{
    delete ui;
}
