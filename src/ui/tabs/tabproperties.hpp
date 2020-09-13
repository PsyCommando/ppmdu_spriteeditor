#ifndef TABPROPERTIES_HPP
#define TABPROPERTIES_HPP

#include <src/ui/tabs/basespritetab.hpp>
#include <QWidget>

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
    void OnShowTab(Sprite *pspr, QPersistentModelIndex element) override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private:
    void on_btnExportPalette_clicked();
    void on_btnImportPalette_clicked();
    static QString GetPaletteImportFiterString();
    static const QString PaletteFilterString();

    QPixmap GenerateSpritePreviewImage();
    QPixmap GenerateSpritePreviewPalette();
    void UpdatePreview();
private:
    Ui::TabProperties *ui;


};

#endif // TABPROPERTIES_HPP
