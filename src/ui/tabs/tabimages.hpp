#ifndef TABIMAGES_HPP
#define TABIMAGES_HPP
#include <src/ui/tabs/basespritetab.hpp>
#include <QWidget>
#include <QModelIndex>
#include <QDataWidgetMapper>
#include <QPixmap>
#include <src/data/sprite/models/images_list_model.hpp>

namespace Ui {
class TabImages;
}

class TabImages : public BaseSpriteTab
{
    Q_OBJECT

public:
    explicit TabImages(QWidget *parent = nullptr);
    ~TabImages();

    // BaseSpriteTab interface
public:
    void OnShowTab(QPersistentModelIndex element)override;
    void OnHideTab() override;
    void OnDestruction() override;
    void PrepareForNewContainer() override;
    void OnItemRemoval(const QModelIndex &item) override;

private slots:
    void on_tblviewImages_clicked(const QModelIndex &index);
    void on_btnImagesExport_clicked();
    void on_btnImagesImport_clicked();
    void on_spbimgunk2_valueChanged(int arg1);
    void on_spbimgunk14_valueChanged(int arg1);
    void on_btnImageCrop_clicked();

private:
    void ClearMappings();
    void SetupImage(QPersistentModelIndex img, Sprite * spr);
    void ClearImage();

private:
    Ui::TabImages *ui;
    QScopedPointer<QDataWidgetMapper>   m_imgdatmapper;

    QPersistentModelIndex               m_currentImage;
    QScopedPointer<ImageListModel>      m_imgListModel;
    //QPixmap                             m_imgNoImg;             //Image displayed when no image can be displayed in a view!
};

#endif // TABIMAGES_HPP