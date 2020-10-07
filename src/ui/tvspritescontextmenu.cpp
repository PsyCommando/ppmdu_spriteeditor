#include "tvspritescontextmenu.hpp"
#include <QFileDialog>
#include <src/ui/mainwindow.hpp>
#include <src/utility/file_support.hpp>

//===================================================================================================================
// TVSpritesContextMenu
//===================================================================================================================
TVSpritesContextMenu::TVSpritesContextMenu( MainWindow * mainwindow,
                                            const QModelIndex & item,
                                            QWidget * parent)
    :QMenu(parent)
{
    m_pitem = static_cast<TreeNode*>(item.internalPointer());
    m_itemidx = item;
    m_pmainwindow = mainwindow;
    BuildMenu();
}

void TVSpritesContextMenu::BuildMenu()
{

    //Common default actions:
    switch(m_pitem->nodeDataTy())
    {
    case eTreeElemDataType::sprite:
        {
            addAction(tr("properties"),   this, &TVSpritesContextMenu::ShowProperties);
            addAction(tr("dump.."),       this, &TVSpritesContextMenu::SaveDump);
            break;
        }
    default:
        break;
    };

    if(m_pitem->nodeIsMutable())
    {
        addAction(tr("remove"), this, &TVSpritesContextMenu::RemoveEntry);
    }

}

void TVSpritesContextMenu::ShowProperties()
{
    Q_ASSERT(m_pitem && m_pmainwindow && m_pitem->nodeDataTy() == eTreeElemDataType::sprite);
    m_pmainwindow->DisplayTabForElement(m_itemidx);
    close();
}

void TVSpritesContextMenu::SaveDump()
{
//    Q_ASSERT(m_pitem && m_pmainwindow && m_itemidx.isValid());
//    ContentManager & sprman = ContentManager::Instance();
//    QString filename = QFileDialog::getSaveFileName(m_pmainwindow,
//                                                    tr("Save Sprite Dump As"),
//                                                    QString(),
//                                                    QString("%1;;%2")
//                                                    .arg(SupportedFileFiltersByTypename[FileExtWAN])
//                                                    .arg(SupportedFileFiltersByTypename[FileExtWAT]) );

//    if(filename.isNull())
//    {
//        close();
//        return;
//    }

//    DumpSprite(m_itemidx, filename);
//    m_pmainwindow->ShowStatusMessage( QString(tr("Sprite dumped!")) );
//    close();

    Q_ASSERT(false);
}

void TVSpritesContextMenu::RemoveEntry()
{
//    Q_ASSERT(m_pitem && m_pmainwindow && m_itemidx.isValid());
//    TreeNode * pparent = m_pitem->parentNode();
//    if(!pparent)
//    {
//        m_pmainwindow->ShowStatusErrorMessage( QString(tr("Entry to remove is invalid!")) );
//        close();
//        return;
//    }
//    if(m_pitem->nodeDataTy() == eTreeElemDataType::sprite &&
//       !SpriteManager::Instance().ContainerIsPackFile())
//    {
//        m_pmainwindow->ShowStatusErrorMessage( QString(tr("Cannot delete the main sprite!")) );
//        close();
//        return;
//    }

//    m_pmainwindow->HandleItemRemoval(m_itemidx);
//    close();
    Q_ASSERT(false);
}

void TVSpritesContextMenu::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit afterclosed();
}


