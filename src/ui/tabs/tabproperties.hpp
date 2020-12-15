#ifndef TABPROPERTIES_HPP
#define TABPROPERTIES_HPP

#include <src/ui/tabs/basespritetab.hpp>
#include <QWidget>
#include <src/data/sprite/models/sprite_props_handler.hpp>
#include <src/data/sprite/models/sprite_overview_model.hpp>
#include <src/utility/file_support.hpp>

namespace Ui {
class TabProperties;
}



class TabProperties : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabProperties(QWidget *parent = nullptr);
    ~TabProperties();

    // BaseSpriteTab interface
public:
    void OnShowTab(QPersistentModelIndex element)override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private slots:
    void on_btnExportPalette_clicked();
    void on_btnImportPalette_clicked();
    //static QString GetPaletteImportFiterString();
    //static const QString PaletteFilterString();
private:
    QPixmap GenerateSpritePreviewImage();
    QPixmap GenerateSpritePreviewPalette();
    void UpdatePreview();
private:
    Ui::TabProperties *ui;
    QScopedPointer<SpritePropertiesHandler> m_propHandler;
    QScopedPointer<SpriteOverviewModel>     m_overviewModel;
};

#endif // TABPROPERTIES_HPP
