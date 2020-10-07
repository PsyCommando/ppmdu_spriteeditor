#include "tabeffects.hpp"
#include "ui_tabeffects.h"

TabEffects::TabEffects(QWidget *parent) :
    BaseSpriteTab(parent),
    ui(new Ui::TabEffects)
{
    ui->setupUi(this);
}

TabEffects::~TabEffects()
{
    delete ui;
}


void TabEffects::OnShowTab(QPersistentModelIndex element)
{
    BaseSpriteTab::OnShowTab(element);
}

void TabEffects::OnHideTab()
{
    BaseSpriteTab::OnHideTab();
}

void TabEffects::OnDestruction()
{
    BaseSpriteTab::OnDestruction();
}

void TabEffects::PrepareForNewContainer()
{
}

void TabEffects::OnItemRemoval(const QModelIndex &item)
{
    BaseSpriteTab::OnItemRemoval(item);
}
