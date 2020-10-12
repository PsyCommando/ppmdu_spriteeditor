#include "tabempty.hpp"
#include "ui_tabempty.h"

TabEmpty::TabEmpty(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::tabempty)
{
    ui->setupUi(this);
}

TabEmpty::~TabEmpty()
{
    delete ui;
}


void TabEmpty::PrepareForNewContainer()
{
}
