#include "dialogimportwizard.hpp"
#include "ui_dialogimportwizard.h"
#include <QFileDialog>
#include <src/ui/mainwindow.hpp>
#include <src/utility/file_support.hpp>
#include <src/data/sprite/sprite.hpp>
#include <src/ui/editor/cropper/autocropper_gizmo.hpp>
#include <src/ppmdu/fmts/sprite/shared_sprite_constants.hpp>

const int PROP_ROW_CELL_WIDTH = 0;
const int PROP_ROW_CELL_HEIGHT = 1;
const int PROP_ROW_CELL_MARGINH = 2;
const int PROP_ROW_CELL_MARGINV = 3;
const int PROP_ROW_NB_CELLS_H = 4;
const int PROP_ROW_NB_CELLS_V = 5;
const int PROP_ROW_NB_SEQ_H = 6;
const int PROP_ROW_NB_SEQ_V = 7;
const int PROP_ROW_X = 8;
const int PROP_ROW_Y = 9;
const int PROP_ROW_REMOVE_EMPTY_CELLS = 10;
const int PROP_ROW_DEF_FRAME_DURATION = 11;

DialogImportWizard::DialogImportWizard(MainWindow *parent, Sprite * pspr) :
    QDialog(parent),
    ui(new Ui::DialogImportWizard),
    m_pSprite(pspr)
{
    ui->setupUi(this);
}

DialogImportWizard::~DialogImportWizard()
{
    delete ui;
}

void DialogImportWizard::Reset()
{

}

void DialogImportWizard::SetupConnections()
{

}

void DialogImportWizard::ClearConnections()
{

}

void DialogImportWizard::SetupViewport()
{

}

void DialogImportWizard::ClearViewport()
{

}

void DialogImportWizard::SetupPropertiesTable()
{
    m_propModel.setRowCount(0);
    m_propModel.setColumnCount(1);
    m_propModel.setParent(ui->tblvProperties);

    //Setup horizontal header
    m_propModel.setHeaderData(0, Qt::Horizontal, QObject::tr("Value"));

    //Setup values
    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_WIDTH, 0), fmt::MaxFrameResValue.first);
    m_propModel.setHeaderData(0, Qt::Vertical, tr("Cell Width"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_HEIGHT, 0), fmt::MaxFrameResValue.second);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Cell Height"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_MARGINH, 0), 0);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Cell H Margin"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_MARGINV, 0), 0);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Cell V Margin"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_CELLS_H, 0), 1);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Nb Cells H"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_CELLS_V, 0), 1);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Nb Cells V"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_SEQ_H, 0), 1);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Nb Sequences H"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_SEQ_V, 0), 1);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Nb Sequences V"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_X, 0), 0);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("X"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_Y, 0), 0);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Y"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_REMOVE_EMPTY_CELLS, 0), (bool)true);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Remove empty cells"));

    m_propModel.insertRow(m_propModel.rowCount());
    m_propModel.setData(m_propModel.index(PROP_ROW_DEF_FRAME_DURATION, 0), 1);
    m_propModel.setHeaderData(1, Qt::Vertical, tr("Default frame duration"));

    ui->tblvProperties->setModel(&m_propModel);
    ui->tblvProperties->update();
}

