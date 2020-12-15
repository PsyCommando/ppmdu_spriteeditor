#include "autocropper_gizmo.hpp"

#include <QPainter>

//
//
//
CellGizmo::CellGizmo(SequenceGizmo *parent)
    :parent_t(parent)
{
}

int CellGizmo::getCellWidth() const
{
    return qRound(m_bounds.width());
}

void CellGizmo::setCellWidth(int w)
{
    m_bounds.setWidth(w);
}

int CellGizmo::getCellHeight() const
{
    return qRound(m_bounds.height());
}

void CellGizmo::setCellHeight(int h)
{
    m_bounds.setHeight(h);
}

QRectF CellGizmo::getCroppedArea() const
{
    return m_bounds;
}

QRectF CellGizmo::boundingRect() const
{
    return m_bounds.marginsAdded(QMarginsF(1,1,1,1)); //Add pen width
}

void CellGizmo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Draw the cell's bounds
    QBrush br(QColor("Orange"));
    QPen   pn(br, 1, Qt::PenStyle::DashLine, Qt::PenCapStyle::SquareCap, Qt::PenJoinStyle::MiterJoin);
    painter->setPen(pn);
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->drawRect(0,0, m_bounds.width(), m_bounds.height());
    //parent_t::paint(painter, option, widget);
}


//
//
//
SequenceGizmo::SequenceGizmo(AutoCropperGizmo *parent)
    :parent_t(parent)
{
}

void SequenceGizmo::UpdateGizmo()
{
    QPointF posLast(0,0); //Position of the last cell gizmo we've handled so far
    for(QVector<CellGizmo*> & vc : m_cells)
    {
        posLast.setX(0); //Reset to 0 for each rows
        posLast.setY(posLast.y() + m_cellMarginV); //Increment last Y position by the margin
        //Go through all the cells on the x axis
        for(int i = 0; i < vc.size(); ++i)
        {
            posLast.setX(posLast.x() + m_cellMarginH); //Increment last X position by the margin
            if(!(vc[i]))
                vc[i] = new CellGizmo(this); //Make sure we're not missing anything
            vc[i]->setCellHeight(m_cellHeight);
            vc[i]->setCellWidth(m_cellWidht);
            vc[i]->setPos(posLast);
            posLast.setX(posLast.x() + vc[i]->boundingRect().width()); //Add cell width
        }
    }
}

void SequenceGizmo::setNbCellsV(int vcells)
{
    if(vcells < m_cells.size())
    {
        for(int i = vcells; i < m_cells.size(); ++i)
            qDeleteAll(m_cells[i]);
    }
    m_cells.resize(vcells);
    UpdateGizmo();
}

int SequenceGizmo::getNbCellsV() const
{
    return m_cells.size();
}

void SequenceGizmo::setNbCellsH(int hcells)
{
    if(m_cells.empty())
        return;
    if(hcells < m_cells.front().size())
    {
        for(auto & v : m_cells)
        {
            for(int j = hcells; j < v.size(); ++j)
                delete v[j];
            v.resize(hcells);
        }
    }
    UpdateGizmo();
}

int SequenceGizmo::getNbCellsH() const
{
    return m_cells.empty()? 0 : m_cells.front().size();
}

QRectF SequenceGizmo::boundingRect() const
{
    if(m_cells.empty() || m_cells.front().empty() || !(m_cells.front().front()))
        return QRectF();
    int nbcellsV = m_cells.size();
    int nbcellsH = m_cells.front().size();
    QRectF cellSz = m_cells.front().front()->boundingRect();

    int w = qRound(nbcellsH * (m_cellMarginH + cellSz.width())) + 2 + m_cellMarginH; //2 is total pen stroke width for both sides drawn on x axis, and we got one extra cell margin at the end
    int h = qRound(nbcellsV * (m_cellMarginV + cellSz.height())) + 2 + m_cellMarginV; //2 is total pen stroke width for both sides drawn on y axis, and we got one extra cell margin at the end
    return QRectF(0,0, w,h);
}

