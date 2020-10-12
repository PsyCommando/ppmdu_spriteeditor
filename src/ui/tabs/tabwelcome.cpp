#include "tabwelcome.hpp"
#include "ui_tabwelcome.h"

TabWelcome::TabWelcome(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabWelcome)
{
    ui->setupUi(this);
}

TabWelcome::~TabWelcome()
{
    delete ui;
}

void TabWelcome::PrepareForNewContainer()
{

}
