#include "tvspritescontextmenu.hpp"
#include <QFileDialog>
#include <src/ui/mainwindow.hpp>
#include <src/utility/file_support.hpp>
#include <src/ui/errorhelper.hpp>

//===================================================================================================================
// TVSpritesContextMenu
//===================================================================================================================
TVSpritesContextMenu::TVSpritesContextMenu( MainWindow * mainwindow,
                                            const QModelIndex & item,
                                            QWidget * parent)
    :QMenu(parent)
{
    m_itemidx = item;
    m_pmainwindow = mainwindow;
    BuildMenu();
}

void TVSpritesContextMenu::BuildMenu()
{
    TreeNode * node = static_cast<TreeNode*>(m_itemidx.internalPointer());
    //Common default actions:
    switch(node->nodeDataTy())
    {
    case eTreeElemDataType::sprite:
        {
            addAction(tr("Properties"),     this, &TVSpritesContextMenu::ShowProperties);
            addAction(tr("Dump to file.."), this, &TVSpritesContextMenu::SaveDump);
            addAction(tr("Dump to xml.."),  this, &TVSpritesContextMenu::SaveXMLDump);
            break;
        }
    default:
        break;
    };

    if(node->nodeIsMutable())
    {
        addAction(tr("remove"), this, &TVSpritesContextMenu::RemoveEntry);
    }
}

void TVSpritesContextMenu::ShowProperties()
{
    Q_ASSERT(m_pmainwindow);
    m_pmainwindow->DisplayTabForElement(m_itemidx);
    closeAndDelete();
}

void TVSpritesContextMenu::SaveDump()
{
    ContentManager & manager = ContentManager::Instance();
    Q_ASSERT(m_pmainwindow);
    if(m_itemidx.isValid())
    {
        Sprite *    spr   = static_cast<Sprite*>(m_itemidx.internalPointer());
        MainWindow *pmain = m_pmainwindow;
        QString filename = QFileDialog::getSaveFileName(m_pmainwindow,
                                                        tr("Save selected as.."),
                                                        manager.getContainerParentDir(),
                                                        AllSupportedGameSpritesFileFilter());
        if(!filename.isNull())
        {
            spr->DumpSpriteToFile(filename);
            pmain->ShowStatusMessage(tr("Dumped!"));
        }
    }
    else
        ErrorHelper::getInstance().sendWarningMessage(tr("Couldn't dump invalid item!"));
    closeAndDelete();
}

void TVSpritesContextMenu::SaveXMLDump()
{
    ContentManager & manager = ContentManager::Instance();
    Q_ASSERT(m_pmainwindow);
    if(m_itemidx.isValid())
    {
        Sprite *    spr     = static_cast<Sprite*>(m_itemidx.internalPointer());
        QString filename = QFileDialog::getSaveFileName(m_pmainwindow,
                                                        tr("Save XML as.."),
                                                        manager.getContainerParentDir(),
                                                        AllSupportedXMLFileFilter());
        if(!filename.isNull())
        {
            spr->DumpSpriteToXML(filename);
            m_pmainwindow->ShowStatusMessage(tr("Dumped to xml and files!"));
        }
    }
    else
        ErrorHelper::getInstance().sendWarningMessage(tr("Couldn't dump invalid item!"));
    closeAndDelete();
}

void TVSpritesContextMenu::RemoveEntry()
{
    Q_ASSERT(m_pmainwindow);
    if(m_itemidx.isValid())
    {
        TreeNode * node = static_cast<TreeNode*>(m_itemidx.internalPointer());
        TreeNode * pparent = node->parentNode();
        if(pparent && ContentManager::Instance().isMultiItemContainer())
        {
            m_pmainwindow->HandleItemRemoval(m_itemidx);
        }
        else if(!pparent)
            ErrorHelper::getInstance().sendErrorMessage(tr("Entry to remove is invalid!"));
        if(!ContentManager::Instance().isMultiItemContainer())
            ErrorHelper::getInstance().sendErrorMessage(tr("Cannot delete the main item!"));
    }
    else
        ErrorHelper::getInstance().sendWarningMessage(tr("Couldn't remove invalid item!"));
    closeAndDelete();
}

void TVSpritesContextMenu::closeAndDelete()
{
    close();
    deleteLater();
}

void TVSpritesContextMenu::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit afterclosed();
}
