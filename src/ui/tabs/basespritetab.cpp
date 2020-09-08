#include "basespritetab.hpp"
#include <src/sprite.hpp>
#include <src/ui/mainwindow.hpp>

BaseSpriteTab::BaseSpriteTab(QWidget *parent): QWidget(parent), m_pmainwindow(nullptr){}

BaseSpriteTab::~BaseSpriteTab(){}

void BaseSpriteTab::setMainWindow(MainWindow *parentwindow)
{
    m_pmainwindow = parentwindow;
}

MainWindow *BaseSpriteTab::getMainWindow()
{
    return m_pmainwindow;
}

void BaseSpriteTab::OnShowTab(Sprite *, QPersistentModelIndex)
{
}

void BaseSpriteTab::OnHideTab()
{
}

void BaseSpriteTab::OnDestruction()
{
}

void BaseSpriteTab::ShowStatusErrorMessage(QString msg)
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        m_pmainwindow->ShowStatusErrorMessage(msg);
}

void BaseSpriteTab::ShowStatusMessage(QString msg)
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        m_pmainwindow->ShowStatusMessage(msg);
}

void BaseSpriteTab::Warn(const QString &title, const QString &text)
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        m_pmainwindow->Warn(title, text);
}

Sprite *BaseSpriteTab::currentSprite()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentSprite();
    return nullptr;
}

MFrame *BaseSpriteTab::currentFrame()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentFrame();
    return nullptr;
}

Image *BaseSpriteTab::currentImage()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentImage();
    return nullptr;
}

AnimSequence *BaseSpriteTab::currentAnimSequence()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentAnimSequence();
    return nullptr;
}

eTreeElemDataType BaseSpriteTab::currentEntryType()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentEntryType();
    return eTreeElemDataType::None;
}

MFramePart *BaseSpriteTab::currentTblFrameParts()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentTblFrameParts();
    return nullptr;
}

Image *BaseSpriteTab::currentTblImages()
{
    Q_ASSERT(m_pmainwindow);
    if(m_pmainwindow)
        return m_pmainwindow->currentTblImages();
    return nullptr;
}
