#include "frame_offset_editor.hpp"
#include <QGraphicsView>


FrameOffsetEditor::FrameOffsetEditor(QObject * parent)
    :QObject(parent)
{
}

FrameOffsetEditor::~FrameOffsetEditor()
{

}

void FrameOffsetEditor::InstallPreview(QGraphicsView *pview, EffectSetModel *pmodel, const Sprite * spr, fmt::frmid_t frmid)
{
    if(!pview || !pmodel)
        Q_ASSERT(false);
    m_model = pmodel;
    m_sprite = spr;
    pview->setScene(&getScene());
    pview->setBackgroundBrush(QBrush(getSpriteBGColor()));
    setFrameId(frmid);
    if(m_frameItem)
        pview->ensureVisible(m_frameItem, 8, 8);
}

QColor FrameOffsetEditor::getSpriteBGColor() const
{
    if(m_sprite && !m_sprite->getPalette().empty())
        return m_sprite->getPalette().first();
    return QColor(220,220,220);
}

void FrameOffsetEditor::Clear()
{
    m_frmid = -1;
    m_scene.removeItem(m_frameItem);

    for(auto * p : m_posMarkers)
        m_scene.removeItem(p);
//    m_scene.removeItem(m_posHead);
//    m_scene.removeItem(m_posRHand);
//    m_scene.removeItem(m_posLHand);
//    m_scene.removeItem(m_posCenter);
}

void FrameOffsetEditor::Update()
{
    const TreeNode * set = m_model->getRootNode();
    Q_ASSERT(m_posMarkers.size() == set->nodeChildCount()); //If we got suddenly more or less offsets per groups something really bad is up
    for(int i = 0; i < set->nodeChildCount(); ++i)
    {
         const EffectOffset * off = static_cast<const EffectOffset *>(set->nodeChild(i));
         m_posMarkers[i]->setPos(off->getX(), off->getY());
         if(i == m_selOffset.row())
             m_posMarkers[i]->setDefaultTextColor(QColor(127,127,0));
         else
             m_posMarkers[i]->setDefaultTextColor(QColor(255,0,0));
    }
    if(m_frameItem && m_frmid != -1)
    {
        const MFrame * frm = m_sprite->getFrame(m_frmid);
        Q_ASSERT(frm);
        m_frameItem->setPixmap(QPixmap::fromImage(frm->AssembleFrame(0,0,QRect(), nullptr, true, const_cast<Sprite*>(m_sprite))));
    }
}

void FrameOffsetEditor::setFrameId(fmt::frmid_t id)
{
    Clear();
    m_frmid = id;
    if(m_frmid == -1)
        return;
    const MFrame * frm = m_sprite->getFrame(m_frmid);
    Q_ASSERT(frm);
    m_frameItem = m_scene.addPixmap(QPixmap::fromImage(frm->AssembleFrame(0,0,QRect(), nullptr, true, const_cast<Sprite*>(m_sprite))));

    m_scene.setForegroundBrush(QBrush(QColor(255,0,0)));
    for(int i = 0; i < m_model->getRootNode()->nodeChildCount(); ++i)
        m_posMarkers.push_back(m_scene.addText("+")); //Just create the items, they'll get moved in place during the update
    //
    Update();
}

fmt::frmid_t FrameOffsetEditor::getFrameId() const
{
    return m_frmid;
}

void FrameOffsetEditor::UninstallPreview(QGraphicsView *pview)
{
    Clear();
    pview->setScene(nullptr);
    pview->invalidateScene();
}

void FrameOffsetEditor::setSelected(const QModelIndex &index)
{
    m_selOffset = index;
    Update();
}

QModelIndex FrameOffsetEditor::getSelected() const
{
    return m_selOffset;
}