void DialogImportWizard::UpdatePropertiesTable()
{
    AutoCropperGizmo * pgizmo = m_scene.getGizmo();
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_WIDTH, 0),  pgizmo->cellWidht());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_HEIGHT, 0), pgizmo->cellHeight());

    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_MARGINH, 0), pgizmo->cellMarginH());
    m_propModel.setData(m_propModel.index(PROP_ROW_CELL_MARGINV, 0), pgizmo->cellMarginV());

    m_propModel.setData(m_propModel.index(PROP_ROW_NB_CELLS_H, 0), pgizmo->nbCellsPerSeqH());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_CELLS_V, 0), pgizmo->nbCellsPerSeqV());

    m_propModel.setData(m_propModel.index(PROP_ROW_NB_SEQ_H, 0), pgizmo->nbSeqH());
    m_propModel.setData(m_propModel.index(PROP_ROW_NB_SEQ_V, 0), pgizmo->nbSeqV());

    m_propModel.setData(m_propModel.index(PROP_ROW_X, 0), pgizmo->pos().x());
    m_propModel.setData(m_propModel.index(PROP_ROW_Y, 0), pgizmo->pos().y());

    m_propModel.setData(m_propModel.index(PROP_ROW_REMOVE_EMPTY_CELLS, 0), m_bRemoveEmptyCells );
    m_propModel.setData(m_propModel.index(PROP_ROW_DEF_FRAME_DURATION, 0), m_frameDuration);
}

void DialogImportWizard::UpdateGizmo()
{
    AutoCropperGizmo * pgizmo = m_scene.getGizmo();
    pgizmo->setCellWidht(m_propModel.data(m_propModel.index(PROP_ROW_CELL_WIDTH, 0), Qt::EditRole).toInt());
    pgizmo->setCellHeight(m_propModel.data(m_propModel.index(PROP_ROW_CELL_HEIGHT, 0), Qt::EditRole).toInt());
    pgizmo->setCellMarginH(m_propModel.data(m_propModel.index(PROP_ROW_CELL_MARGINH, 0), Qt::EditRole).toInt());
    pgizmo->setCellMarginV(m_propModel.data(m_propModel.index(PROP_ROW_CELL_MARGINV, 0), Qt::EditRole).toInt());
    pgizmo->setNbCellsPerSeqH(m_propModel.data(m_propModel.index(PROP_ROW_NB_CELLS_H, 0), Qt::EditRole).toInt());
    pgizmo->setNbCellsPerSeqV(m_propModel.data(m_propModel.index(PROP_ROW_NB_CELLS_V, 0), Qt::EditRole).toInt());
    pgizmo->setNbSeqH(m_propModel.data(m_propModel.index(PROP_ROW_NB_SEQ_H, 0), Qt::EditRole).toInt());
    pgizmo->setNbSeqV(m_propModel.data(m_propModel.index(PROP_ROW_NB_SEQ_V, 0), Qt::EditRole).toInt());
    pgizmo->setX(m_propModel.data(m_propModel.index(PROP_ROW_X, 0), Qt::EditRole).toInt());
    pgizmo->setY(m_propModel.data(m_propModel.index(PROP_ROW_Y, 0), Qt::EditRole).toInt());
    m_bRemoveEmptyCells = m_propModel.data(m_propModel.index(PROP_ROW_REMOVE_EMPTY_CELLS, 0), Qt::EditRole).toBool();
    m_frameDuration = m_propModel.data(m_propModel.index(PROP_ROW_DEF_FRAME_DURATION, 0), Qt::EditRole).toBool();
}

void DialogImportWizard::ClearPropertiesTable()
{
    ui->tblvProperties->setModel(nullptr);
    m_scene.getGizmo();
}

void DialogImportWizard::SetupImage(const QString & imgpath)
{
    m_lastImgPath = imgpath;
    m_loadedImg = QImage(imgpath);
    m_scene.setImage(m_loadedImg);

    ui->btnImport->         setEnabled(true);
    ui->btnSaveTemplate->   setEnabled(true);
    ui->btnReset->          setEnabled(true);
    ui->tblProperties->     setEnabled(true);

    ui->gvSheetEditor->setScene(&m_scene);
    ui->gvSheetEditor->update();
    //#TODO: setup scene
}

void DialogImportWizard::ClearImage()
{
    ui->gvSheetEditor->setScene(nullptr);
    m_scene.setImage(QImage());
    m_scene.Reset();
    m_lastImgPath = QString();

    ui->btnImport->         setEnabled(false);
    ui->btnSaveTemplate->   setEnabled(false);
    ui->btnReset->          setEnabled(false);
    ui->tblProperties->     setEnabled(false);

    ui->gvSheetEditor->update();
    //#TODO: reset scene
}