void SequenceGizmo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Draw the box around the cells
    QBrush br(QColor("Yellow"));
    QPen   pn(br, 1, Qt::PenStyle::DashLine, Qt::PenCapStyle::SquareCap, Qt::PenJoinStyle::MiterJoin);
    painter->setPen(pn);
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->drawRect(boundingRect());
    //parent_t::paint(painter, option, widget);
}

int SequenceGizmo::getCellMarginH() const
{
    return m_cellMarginH;
}

void SequenceGizmo::setCellMarginH(int cellMarginH)
{
    m_cellMarginH = cellMarginH;
}

int SequenceGizmo::getCellMarginV() const
{
    return m_cellMarginV;
}

void SequenceGizmo::setCellMarginV(int cellMarginV)
{
    m_cellMarginV = cellMarginV;
}

int SequenceGizmo::getCellWidth() const
{
    return m_cellWidht;
}

void SequenceGizmo::setCellWidth(int w)
{
    m_cellWidht = w;
}

int SequenceGizmo::getCellHeight() const
{
    return m_cellHeight;
}

void SequenceGizmo::setCellHeight(int h)
{
    m_cellHeight = h;
}

QVector<QRectF> SequenceGizmo::GetCellsRects(eCellsOrdering vorder, eCellsOrdering horder) const
{
    QVector<QRectF> rects;
    rects.reserve(getNbCellsH() * getNbCellsV());

    int i = 0, j = 0;
    for((vorder == eCellsOrdering::UpToDown)? i = 0             : i = (m_cells.size() - 1);
        (vorder == eCellsOrdering::UpToDown)? i < m_cells.size(): i >= 0 ;
        (vorder == eCellsOrdering::UpToDown)? ++i               : --i )
    {
        const QVector<CellGizmo*> & curcells = m_cells[i];
        for((horder == eCellsOrdering::RightToLeft)? j = 0              : j = curcells.size() - 1;
            (horder == eCellsOrdering::RightToLeft)? j < curcells.size(): j >= 0;
            (horder == eCellsOrdering::RightToLeft)? ++j                : --j)
        {
            rects.push_back(curcells[j]->getCroppedArea());
        }
    }
    return rects;
}


//
//
//
AutoCropperGizmo::AutoCropperGizmo()
    :parent_t()
{

}

void AutoCropperGizmo::UpdateGizmo()
{
    QPointF posLast(0,0); //Position of the last seq gizmo we've handled so far
    m_SeqGizmos.resize(m_nbSeqV);
    for(QVector<SequenceGizmo *> & v : m_SeqGizmos)
    {
        posLast.setX(0); //Reset to 0 for each rows
        posLast.setY(posLast.y() + m_seqMarginV); //Increment last Y position by the margin
        v.resize(m_nbSeqH);
        //Go through all the sequence gizmo on the x axis
        for(int i = 0; i < v.size(); ++i)
        {
            posLast.setX(posLast.x() + m_seqMarginH); //Increment last X position by the margin
            if(!(v[i]))
                v[i] = new SequenceGizmo(this);
            v[i]->setCellHeight(m_cellHeight);
            v[i]->setCellWidth(m_cellWidht);
            v[i]->setCellMarginH(m_cellMarginH);
            v[i]->setCellMarginV(m_cellMarginV);
            v[i]->setNbCellsH(m_nbCellsPerSeqH);
            v[i]->setNbCellsV(m_nbCellsPerSeqV);
            v[i]->UpdateGizmo();
            v[i]->setPos(posLast);
            posLast.setX(posLast.x() + v[i]->boundingRect().width()); //Add sequence width
        }
    }
}

int AutoCropperGizmo::seqMarginH() const
{
    return m_seqMarginH;
}

void AutoCropperGizmo::setSeqMarginH(int seqMarginH)
{
    m_seqMarginH = seqMarginH;
}

