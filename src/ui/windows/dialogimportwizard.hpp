#ifndef DIALOGIMPORTWIZARD_HPP
#define DIALOGIMPORTWIZARD_HPP

#include <QDialog>
#include <QPointer>
#include <QStandardItem>
#include <src/ui/editor/cropper/autocropper_scene.hpp>

namespace Ui {
class DialogImportWizard;
}

class Sprite;
class MainWindow;
class DialogImportWizard : public QDialog
{
    Q_OBJECT

public:
    using imgchunk_t   = std::pair<QRect,QImage>; //A single chunk we cut from the source image, with its rect for position on the parent image
    using choppedimg_t = QVector<imgchunk_t>;   //All the parts cut from a single source image

    explicit DialogImportWizard(MainWindow *parent, Sprite * pspr);
    ~DialogImportWizard();

    void Reset();

private slots:
    void on_btnOpenImage_clicked();

    void on_btnSaveTemplate_clicked();

    void on_btnLoadTemplate_clicked();

    void on_btnImport_clicked();

    void on_tblProperties_cellChanged(int row, int column);

private:
    void SetupConnections();
    void ClearConnections();

    void SetupViewport();
    void ClearViewport();

    void SetupPropertiesTable();
    void UpdatePropertiesTable();
    void UpdateGizmo();
    void ClearPropertiesTable();

    void SetupImage(const QString & imgpath);
    void ClearImage();

    //Returns all the QRects of the cells to cut in the source image
    // Sorted by animation sequences
    QVector<QVector<QRectF>> getCellsRectToCrop();

    QVector<choppedimg_t> autochopImages(const QVector<QImage>& imgs);
    choppedimg_t autochopImage(const QImage &img);

private:
    Ui::DialogImportWizard *ui;
    QPointer<MainWindow>    m_main;
    Sprite *                m_pSprite {nullptr};
    QStandardItemModel      m_propModel;

    AutoCropperScene        m_scene;
    QImage                  m_loadedImg;
    QString                 m_lastImgPath;

    bool                    m_bRemoveEmptyCells{true};
    uint8_t                 m_frameDuration{1};
};

#endif // DIALOGIMPORTWIZARD_HPP
