#include "paletteeditor.hpp"
#include "ui_paletteeditor.h"

PaletteEditor::PaletteEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PaletteEditor),
    m_model(nullptr)
{
    ui->setupUi(this);
}

PaletteEditor::~PaletteEditor()
{
    delete ui;
}

void PaletteEditor::setPalModel(PaletteModel *model)
{
    m_model = model;
    m_scene.reset(new PaletteEditorScene);
    m_scene->displayPaletteModel(m_model.data());


    //Map model's columns to some of the controls
    m_wmap.reset(new QDataWidgetMapper);
    m_wmap->setModel(m_model);
    m_wmap->addMapping(ui->spbColRed,   0 );
    m_wmap->addMapping(ui->spbColRed,   1 );
    m_wmap->addMapping(ui->spbColGreen, 2 );
    m_wmap->addMapping(ui->spbColBlue,  3 );
    m_wmap->toFirst();
    m_scene->selectFirst();
    connect(m_scene.data(), &PaletteEditorScene::selectedIndex,
            m_wmap.data(), &QDataWidgetMapper::setCurrentModelIndex);


}

void PaletteEditor::redrawPalette()
{
    if(m_scene)
        m_scene->redrawPalette();
}


//
//  Actions:
//
void PaletteEditor::on_btnPickColor_clicked()
{
    //TODO: Show color picker!
    redrawPalette();
}

void PaletteEditor::on_btnClearColor_clicked()
{
    //RESET COLOR
    redrawPalette();
}

void PaletteEditor::on_btnImportPal_clicked()
{
    emit importPalette();
    redrawPalette();
}

void PaletteEditor::on_btnExportPal_clicked()
{
    emit exportPalette();
    redrawPalette();
}

void PaletteEditor::on_buttonBox_accepted()
{
    if(m_model)
        m_model->submit();
    close();
}

void PaletteEditor::on_buttonBox_rejected()
{
    if(m_model)
        m_model->revert();
    close();
}


//===================================================================================
//  PaletteEditorScene
//===================================================================================
PaletteEditorScene::PaletteEditorScene(QObject *parent)
    :QGraphicsScene(0, 0, 128, 128, parent)
{

}

QModelIndex PaletteEditorScene::currentIndex()
{
    return m_curindex;
}

void PaletteEditorScene::selectFirst()
{

}

void PaletteEditorScene::displayPaletteModel(PaletteModel *model)
{
    m_model = model;
    redrawPalette();
}

void PaletteEditorScene::redrawPalette()
{
    clear();
    m_cursor   = nullptr;
    m_curindex = m_model->index(0,0, QModelIndex());

    const int NbColors = m_model->rowCount(QModelIndex());
//    const size_t NbRows = (NbColors % NbColorsPerRow == 0)?
//                              (NbColors / NbColorsPerRow) :
//                              ((NbColors / NbColorsPerRow) + 1);
    int curY = 0;
    for( int cntcolr = 0; cntcolr < NbColors; )
    {
        fillRow(cntcolr, curY);
        curY += NbColorsPerRow;
    }

    select(m_curindex);
}

void PaletteEditorScene::fillRow(int &cntcolr, int curY, int rowlen)
{
    const int NbColors = m_model->rowCount(QModelIndex());
    for(int cntr = 0; (cntr < rowlen) && (cntcolr < NbColors); ++cntr, ++cntcolr)
    {
        QVariant dat = m_model->data( m_model->index(cntcolr, 0, QModelIndex()), Qt::DisplayRole );
        QGraphicsPixmapItem * pix = this->addPixmap( QPixmap::fromImage(dat.value<QImage>()) );
        pix->setPos( cntr * ColorSquareRes, curY );
    }
}

void PaletteEditorScene::select(const QModelIndex &index)
{
    QColor selcol( QRgb(m_model->data(index, Qt::EditRole).toUInt()) );
    m_curindex = QModelIndex(index);
    if(!m_cursor)
    {
        //Make cursor
        m_cursor = addRect( m_curindex.row() % NbColorsPerRow,
                            m_curindex.row() / NbColorsPerRow,
                            ColorSquareRes,
                            ColorSquareRes,
                            QPen(selcol.lighter(200)));
    }
    else
    {
        m_cursor->setPos( m_curindex.row() % NbColorsPerRow,
                          m_curindex.row() / NbColorsPerRow );
        m_cursor->setPen(QPen(selcol.lighter(200)));
    }
    emit selectedIndex(index);
}

void PaletteEditorScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsScene::contextMenuEvent(event);
}

void PaletteEditorScene::keyPressEvent(QKeyEvent *event)
{
    QGraphicsScene::keyPressEvent(event);
}

void PaletteEditorScene::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsScene::keyReleaseEvent(event);
}

void PaletteEditorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
}

void PaletteEditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
}

void PaletteEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}

void PaletteEditorScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
}

void PaletteEditorScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter,rect);
}
