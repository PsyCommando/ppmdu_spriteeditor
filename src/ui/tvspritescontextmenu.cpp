#include "tvspritescontextmenu.hpp"
#include <QFileDialog>
#include <src/ui/mainwindow.hpp>

//===================================================================================================================
// TVSpritesContextMenu
//===================================================================================================================
TVSpritesContextMenu::TVSpritesContextMenu( MainWindow * mainwindow,
                                            const QModelIndex & item,
                                            QWidget * parent)
    :QMenu(parent)
{
    m_pitem = static_cast<TreeElement*>(item.internalPointer());
    m_itemidx = item;
    m_pmainwindow = mainwindow;
    BuildMenu();
}

void TVSpritesContextMenu::BuildMenu()
{

    //Common default actions:
    switch(m_pitem->getNodeDataTy())
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
    Q_ASSERT(m_pitem && m_pmainwindow && m_pitem->getNodeDataTy() == eTreeElemDataType::sprite);
    m_pmainwindow->DisplayPropertiesPage(static_cast<Sprite*>(m_pitem));
    close();
}

void TVSpritesContextMenu::SaveDump()
{
    Q_ASSERT(m_pitem && m_pmainwindow && m_itemidx.isValid());
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    QString filename = QFileDialog::getSaveFileName(m_pmainwindow,
                                                    tr("Save Sprite Dump As"),
                                                    QString(),
                                                    QString("%1;;%2")
                                                    .arg(m_pmainwindow->WanFileFilter())
                                                    .arg(m_pmainwindow->WatFileFilter()) );

    if(filename.isNull())
    {
        close();
        return;
    }

    sprman.DumpSprite(m_itemidx, filename);
    m_pmainwindow->ShowStatusMessage( QString(tr("Sprite dumped!")) );
    close();
}

void TVSpritesContextMenu::RemoveEntry()
{
    Q_ASSERT(m_pitem && m_pmainwindow && m_itemidx.isValid());
    TreeElement * pparent = m_pitem->parentNode();
    if(!pparent)
    {
        m_pmainwindow->ShowStatusErrorMessage( QString(tr("Entry to remove is invalid!")) );
        close();
        return;
    }
    if(m_pitem->getNodeDataTy() == eTreeElemDataType::sprite &&
       !spr_manager::SpriteManager::Instance().ContainerIsPackFile())
    {
        m_pmainwindow->ShowStatusErrorMessage( QString(tr("Cannot delete the main sprite!")) );
        close();
        return;
    }

    m_pmainwindow->HandleItemRemoval(m_itemidx);
    close();
}

void TVSpritesContextMenu::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit afterclosed();
}