QVector<QVector<QRectF> > DialogImportWizard::getCellsRectToCrop()
{
    QVector<QVector<QRectF> > tocrop = m_scene.getGizmo()->GetAllCellsRects();
    return tocrop;
}

void DialogImportWizard::on_btnOpenImage_clicked()
{
    if(!m_lastImgPath.isEmpty())
    {
        //Pop dialog to ask if we really should load a new image and clear all work not saved
    }
    QString imgpath = GetImagePathFromDialog(tr("Select the image with the animation sheet to import.."), this);
    if(imgpath.isEmpty())
        return;

    //#TODO: Validate image is 4 or 8 bpp

    SetupImage(imgpath);
}

void DialogImportWizard::on_btnSaveTemplate_clicked()
{
    QString xmlpath = GetXMLSaveFile(tr("Save the current cropping template as.."), this);
}

void DialogImportWizard::on_btnLoadTemplate_clicked()
{
    QString xmlpath = GetXMLOpenFile(tr("Open a cropping template XML file.."), this);
}

bool ImageIsEmpty(const QImage& img)
{
    const QColor & DefColor = img.colorTable().first();
    for(int x = 0; x < img.width(); ++x)
    {
        for(int y = 0; y < img.height(); ++y)
        {
            if(img.pixelColor(x, y) != DefColor)
                return false;
        }
    }
    return true;
}

void DialogImportWizard::on_btnImport_clicked()
{
    QVector<QVector<QImage> > cropped;
    {
        //First, crop all cells
        QVector<QVector<QRectF>> tocrop = getCellsRectToCrop();
        cropped.reserve(tocrop.size());
        for(const QVector<QRectF> & seq : tocrop)
        {
            QVector<QImage> croppedseq;
            croppedseq.reserve(seq.size());
            for(const QRectF & cell : seq)
            {
                QImage tmpimg = m_loadedImg.copy(cell.toRect());
                if(m_bRemoveEmptyCells && ImageIsEmpty(tmpimg))
                    continue; //Skip empty cells
                croppedseq.push_back(tmpimg);
            }
        }
    }

    //autochop all cropped cells for all animation sequences
    QVector<QVector<choppedimg_t> > choppedSeq; //All images for all imported animation sequences
    {
        choppedSeq.reserve(cropped.size());
        for(const QVector<QImage> & sequenceimg : cropped)
        {
            QVector<choppedimg_t> seqchopped = autochopImages(sequenceimg);
            choppedSeq.push_back(seqchopped);
        }
    }

    //insert mframes in the sprite for all individual imported cells
    m_pSprite->importImageSequences(choppedSeq, m_frameDuration);

    //Replace the palette if specified
    if(ui->chkReplacePalette->isChecked())
        m_pSprite->setPalette(m_loadedImg.colorTable());

    //Close the dialog
    close();
}

QRect FindWeightCenter(const QImage &img, QRect area)
{
    const int ImgWidth = area.width();
    const int ImgHeight = area.height();
    const int ImgOffsetX = area.x();
    const int ImgOffsetY = area.y();
    //Weight centers on both axis
    int cntrX = 0, cntrY = 0;
    //Number of used pixels for the current weight center
    int cntrXNbPix = 0, cntrYNbPix = 0;
    QVector<int> columncount; //Count of used pixels for the given pixel column
    QVector<int> rowcount; //Count of used pixels for the given pixel row
    columncount.resize(ImgWidth);
    rowcount.resize(ImgHeight);

    const QColor unusedColor = img.colorTable().front();
    for(int x = ImgOffsetX; x < ImgWidth; ++x)
    {
        for(int y = ImgOffsetY; y < ImgHeight; ++y)
        {
            QColor col = img.pixelColor(x, y);
            if(col != unusedColor)
            {
                rowcount[y - ImgOffsetY] += 1;
                columncount[x - ImgOffsetX] += 1;
            }
        }
    }

    //Find row with most used pixels
    for(int i = 0; i < rowcount.size(); ++i)
    {
        if(rowcount[i] > cntrXNbPix)
        {
            cntrX = i + ImgOffsetX;
            cntrXNbPix = rowcount[i];
        }
    }

    for(int j = 0; j < columncount.size(); ++j)
    {
        if(columncount[j] > cntrYNbPix)
        {
            cntrY = j + ImgOffsetY;
            cntrYNbPix = columncount[j];
        }
    }
    return QRect(cntrX, cntrY, cntrXNbPix, cntrYNbPix);
}

