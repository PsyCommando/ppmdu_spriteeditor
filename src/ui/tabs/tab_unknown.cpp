#include "tab_unknown.hpp"
#include "ui_tab_unknown.h"

TabUnknown::TabUnknown(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabUnknown)
{
    ui->setupUi(this);
}

TabUnknown::~TabUnknown()
{
    delete ui;
}

void TabUnknown::OnShowTab(QPersistentModelIndex element)
{
}

void TabUnknown::OnHideTab()
{
}

void TabUnknown::PrepareForNewContainer()
{
}
