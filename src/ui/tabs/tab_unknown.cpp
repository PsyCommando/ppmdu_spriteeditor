#include "tab_unknown.hpp"
#include "ui_tab_unknown.h"
#include <src/data/models/unknown_item_model.hpp>

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
    UnknownItemNode * itm = static_cast<UnknownItemNode*>(element.internalPointer());
    m_overviewModel.reset(new UnknownItemModel(itm));
    ui->tblvDetails->setModel(m_overviewModel.data());
}

void TabUnknown::OnHideTab()
{
    PrepareForNewContainer();
}

void TabUnknown::PrepareForNewContainer()
{
    ui->tblvDetails->setModel(nullptr);
    m_overviewModel.reset();
}