void AutoCropperGizmo::setMaxGizmoSize(QRectF sz)
{
    m_maxSize = sz;
    //#TODO: do a resize check
}

const QRectF &AutoCropperGizmo::getMaxGizmoSize() const
{
    return m_maxSize;
}

QVector<QVector<QRectF> > AutoCropperGizmo::GetAllCellsRects() const
{
    QVector<QVector<QRectF> > rects;
    rects.resize(nbSeqV() * nbSeqH());
    for(const QVector<SequenceGizmo*> & seqs : m_SeqGizmos)
    {
        for(const SequenceGizmo * seq : seqs)
            rects.push_back(seq->GetCellsRects());
    }
    return rects;
}

int AutoCropperGizmo::seqMarginV() const
{
    return m_seqMarginV;
}

void AutoCropperGizmo::setSeqMarginV(int seqMarginV)
{
    m_seqMarginV = seqMarginV;
}

int AutoCropperGizmo::nbCellsPerSeqV() const
{
    return m_nbCellsPerSeqV;
}

void AutoCropperGizmo::setNbCellsPerSeqV(int nbCellsPerSeqV)
{
    m_nbCellsPerSeqV = nbCellsPerSeqV;
}

int AutoCropperGizmo::nbCellsPerSeqH() const
{
    return m_nbCellsPerSeqH;
}

void AutoCropperGizmo::setNbCellsPerSeqH(int nbCellsPerSeqH)
{
    m_nbCellsPerSeqH = nbCellsPerSeqH;
}

int AutoCropperGizmo::nbSeqH() const
{
    return m_nbSeqH;
}

void AutoCropperGizmo::setNbSeqH(int nbSeqH)
{
    m_nbSeqH = nbSeqH;
}

int AutoCropperGizmo::nbSeqV() const
{
    return m_nbSeqV;
}

void AutoCropperGizmo::setNbSeqV(int nbSeqV)
{
    m_nbSeqV = nbSeqV;
}

int AutoCropperGizmo::cellMarginV() const
{
    return m_cellMarginV;
}

void AutoCropperGizmo::setCellMarginV(int cellMarginV)
{
    m_cellMarginV = cellMarginV;
}

int AutoCropperGizmo::cellMarginH() const
{
    return m_cellMarginH;
}

void AutoCropperGizmo::setCellMarginH(int cellMarginH)
{
    m_cellMarginH = cellMarginH;
}

int AutoCropperGizmo::cellHeight() const
{
    return m_cellHeight;
}

void AutoCropperGizmo::setCellHeight(int cellHeight)
{
    m_cellHeight = cellHeight;
}

int AutoCropperGizmo::cellWidht() const
{
    return m_cellWidht;
}

void AutoCropperGizmo::setCellWidht(int cellWidht)
{
    m_cellWidht = cellWidht;
}

QRectF AutoCropperGizmo::boundingRect() const
{
    if(m_SeqGizmos.empty() || m_SeqGizmos.front().empty() || !(m_SeqGizmos.front().front()))
        return QRectF();
    int nbSeqV = m_SeqGizmos.size();
    int nbSeqH = m_SeqGizmos.front().size();
    QRectF seqSz = m_SeqGizmos.front().front()->boundingRect();
    int w = qRound(nbSeqH * (m_seqMarginH + seqSz.width())) + 2 + m_seqMarginH;
    int h = qRound(nbSeqV * (m_seqMarginV + seqSz.height())) + 2 + m_seqMarginV;
    return QRectF(0,0, w, h);
}

void AutoCropperGizmo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //Draw the box around the cells
    QBrush br(QColor("Pink"));
    QPen   pn(br, 1, Qt::PenStyle::DashLine, Qt::PenCapStyle::SquareCap, Qt::PenJoinStyle::MiterJoin);
    painter->setPen(pn);
    painter->setCompositionMode(QPainter::CompositionMode_Multiply);
    painter->drawRect(boundingRect());
    //parent_t::paint(painter, option, widget);
}
