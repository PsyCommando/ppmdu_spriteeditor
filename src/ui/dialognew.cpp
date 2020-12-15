#include "dialognew.hpp"
#include "ui_dialognew.h"
#include <QStandardItemModel>
#include <src/data/content_factory.hpp>

DialogNew::DialogNew(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNew)
{
    ui->setupUi(this);
    InitTabs(); //Init tabs first since we refer to them in the typelist
    InitTypeList();

}

DialogNew::~DialogNew()
{
    delete ui;
}

void DialogNew::InitTabs()
{
    //Add sprite format types to format list
    ui->cmbSprFormat->addItem(SpriteContainer::ContainerTypeNames[SpriteContainer::eContainerType::WAN], static_cast<int>(SpriteContainer::eContainerType::WAN));
    ui->cmbSprFormat->addItem(SpriteContainer::ContainerTypeNames[SpriteContainer::eContainerType::WAT], static_cast<int>(SpriteContainer::eContainerType::WAT));
    ui->cmbSprFormat->addItem(SpriteContainer::ContainerTypeNames[SpriteContainer::eContainerType::PACK], static_cast<int>(SpriteContainer::eContainerType::PACK));
    ui->cmbSprFormat->setCurrentIndex(0);

    //Add sprite compression types to list
    for(int i = 0; i < CompressionFmtOptions.size(); ++i)
        ui->cmbSprCompression->addItem(CompressionFmtOptions[i], i);
    ui->cmbSprCompression->setCurrentIndex(static_cast<int>(eCompressionFmtOptions::NONE));
}

void DialogNew::InitTypeList()
{
    ContentContainerFactory                      & factory = ContentContainerFactory::instance();
    const ContentContainerFactory::regtypeslst_t & types = factory.RegisteredContainerTypes();

    auto ittypes = types.begin();
    for(size_t i = 0; i < types.size(); ++i, ++ittypes)
        ui->cmbContainerType->addItem(ittypes->second.cntDisplayName, ittypes->first);

    if(types.size() == 1)
        ui->cmbContainerType->setEnabled(false); //Disable it when there's only one type of item
    if(types.size() > 0)
    {
        ui->cmbContainerType->setCurrentIndex(0);
        SelectType(ui->cmbContainerType->currentData(Qt::UserRole).toString());
    }
}

void DialogNew::SelectType(const QString &type)
{
    if(type == ContentName_Sprite)
    {
        ui->stkContentOption->setCurrentWidget(ui->tabSprite);
    }
    else
    {
        qWarning() << "DialogNew::SelectType(): Got unknown container type \"" <<type <<"\" ignoring..";
        ui->stkContentOption->setCurrentWidget(ui->tabEmpty);
    }
}

QString DialogNew::getContentType() const
{
    return ui->cmbContainerType->currentData(Qt::UserRole).toString();
}

eCompressionFmtOptions DialogNew::getSprCompression() const
{
    return static_cast<eCompressionFmtOptions>(ui->cmbSprCompression->currentData(Qt::UserRole).toInt());
}

SpriteContainer::eContainerType DialogNew::getSprFormatType() const
{
    return static_cast<SpriteContainer::eContainerType>(ui->cmbSprFormat->currentData(Qt::UserRole).toInt());
}

void DialogNew::on_cmbContainerType_activated(int index)
{
    SelectType(ui->cmbContainerType->itemData(index, Qt::UserRole).toString());
}

void DialogNew::on_cmbContainerType_currentIndexChanged(int index)
{

}

void DialogNew::on_cmbSprFormat_activated(int index)
{

}

void DialogNew::on_cmbSprCompression_activated(int index)
{

}