//Draws a box around all the used pixels on the image
QRect FindSizeOfUsedPixels(const QImage &img, QRect area)
{
    const QColor unusedColor = img.colorTable().front();
    const int ImgWidth = area.width();
    const int ImgHeight = area.height();
    const int ImgOffsetX = area.x();
    const int ImgOffsetY = area.y();
    const int ImgMaxX = ImgOffsetX + ImgWidth;
    const int ImgMaxY = ImgOffsetY + ImgHeight;

    //Smallest and biggest X/Y coordinates
    int smallestX = ImgOffsetX + ImgWidth;
    int smallestY = ImgOffsetY + ImgHeight;
    int biggestX = ImgOffsetX;
    int biggestY = ImgOffsetY;

    QRect usedArea;
    for(int x = ImgOffsetX; x < ImgMaxX; ++x)
    {
        for(int y = ImgOffsetY; y < ImgMaxY; ++y)
        {
            const QColor pix = img.pixelColor(x,y);
            //Check if we got a used pixel
            if(pix != unusedColor)
            {
                if(smallestX > x)
                    smallestX = x;
                if(smallestY > y)
                    smallestY = y;
                if(biggestX < x)
                    biggestX = x; //increase the biggest until we don't hit used pixels anymore
                if(biggestY < y)
                    biggestY = y; //increase the biggest until we don't hit used pixels anymore
            }
        }
    }

    //Make sure we're using a resolution divisible by 8
    smallestX -= 8 - (smallestX % 8);
    smallestY -= 8 - (smallestY % 8);
    biggestX += (biggestX % 8);
    biggestY += (biggestY % 8);

    //Set the rectangle
    usedArea.setTop(smallestY);
    usedArea.setBottom(biggestY);
    usedArea.setLeft(smallestX);
    usedArea.setRight(biggestX);
    return usedArea;
}

enum struct eIgnoreEdge : uint8_t
{
    None   = 0,
    Bottom = 0b00000001,
    Top    = 0b00000010,
    Right  = 0b00000100,
    Left   = 0b00001000,
};

//Check if the rectangle's edges have any used pixels touching them
//ignored sets the edges to ignore when doing the test
bool DoBoxEdgesIntersectWithUsedPixels(const QImage &img, QRect box, eIgnoreEdge ignored = eIgnoreEdge::None)
{
    const QColor unusedColor = img.colorTable().front();
    const int ImgWidth = box.width();
    const int ImgHeight = box.height();
    const int ImgOffsetX = box.x();
    const int ImgOffsetY = box.y();
    const int ImgMaxX = ImgOffsetX + ImgWidth;
    const int ImgMaxY = ImgOffsetY + ImgHeight;

    //Horizontal Check
    for(int x = ImgOffsetX; x < ImgMaxX; ++x)
    {
        //Top Edge
        if(((uint8_t)ignored & (uint8_t)eIgnoreEdge::Top) == 0 && img.pixelColor(x, 0) != unusedColor)
            return true;
        //Bottom Edge
        if(((uint8_t)ignored & (uint8_t)eIgnoreEdge::Bottom) == 0 && img.pixelColor(x, ImgMaxY-1) != unusedColor)
            return true;
    }
    //Vertical Check
    for(int y = ImgOffsetY; y < ImgMaxY; ++y)
    {
        //Left Edge
        if(((uint8_t)ignored & (uint8_t)eIgnoreEdge::Left) == 0 && img.pixelColor(0, y) != unusedColor)
            return true;
        //Right Edge
        if(((uint8_t)ignored & (uint8_t)eIgnoreEdge::Right) == 0 && img.pixelColor(ImgMaxX-1, y) != unusedColor)
            return true;
    }
    return false;
}

