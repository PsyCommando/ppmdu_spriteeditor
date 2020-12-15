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
    static const QString IMG_FORMAT_IO;
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
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();
    void on_btnExport_clicked();

    void on_btnImport_clicked();

private:
    void SetupMappings(Sprite * spr);
    void ClearMappings();
    void SetupImage(QPersistentModelIndex img);
    void ClearImage();
    QModelIndexList getSelectedIndices() const;
    void TryImportImage(const QString & path);

private:
    Ui::TabImages *ui;
    QScopedPointer<QDataWidgetMapper>   m_imgdatmapper;

    QPersistentModelIndex               m_currentImage;
    QScopedPointer<ImageListModel>      m_imgListModel;
};

#endif // TABIMAGES_HPP
