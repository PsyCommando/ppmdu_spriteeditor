#ifndef AUTOCROPPERGIZMO_HPP
#define AUTOCROPPERGIZMO_HPP

#include <QGraphicsItem>

enum struct eCellsOrdering : int
{
    LeftToRight,
    RightToLeft,
    UpToDown,
    DownToUp,
};


//Item that makes up a sequence
class SequenceGizmo;
class CellGizmo : public QGraphicsItem
{
    using parent_t = QGraphicsItem;
public:
    explicit CellGizmo(SequenceGizmo * parent);

    int getCellWidth()const;
    void setCellWidth(int w);

    int getCellHeight()const;
    void setCellHeight(int h);

    //Returns a rectangle with the area this cell contains
    QRectF getCroppedArea()const;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QRectF m_bounds;
};

//Item made up of cells
class AutoCropperGizmo;
class SequenceGizmo : public QGraphicsItem
{
    using parent_t = QGraphicsItem;
public:
    explicit SequenceGizmo(AutoCropperGizmo * parent);

    void UpdateGizmo();

    void setNbCellsV(int vcells);
    int getNbCellsV()const;

    void setNbCellsH(int vcells);
    int getNbCellsH()const;

    int getCellMarginH() const;
    void setCellMarginH(int margin);

    int getCellMarginV() const;
    void setCellMarginV(int margin);

    int getCellWidth()const;
    void setCellWidth(int w);

    int getCellHeight()const;
    void setCellHeight(int h);

    QVector<QRectF> GetCellsRects(eCellsOrdering vorder = eCellsOrdering::UpToDown, eCellsOrdering horder = eCellsOrdering::LeftToRight)const;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QVector<QVector<CellGizmo*>> m_cells;
    int m_cellMarginH{0};   //The margin between each cells within a sequence horizontally
    int m_cellMarginV{0};   //The margin between each cells within a sequence vertically
    int m_cellWidht{1};     //The Width of a single cell
    int m_cellHeight{1};    //The Height of a single cell
};

//
// Graphic item meant to indicate the way the animation sheet will be cropped
class AutoCropperGizmo : public QGraphicsItem
{
    using parent_t = QGraphicsItem;
public:
    AutoCropperGizmo();

    void UpdateGizmo();

public:
    int cellWidht() const;
    void setCellWidht(int cellWidht);
    int cellHeight() const;
    void setCellHeight(int cellHeight);
    int cellMarginH() const;
    void setCellMarginH(int cellMarginH);
    int cellMarginV() const;
    void setCellMarginV(int cellMarginV);
    int nbSeqV() const;
    void setNbSeqV(int nbSeqV);
    int nbSeqH() const;
    void setNbSeqH(int nbSeqH);
    int nbCellsPerSeqH() const;
    void setNbCellsPerSeqH(int nbCellsPerSeqH);
    int nbCellsPerSeqV() const;
    void setNbCellsPerSeqV(int nbCellsPerSeqV);
    int seqMarginV() const;
    void setSeqMarginV(int seqMarginV);
    int seqMarginH() const;
    void setSeqMarginH(int seqMarginH);

    void setMaxGizmoSize(QRectF sz);
    const QRectF &getMaxGizmoSize()const;

    QVector<QVector<QRectF>> GetAllCellsRects()const;

private:
    int m_cellWidht{1};     //The Width of a single cell
    int m_cellHeight{1};    //The Height of a single cell
    int m_cellMarginH{0};   //The margin between each cells within a sequence horizontally
    int m_cellMarginV{0};   //The margin between each cells within a sequence vertically

    int m_nbSeqV{0}; //The amount of animation sequences in total vertically
    int m_nbSeqH{0}; //The amount of animation sequences in total horizontally
    int m_nbCellsPerSeqH{1}; //The amount of cells within a single animation sequence horizontally
    int m_nbCellsPerSeqV{1}; //The amount of cells within a single animation sequence verically
    int m_seqMarginV{0};    //Vertical margin between 2 vertical animation sequences
    int m_seqMarginH{0};    //Horizontal margin between 2 horizontal animation sequences

    QRectF m_maxSize;
    //X, Y of the cropping gizmo are stored in the graphics item itself


    QVector<QVector<SequenceGizmo*>> m_SeqGizmos; //Matrix of sequence gizmos in use
    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

};

#endif // AUTOCROPPERGIZMO_HPP