enum struct eGrowthDirection : uint8_t
{
    Down        = 0b00000001,
    Up          = 0b00000010,
    Right       = 0b00000100,
    Left        = 0b00001000,
    DownRight   = Down | Right,
    UpRight     = Up   | Right,
    UpLeft      = Up   | Left,
    DownLeft    = Down | Left,
};


QRect GrowChoppingBox2(const QImage &img, QRect maxarea, eGrowthDirection growdir)
{
    QRect   usedarea    = FindSizeOfUsedPixels(img, maxarea);
    uint8_t growdirflag = static_cast<uint8_t>(growdir);

    //Constrain the growth to the edge of the rest of the sprite
    if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Up))
        usedarea.setBottom(maxarea.bottom());
    else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Down))
        usedarea.setTop(maxarea.top());
    if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Left))
        usedarea.setRight(maxarea.right());
    else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Right))
        usedarea.setLeft(maxarea.left());

    //Constrain to legal resolutions
    auto optres = GetNextBestImgPartResolution(QSize{usedarea.width(), usedarea.height()});
    if(optres.has_value())
    {
        QSize res = optres.value();
        usedarea.setWidth(res.width());
        usedarea.setHeight(res.height());

        //Move the rectangle close to the edge we're growing from, and keep the resolution
        if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Up))
            usedarea.moveBottom(maxarea.bottom());
        else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Down))
            usedarea.moveTop(maxarea.top());
        if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Left))
            usedarea.moveRight(maxarea.right());
        else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Right))
            usedarea.moveLeft(maxarea.left());
    }
    return usedarea;
}

