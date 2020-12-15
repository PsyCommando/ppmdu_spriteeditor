#include "autocropper_scene.hpp"
#include <src/ui/editor/cropper/autocropper_gizmo.hpp>
#include <QImage>
#include <QPainter>

AutoCropperScene::AutoCropperScene()
    :parent_t()
{
    m_pgizmo = new AutoCropperGizmo();
    addItem(m_pgizmo);
    m_pgizmo->setActive(false);
}

AutoCropperScene::~AutoCropperScene()
{
    //Scene will delete scene items automatically
}

void AutoCropperScene::Reset()
{
    if(m_pgizmo)
     {
        removeItem(m_pgizmo);
        delete m_pgizmo;
        m_pgizmo = nullptr;
    }
    m_sheet = QImage();
    clear();
    m_pgizmo = new AutoCropperGizmo();
    addItem(m_pgizmo);
    m_pgizmo->setActive(false);
}

void AutoCropperScene::setImage(const QImage &img)
{
    m_sheet = img;
    setSceneRect(m_sheet.rect());
    m_pgizmo->setMaxGizmoSize(m_sheet.rect());

}

const QImage &AutoCropperScene::getImage() const
{
    return m_sheet;
}

AutoCropperGizmo *AutoCropperScene::getGizmo()
{
    return m_pgizmo;
}

void AutoCropperScene::showGizmo(bool bshow)
{
    m_pgizmo->setActive(bshow);
}

void AutoCropperScene::drawBackground(QPainter *painter, const QRectF &/*rect*/)
{
    painter->drawImage(0,0,m_sheet);
}
