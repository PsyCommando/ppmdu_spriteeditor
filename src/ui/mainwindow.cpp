#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>
#include <QSpinBox>
#include <QTimer>
#include <QStackedLayout>
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/ui/tvspritescontextmenu.hpp>
#include <src/ui/errorhelper.hpp>
#include <src/data/sprite/sprite_container.hpp>
#include <src/data/sprite/spritemanager.hpp>
#include <src/utility/file_support.hpp>

static const QString MAIN_WINDOW_TITLE = "PMD2 Sprite Muncher v%1";


//Utiliy method to run a lambda on all BaseSpriteTab in the main stacked widget!
void MainWindow::forEachTab(std::function<void(BaseSpriteTab*)> fun)
{
    QStackedLayout * sl = static_cast<QStackedLayout*>(ui->stkEditor->layout());
    for(int i = 0; i < sl->count(); ++i)
    {
        BaseSpriteTab* curtab = dynamic_cast<BaseSpriteTab*>(sl->itemAt(i)->widget());
        if(curtab)
            fun(curtab);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings("./settings.ini",QSettings::Format::IniFormat)
{
    //Resources alloc
    m_pStatusFileType.reset(new QLabel("     "));
    setWindowTitle(MAIN_WINDOW_TITLE.arg(QString(GIT_MAJORMINOR_VERSION)));
    m_imgNoImg = QPixmap(":/imgs/resources/imgs/noimg.png");
    m_aboutdiag.setParent(this);
    m_progress.setParent(this);

    //Setup error helper
    ErrorHelper::getInstance().setMainWindow(this);

    //UI init
    ui->setupUi(this);

    //Setup references to main window in tabs
    forEachTab([this](BaseSpriteTab * ptab){ptab->setMainWindow(this);});

    //Parse settings!
    readSettings();

    //Undo-Redo
    ui->actionUndo = m_undoStack.createUndoAction(this, tr("&Undo"));
    ui->actionUndo->setShortcuts(QKeySequence::Undo);

    ui->actionRedo = m_undoStack.createRedoAction(this, tr("&Redo"));
    ui->actionRedo->setShortcuts(QKeySequence::Redo);

    //Main treeview
    ui->tv_sprcontent->setModel( &ContentManager::Instance() );
    ui->statusBar->addPermanentWidget(m_pStatusFileType.data());
    DisplayStartScreen();
}

MainWindow::~MainWindow()
{
    qDebug("MainWindow::~MainWindow()\n");

    //Gotta do some special handling to avoid things accessing sprites from breaking things
    forEachTab([](BaseSpriteTab * ptab){ptab->OnDestruction();});

    //Get rid of the ui
    delete ui;

    //Delete the sprite container before the Qt framework is unloaded!
    ContentManager::Instance().CloseContainer();

    //Clear error helper
    ErrorHelper::getInstance().setMainWindow(nullptr);
}

//
// Serialization for using settings
//
void MainWindow::writeSettings()
{
    m_settings.beginGroup("MainWindow");
    m_settings.setValue("size", size());
    m_settings.setValue("pos", pos());
    m_settings.endGroup();
}

void MainWindow::readSettings()
{
    m_settings.beginGroup("MainWindow");
    resize( m_settings.value("size", sizeHint() ).toSize());
    move( m_settings.value("pos", pos() ).toPoint());
    m_settings.endGroup();
}

void MainWindow::HideAllTabs()
{
    //Disable updates temporarily
    //ui->stkEditor->setUpdatesEnabled(false);

    //New tabs
    QWidget * pcurtab = ui->stkEditor->currentWidget();
    forEachTab([pcurtab](BaseSpriteTab * ptab){ if(pcurtab == ptab){ptab->OnHideTab();}});

    ui->stkEditor->setCurrentWidget(ui->tabEmpty);
    //ui->stkEditor->setUpdatesEnabled(true);
}

void MainWindow::ShowATab(BaseSpriteTab *ptab, const QModelIndex &element)
{
    Q_ASSERT(ptab);
    ShowATab(ptab);
    ptab->OnShowTab(element);
}

void MainWindow::ShowATab(QWidget *ptab)
{
    Q_ASSERT(ptab);
    qDebug() << "MainWindow::ShowATab(): Hiding tabs!\n";
    HideAllTabs();
    qDebug() << "MainWindow::ShowATab(): Displaying tab!\n";
    ui->stkEditor->setCurrentWidget(ptab);
    qDebug() << "MainWindow::ShowATab(): Tab displayed!\n";
    //ui->stkEditor->update(ui->stkEditor->rect());
}

void MainWindow::DisplayStartScreen()
{
    qDebug() << "MainWindow::DisplayStartScreen(): Showing start screen!\n";
    ShowATab(ui->tabWelcome);
    ShowStatusMessage(tr("Welcome! May you encounter no bugs today!"));
}

void MainWindow::SetupUIForNewContainer(BaseContainer * sprcnt)
{
    ContentManager & manager = ContentManager::Instance();
    ui->tv_sprcontent->setModel( & ContentManager::Instance() );
    ui->tv_sprcontent->setRootIndex(QModelIndex());
    connect(sprcnt, SIGNAL(showProgress(QFuture<void>&)), this, SLOT(ShowProgressDiag(QFuture<void>&)) );
    setupListView();
    updateActions();

    //Display!
    setSelectedTreeViewIndex(manager.index(0,0));
    DisplayTabForElement(m_curItem);

    ui->tv_sprcontent->repaint();
    setWindowFilePath(sprcnt->GetContainerSrcPath());
}

void MainWindow::DisplayTabForElement(const QModelIndex & index)
{
    if(!index.isValid())
        return;
    DisplayTabForElement(reinterpret_cast<TreeNode*>(index.internalPointer()));
}

void MainWindow::DisplayTabForElement(TreeNode * item)
{
    if(!item)
        return;
    ContentManager & manager = ContentManager::Instance();
    if(item->nodeCanFetchMore())
        item->nodeFetchMore();

    if(item->nodeDataTypeName() == ElemName_Sprite)
        ShowATab(ui->tabProperties, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_AnimTable)
        ShowATab(ui->tabAnimTable, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_AnimGroup)
        ShowATab(ui->tabAnimGroup, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_AnimSequences)
        ShowATab(ui->tabEmpty, QModelIndex());

    else if(item->nodeDataTypeName() == ElemName_AnimSequence)
        ShowATab(ui->tabSequence, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_FrameCnt)
        ShowATab(ui->tabEmpty, QModelIndex());

    else if(item->nodeDataTypeName() == ElemName_Frame)
        ShowATab(ui->tabFrame, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_Images)
        ShowATab(ui->tabImages, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_Image)
        ShowATab(ui->tabImages, manager.modelIndexOf(item));

    else if(item->nodeDataTypeName() == ElemName_EffectOffset)
        ShowATab(ui->tabEfx, manager.modelIndexOf(item));
    else
        HideAllTabs();
}

//Clear all selections to avoid warnings and  etc..
void MainWindow::PrepareForNewContainer()
{
    HideAllTabs();
    auto lambdaClearModel = [&](auto * pmod)
    {
        pmod->clearSelection();
        pmod->setModel(nullptr);
        pmod->reset();
    };

    lambdaClearModel(ui->tv_sprcontent);

    forEachTab([](BaseSpriteTab * ptab){ptab->PrepareForNewContainer();});
}

void MainWindow::LoadContainer(const QString &path)
{
    try
    {
        PrepareForNewContainer();

        //Pre-check if file exists
        if(QFile::exists(path))
        {
            ContentManager & manager = ContentManager::Instance();
            //Open
            qInfo() <<"MainWindow::LoadContainer() : " <<path <<"!\n";
            manager.OpenContainer(path);
            qInfo() <<"\nLoaded!\n";
            m_lastSavePath = path;
            SetupUIForNewContainer(manager.getContainer());
        }
        else
        {
            qInfo() << "Failed to open file \"" <<path <<"\"!";
        }
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::LoadContainer(): loading file \"" <<path <<"\" failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::SaveContainer(const QString &path)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        manager.SaveContainer(path);
        updateActions();
        if(manager.getContainer())
            setWindowFilePath(manager.getContainer()->GetContainerSrcPath());
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::SaveContainer(): Saving file \"" <<path <<"\" failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::SaveAs(const QString &path)
{
    try
    {
        ContentManager & sprman = ContentManager::Instance();
        if (!path.isEmpty())
        {
            qInfo() <<"Saving file " <<path <<"!\n";
            int wrotelen = sprman.SaveContainer(path);
            ShowStatusMessage( QString(tr("Wrote %1 bytes!")).arg(wrotelen) );
            qInfo() <<path <<" saved!\n";
            m_lastSavePath = path;
        }
        else
            qWarning() << "Got an empty path!\n";
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::SaveAs(): Saving file \"" <<path <<"\" failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::ExportContainer(const QString &path, const QString &exportType)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        manager.ExportContainer(path, exportType);
        updateActions();
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::ExportContainer(): Exporting file \"" <<path <<"\" as type \"" <<exportType <<"\" failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::ImportContainer(const QString &path)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        manager.ImportContainer(path);
        SetupUIForNewContainer(manager.getContainer());
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::ImportContainer(): Importing file \"" <<path <<"\" failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::CloseContainer()
{
    HideAllTabs();
    ContentManager::Instance().CloseContainer();
    DisplayStartScreen();
    setWindowFilePath(QString());
}

void MainWindow::updateActions()
{
    qInfo() <<"MainWindow::updateActions(): Updating!\n";
    ContentManager & manager = ContentManager::Instance();
    m_pStatusFileType->setText(manager.getContentShortName());

    if(manager.isContainerLoaded())
    {
        BaseContainer * cnt = manager.getContainer();
        if(cnt->isMultiItemContainer())
            addMultiItemActions(cnt->GetTopNodeName());
        else
            remMultiItemActions();
    }
}

void MainWindow::addMultiItemActions(const QString & itemname)
{
    m_pActionAddSprite.reset(ui->menu_Edit->addAction(QString("Add %1..").arg(itemname),      this, &MainWindow::OnActionAddTopItem));
    m_pActionRemSprite.reset(ui->menu_Edit->addAction(QString("Remove %1..").arg(itemname),   this, &MainWindow::OnActionRemTopItem));
}

void MainWindow::remMultiItemActions()
{
    ui->menu_Edit->removeAction(m_pActionAddSprite.data());
    ui->menu_Edit->removeAction(m_pActionRemSprite.data());
    m_pActionAddSprite.reset(nullptr);
    m_pActionRemSprite.reset(nullptr);
}

ContentManager & MainWindow::getManager()
{
    return ContentManager::Instance();
}

BaseContainer *MainWindow::getContainer()
{
    return ContentManager::Instance().getContainer();
}

void MainWindow::OnActionAddTopItem()
{
    try
    {
        qInfo() <<"MainWindow::OnActionAddSprite(): Adding sprite!\n";
        ContentManager & manager = ContentManager::Instance();

        //Insert at the end if nothing selected, or at the selected item otherwise
        TreeNode * topnode = manager.getOwnerNode(m_curItem);
        int insertpos = 0;
        if(!m_curItem.isValid() || !topnode)
            insertpos = manager.rowCount();
        else
            insertpos = topnode->nodeIndex();
        manager.insertRow(insertpos);
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::OnActionAddTopItem():Adding a new item failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::OnActionRemTopItem()
{
    try
    {
        if(!m_curItem.isValid())
        {
            Q_ASSERT(false);
            return;
        }
        qInfo() <<"MainWindow::OnActionRemSprite(): Removing sprite!\n";
        ContentManager & manager = ContentManager::Instance();

        //Insert at the end if nothing selected, or at the selected item otherwise
        TreeNode * topnode = manager.getOwnerNode(m_curItem);

        if(!topnode)
        {
            Q_ASSERT(false);
            return ;
        }
        manager.removeRow(topnode->nodeIndex());
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::OnActionAddTopItem():Adding a new item failed with exception: \'" << e.what() << "\'";
    }
}

QPixmap MainWindow::RenderNoImageSvg()
{
    QImage image(QString(":/imgs/resources/imgs/noimg.png"));
    return qMove(QPixmap::fromImage(image, Qt::ImageConversionFlag::ColorOnly | Qt::ImageConversionFlag::AvoidDither));
}

void MainWindow::on_action_Open_triggered()
{
    ContentManager & manager = ContentManager::Instance();

    //#TODO: Move that to constant!!
    QString filefilter;
    for(const auto & filters : SupportedFileFiltersByTypename)
    {
        if(filefilter.size() > 0)
            filefilter = QString("%1;;%2").arg(filefilter).arg(filters);
        else
            filefilter = QString("%1").arg(filefilter);
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), filefilter);

    if(fileName.isNull())
        return;

    //Check if a sprite was opened and ask to save changes!
    //DO OPEN FILE HERE!
    if( manager.isContainerLoaded() )
    {
        qDebug() << "MainWindow::on_action_Open_triggered(): Asking for saving changes!\n";
        //Ask to save or discard changes!!
        int choice = AskSaveChanges();
        switch(choice)
        {
        case QMessageBox::StandardButton::Save:
            this->on_action_Save_triggered();
            break;
        case QMessageBox::StandardButton::Cancel:
            return;
        };
    }
    LoadContainer(fileName);
    setupListView();
    updateActions();
}

void MainWindow::on_action_Quit_triggered()
{
    qDebug() <<"Quitting!\n";
    QApplication::instance()->exit();
    qDebug() <<"After exit call!\n";
}

void MainWindow::on_tv_sprcontent_expanded(const QModelIndex &index)
{
    try
    {
        TreeNode * pcur = static_cast<TreeNode*>(index.internalPointer());
        if(!pcur)
            return;

        //qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Expanding! Disabling updates!\n";
        //ui->tv_sprcontent->setUpdatesEnabled(false);
        //pcur->OnExpanded();
        //ui->tv_sprcontent->dataChanged(index, index); //Make sure we display expandable items properly if we just loaded them!
        //qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Changing current index!\n";
        //ui->tv_sprcontent->setCurrentIndex(index);

    //    ui->tv_sprcontent->setUpdatesEnabled(false);
    //    ui->tv_sprcontent->expand(index);
    //    ui->tv_sprcontent->setUpdatesEnabled(true);

        qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Opening proper tab!\n";
        m_curItem = m_curItem = index;
        DisplayTabForElement(index);
        //qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Enabling updates!\n";
        //ui->tv_sprcontent->setUpdatesEnabled(true);
        ui->tv_sprcontent->viewport()->update();
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::on_tv_sprcontent_expanded(): failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::on_action_Save_triggered()
{
    //QSaveFile;
    if(!m_lastSavePath.isEmpty())
        SaveAs(m_lastSavePath);
    else
        on_actionSave_As_triggered();
}

void MainWindow::on_actionSave_As_triggered()
{
    //QSaveFile;
    ContentManager & manager = ContentManager::Instance();
    const QString & cnttype = manager.getContainerType();

    QString filetypestr;
    if(!SupportedFileFiltersByTypename.contains(cnttype))
    {
        Warn(tr("Invalid Type!"), tr("The container you're trying to save doesn't seems to have a type somehow. Try changing the type of container."));
        Q_ASSERT(false);
        return;
    }
    filetypestr = SupportedFileFiltersByTypename[manager.getContainerType()];
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File As"), QString(), filetypestr);
    SaveAs(filename);
}

void MainWindow::on_action_About_triggered()
{
    m_aboutdiag.setModal(true);
    m_aboutdiag.show();
}

void MainWindow::on_actionNewSprite_triggered()
{
    try
    {
        Q_ASSERT(false);
    }
    catch (const std::exception & e)
    {
        qWarning() << "MainWindow::on_actionNewSprite_triggered(): failed with exception: \'" << e.what() << "\'";
    }
//    ContentManager & sprman = ContentManager::Instance();
//    BaseContainer * cnt = sprman.NewContainer();

//    if(cnt)
//        DisplayTabForElement(cnt->nodeChild(0));

//    //Display!
//    updateActions();
//    setupListView();
//    ShowStatusMessage(tr("New sprite!"));

}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    try
    {
        Q_ASSERT(false);
    }
    catch (const std::exception & e)
    {
        qWarning() << "MainWindow::on_actionNewSprite_Pack_File_triggered(): failed with exception: \'" << e.what() << "\'";
    }
    //ContentManager   & sprman = ContentManager::Instance();
    //SpriteContainer * sprcnt = sprman.NewContainer( SpriteContainer::eContainerType::PACK );

    //Display!
//    updateActions();
//    setupListView();
//    ShowStatusMessage(tr("New pack file!"));
}

void MainWindow::setupListView()
{
    ContentManager & manager = ContentManager::Instance();
    if(manager.isMultiItemContainer())
    {
        ui->tv_sprcontent->setRootIsDecorated(true);
        //ui->tv_sprcontent->expandToDepth(1);
        //ui->tv_sprcontent->viewport()->update();
        //ui->tv_sprcontent->collapseAll();
        ui->tv_sprcontent->viewport()->update();
    }
    else
    {
        ui->tv_sprcontent->setRootIsDecorated(true);
        ui->tv_sprcontent->collapseAll();
    }
}

int MainWindow::AskSaveChanges()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Icon::Question);
    msgBox.setText("Really close this file?");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    return msgBox.exec();
}

void MainWindow::PushUndoAction(QUndoCommand *cmd)
{
    Q_ASSERT(cmd);
    m_undoStack.push(cmd);
}

void MainWindow::Warn(const QString &title, const QString &text)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Icon::Warning);
    msgBox.setText(title);
    msgBox.setInformativeText(text);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::ShowStatusMessage(const QString &msg)
{
    ui->statusBar->showMessage( msg, 8000);
}

void MainWindow::ShowStatusErrorMessage(const QString &msg)
{
    QString errormsg = QString("<span style=\" font-size:9pt; font-weight:600; color:#aa0000;\">%1</span>").arg(msg);

    if(!m_pStatusError.isNull()) //don's spam!
        m_pStatusError->setText(errormsg);
    else
    {
        m_pStatusError.reset(new QLabel(errormsg,ui->statusBar) );
        ui->statusBar->addWidget(m_pStatusError.data());
        m_pStatusError->setTextFormat(Qt::TextFormat::RichText);
    }
    m_pStatusError->show();
    QApplication::beep();
    QTimer::singleShot( 8000, m_pStatusError.data(), &QLabel::hide );
}

void MainWindow::HandleItemRemoval(QModelIndex spriteidx)
{
    try
    {
        if(currentSpriteModelIndex() == spriteidx)
        {
            HideAllTabs(); //Deselect everything if we're deleting the current entry

            forEachTab([spriteidx](BaseSpriteTab * ptab){ptab->OnItemRemoval(spriteidx);});
            QModelIndex parentidx = ui->tv_sprcontent->currentIndex().parent();
            if(parentidx.isValid())
                ui->tv_sprcontent->setCurrentIndex(parentidx);
            else
                ui->tv_sprcontent->setCurrentIndex(QModelIndex());
        }
        ui->tv_sprcontent->model()->removeRow( spriteidx.row(), spriteidx.parent() );
        ShowStatusMessage( QString(tr("Entry removed!")) );
    }
    catch(const std::exception &e)
    {
        qWarning() << "MainWindow::HandleItemRemoval(): failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::on_tv_sprcontent_activated(const QModelIndex &index)
{
    ContentManager & manager = ContentManager::Instance();
    TreeNode * pcur = manager.getItem(index);
    if(!pcur)
        return;
    m_curItem = index;

    DisplayTabForElement(index);
    ui->tv_sprcontent->update();
}

void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    ContentManager & manager = ContentManager::Instance();
    TreeNode * pcur = manager.getItem(index);
    if(!pcur)
        return;
    m_curItem = index;
    DisplayTabForElement(index);

//    if(pcur->nodeShouldAutoExpand())
//        ui->tv_sprcontent->expand(index);

    ui->tv_sprcontent->update();
}

void MainWindow::on_tv_sprcontent_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex entry = ui->tv_sprcontent->indexAt(pos);
    if( entry.isValid() )
    {
        //This is triggered when clicking on an item
        //remove/copy/duplicate/etc
        QMenu * menu = makeSpriteContextMenu(entry);
        menu->popup(ui->tv_sprcontent->mapToGlobal(pos));
        //menu->show();
    }
    else
    {
        //This is triggered when clicking in the white unused space
        //Add sprite/frame/sequence/image/etc
    }
}

QMenu *MainWindow::makeSpriteContextMenu(QModelIndex entry)
{
    BaseContainer * pcnt = getContainer();
    Q_ASSERT(pcnt);
    TreeNode * elem = static_cast<TreeNode*>(entry.internalPointer());
    Q_ASSERT(elem);

    TVSpritesContextMenu * menu = new TVSpritesContextMenu( this, entry, ui->tv_sprcontent);
    connect( menu, SIGNAL(afterclosed()), menu, SLOT(deleteLater()) );
    return menu;
}

Sprite * MainWindow::currentSprite()
{
    ContentManager & manager = ContentManager::Instance();
    if(!manager.isContainerLoaded())
        return nullptr;

    if(!m_curItem.isValid())
        return nullptr;

    BaseContainer   * cnt = ContentManager::Instance().getContainer();
    Sprite          * spr = dynamic_cast<Sprite*>(cnt->getOwnerNode(m_curItem));
    return spr;
}

QModelIndex MainWindow::currentSpriteModelIndex()
{
    return m_curItem;
}

MFrame *MainWindow::currentFrame()
{
    TreeNode * elem = reinterpret_cast<TreeNode*>(ui->tv_sprcontent->currentIndex().internalPointer());

    //In cases where we're in the treeview on a child entry, get the parent!
    if(elem && elem->nodeDataTy() == eTreeElemDataType::framepart)
        return static_cast<MFrame*>(elem->parentNode());

    return static_cast<MFrame*>(elem);
}

Image *MainWindow::currentImage()
{
    Image * elem = reinterpret_cast<Image*>(ui->tv_sprcontent->currentIndex().internalPointer());
    return elem;
}

AnimSequence *MainWindow::currentAnimSequence()
{
    AnimSequence * elem = reinterpret_cast<AnimSequence*>(ui->tv_sprcontent->currentIndex().internalPointer());
    return elem;
}

eTreeElemDataType MainWindow::currentEntryType()
{
    TreeNode * elem = reinterpret_cast<TreeNode*>(ui->tv_sprcontent->currentIndex().internalPointer());
    if(elem)
        return elem->nodeDataTy();
    return eTreeElemDataType::None;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //        if (userReallyWantsToQuit())
    //        {
    writeSettings();
    event->accept();
    //        }
    //        else
    //        {
    //            event->ignore();
    //        }
}

void MainWindow::ShowProgressDiag(QFuture<void> &task)
{
    m_progress.setFuture(task);
    m_progress.setModal(true);
    m_progress.show();
}

void MainWindow::setSelectedTreeViewIndex(const QModelIndex & index)
{
    ui->tv_sprcontent->setCurrentIndex(index);
}