//Basically try to contain as much used pixels within a rectangle of 8x8 tiles vertically, horizontally or withing a square region.
//growdir is used to tell from what edge of the area we should grow the box from
QRect GrowChoppingBox(const QImage &img, QRect area, eGrowthDirection growdir)
{
    static const QSize GROWTH_SIZE{fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_8x8)].first, fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_8x8)].second};
    static const QSize MAXSIZE = QSize{fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_64x64)].first, fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_64x64)].second};
    QRect cookieCutter{0,0, fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_8x8)].first, fmt::FrameResValues[static_cast<int>(fmt::eFrameRes::Square_8x8)].second};

    //Set starting pos of cutter
    uint8_t     growdirflag    = static_cast<uint8_t>(growdir);
    eIgnoreEdge ignoreedgeflag = eIgnoreEdge::None;
    if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Up))
    {
        cookieCutter.setBottom(area.bottom());
        cookieCutter.setTop(area.bottom() - GROWTH_SIZE.height());
        ignoreedgeflag = static_cast<eIgnoreEdge>((uint8_t)ignoreedgeflag | (uint8_t)eIgnoreEdge::Bottom);
    }
    else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Down))
    {
        cookieCutter.setTop(area.top());
        cookieCutter.setBottom(area.bottom() + GROWTH_SIZE.height());
        ignoreedgeflag = static_cast<eIgnoreEdge>((uint8_t)ignoreedgeflag | (uint8_t)eIgnoreEdge::Top);
    }
    if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Left))
    {
        cookieCutter.setLeft(area.left());
        cookieCutter.setRight(area.left() + GROWTH_SIZE.width());
        ignoreedgeflag = static_cast<eIgnoreEdge>((uint8_t)ignoreedgeflag | (uint8_t)eIgnoreEdge::Left);
    }
    else if(growdirflag & static_cast<uint8_t>(eGrowthDirection::Right))
    {
        cookieCutter.setRight(area.right());
        cookieCutter.setLeft(area.right() - GROWTH_SIZE.width());
        ignoreedgeflag = static_cast<eIgnoreEdge>((uint8_t)ignoreedgeflag | (uint8_t)eIgnoreEdge::Right);
    }

    //Grow cutter until we hit max
    while(cookieCutter.width() < MAXSIZE.width() && cookieCutter.height() < MAXSIZE.height())
    {
        if(!DoBoxEdgesIntersectWithUsedPixels(img, cookieCutter, ignoreedgeflag))
            break; //if the grown edges don't intersect with used pixels, we stop there!
        //Test an horizontal strip, then a vertical, then a square
        QRect hrect = cookieCutter;
        QRect vrect = cookieCutter;
        QRect square = cookieCutter;

        //Setup the horizontal, vertical and square
        if((uint8_t)growdir & (uint8_t)eGrowthDirection::Left)
        {
            hrect.setLeft(hrect.left() - 8);
            square.setLeft(square.left() - 8);
        }
        else
        {
            hrect.setRight(hrect.right() + 8); //Always grow right by default!!
            square.setRight(square.right() + 8);
        }
        if((uint8_t)growdir & (uint8_t)eGrowthDirection::Up)
        {
            vrect.setTop(vrect.top() - 8);
            square.setTop(square.top() - 8);
        }
        else
        {
            vrect.setBottom(vrect.bottom() + 8); //Always grow down by default!!
            square.setBottom(square.bottom() + 8);
        }

        //Text the rectangles
        if(!DoBoxEdgesIntersectWithUsedPixels(img, hrect, ignoreedgeflag))
        {
            cookieCutter = hrect;
            break;
        }
        if(!DoBoxEdgesIntersectWithUsedPixels(img, vrect, ignoreedgeflag))
        {
            cookieCutter = vrect;
            break;
        }
        cookieCutter = square; //Grow by square if the others didn't match
    }
    return cookieCutter;
}

//Image resolution MUST be divisible by 8!!!
DialogImportWizard::choppedimg_t DialogImportWizard::autochopImage(const QImage &img)
{
    choppedimg_t parts;

    //First find out if we can get away with just using the source image, or by cropping out unused pixels!

    //See if we can contain the entire thing. And pick the next best size.
    QRect sizeOfUsedPixels;
    auto newres = GetNextBestImgPartResolution(QSize{img.width(), img.height()});
    if(newres.has_value())
    {
        //We can!
        QSize res = newres.value();
        QImage newimg = img.copy(0, 0, res.width(), res.height());
        parts = choppedimg_t{std::make_pair(newimg.rect(), newimg)}; //only a single part!
    }
    else
    {
        //We can't so try removing surrounding unused pixels
        sizeOfUsedPixels = FindSizeOfUsedPixels(img, img.rect());
        //Check if this matches any available resolutions
        auto matchedres = GetNextBestImgPartResolution(QSize{sizeOfUsedPixels.width(), sizeOfUsedPixels.height()});
        if(matchedres.has_value())
        {
            //Removing unused pixels works!
            QSize res = matchedres.value();
            //We chop it to size and call it a day!
            QImage newimg = img.copy(0, 0, res.width(), res.height());
            parts = choppedimg_t{std::make_pair(newimg.rect(), newimg)}; //only a single part!
        }
    }

    //We Couldn't do it in a single part
    if(parts.empty())
    {
        sizeOfUsedPixels;
    }

    return parts;
}

QVector<DialogImportWizard::choppedimg_t> DialogImportWizard::autochopImages(const QVector<QImage> &imgs)
{
    QVector<DialogImportWizard::choppedimg_t> chopped;
    for(const QImage & img : imgs)
        chopped.push_back(autochopImage(img));
    return chopped;
}

void DialogImportWizard::on_tblProperties_cellChanged(int /*row*/, int /*column*/)
{
    UpdateGizmo();
}
