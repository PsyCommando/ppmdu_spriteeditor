#ifndef PALETTEEDITOR_HPP
#define PALETTEEDITOR_HPP

#include <QDialog>
#include <QVector>
#include <QRgb>
#include <QColor>
#include <QAbstractItemModel>
#include <QScopedPointer>
#include <QPointer>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDataWidgetMapper>
#include <QPersistentModelIndex>
#include <QGraphicsRectItem>

#include <src/sprite_img.hpp>
#include <src/spritemanager.h>
#include <src/ppmdu/utils/color_utils.hpp>

namespace Ui {
    class PaletteEditor;
}

static const int PaletteDisplayNBColorsPerRow = 16;

//===============================================================================
//  PaletteEditorScene
//===============================================================================
class PaletteEditorScene : public QGraphicsScene
{
    Q_OBJECT

    QPointer<PaletteModel> m_model;
    QPersistentModelIndex  m_curindex;
    QGraphicsRectItem     *m_cursor;
public:
    PaletteEditorScene(QObject *parent = nullptr);

public:
    QModelIndex currentIndex();

signals:
    void selectedIndex(const QModelIndex &);

public:
    static const size_t NbColorsPerRow = PaletteDisplayNBColorsPerRow;
    static const size_t ColorSquareRes = 16;


    void selectFirst();

    void displayPaletteModel( PaletteModel * model );

    void redrawPalette();

    void fillRow( int & cntcolr, int curY, int rowlen = PaletteDisplayNBColorsPerRow );

    void select(const QModelIndex & index);

    // QGraphicsScene interface
protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    virtual void keyPressEvent(QKeyEvent *event) override;

    virtual void keyReleaseEvent(QKeyEvent *event) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
};

//===============================================================================
//  PaletteEditor
//===============================================================================
class PaletteEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteEditor(QWidget *parent = 0);
    ~PaletteEditor();

public slots:
    void setPalModel(PaletteModel * model);

    void redrawPalette();

    void on_btnPickColor_clicked();

    void on_btnClearColor_clicked();

    void on_btnImportPal_clicked();

    void on_btnExportPal_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

signals:
    void importPalette();
    void exportPalette();

private:
    Ui::PaletteEditor *ui;
    QPointer<PaletteModel> m_model;
    QScopedPointer<PaletteEditorScene> m_scene;
    QScopedPointer<QDataWidgetMapper> m_wmap;
};

#endif // PALETTEEDITOR_HPP
