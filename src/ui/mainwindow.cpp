#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <functional>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>
#include <QSpinBox>
#include <QTimer>
#include <QStackedLayout>
#include <src/ui/editor/palette/paletteeditor.hpp>
#include <src/ui/tvspritescontextmenu.hpp>
#include <src/ui/errorhelper.hpp>
#include <src/utility/program_settings.hpp>
#include <src/utility/file_support.hpp>
#include <src/ui/windows/dialog_settings.hpp>
#include <src/ui/windows/dialogimportwizard.hpp>
#include <src/ui/windows/dialogabout.hpp>
#include <src/ui/windows/dialognew.hpp>
#include <src/ui/windows/diagsingleimgcropper.hpp>
#include <src/data/sprite/sprite_container.hpp>
#include <src/data/contents_selection_manager.hpp>
#include <src/data/sprite/unknown_item.hpp>
#include <src/utility/ui_helpers.hpp>

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
    m_aboutdiag(this),
    m_progress(this)
{
    //UI init
    ui->setupUi(this);

    SetupContent();
    SetupMenu();
    SetupStatusBar();
    DisplayStartScreen();

    //Parse settings!
    ProgramSettings::Instance().ReadSettings(objectName(), std::bind(&MainWindow::readSettings, this, std::placeholders::_1) );
}

void MainWindow::SetupContent()
{
    setWindowTitle(QString("%1 v%2").arg(QApplication::applicationName(), QString(GIT_MAJORMINOR_VERSION)));
    m_imgNoImg = QPixmap(":/imgs/resources/imgs/noimg.png");

    //Setup error helper
    ErrorHelper::getInstance().setMainWindow(this);

    //Setup references to main window in tabs
    forEachTab([this](BaseSpriteTab * ptab){ptab->setMainWindow(this);});

    //Main treeview
    ui->tv_sprcontent->setModel(&ContentManager::Instance());
    ui->tv_sprcontent->setSelectionModel(ContentsSelectionManager::SelectionModel());

    //Signals
    connect(&ContentManager::Instance(), &ContentManager::contentChanged,       this, &MainWindow::updateCurrentTab);
    connect(&ContentManager::Instance(), &ContentManager::rowsAboutToBeRemoved, this, &MainWindow::OnContentRowsAboutToBeRemoved);
    connect(&ContentManager::Instance(), &ContentManager::rowsAboutToBeMoved,   this, &MainWindow::OnContentRowsAboutToBeMoved);
    connect(&ContentManager::Instance(), &ContentManager::rowsInserted,         this, &MainWindow::OnRowInserted);
}

void MainWindow::SetupMenu()
{
    //Undo-Redo
    ui->actionUndo = m_undoStack.createUndoAction(this, tr("&Undo"));
    ui->actionUndo->setShortcuts(QKeySequence::Undo);

    ui->actionRedo = m_undoStack.createRedoAction(this, tr("&Redo"));
    ui->actionRedo->setShortcuts(QKeySequence::Redo);
}

void MainWindow::SetupStatusBar()
{
    //Status bar
    QSizePolicy szp(QSizePolicy::Fixed, QSizePolicy::Expanding);
    szp.setHorizontalStretch(0);
    szp.setVerticalStretch(0);

    //Coordinate widget
    m_pStatusCoordinates.reset(new QLabel(""));
    m_pStatusCoordinates->setMinimumSize(96, 24);
    m_pStatusCoordinates->setSizePolicy(szp);
    ui->statusBar->addPermanentWidget(m_pStatusCoordinates.data());

    //File type indicator
    m_pStatusFileType.reset(new QLabel(""));
    m_pStatusFileType->setMinimumSize(48, 24);
    m_pStatusFileType->setSizePolicy(szp);
    ui->statusBar->addPermanentWidget(m_pStatusFileType.data());
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
void MainWindow::writeSettings(QSettings & settings)
{
    //Main window state
    settings.setValue("size",   size());
    settings.setValue("pos",    pos());

    //Do tabs
    forEachTab([](BaseSpriteTab * ptab)
    {
        ProgramSettings::Instance().WriteSettings(ptab->objectName(), [ptab](QSettings &){ptab->writeSettings();});
    });
}

void MainWindow::readSettings(QSettings & settings)
{
    resize  (settings.value("size", sizeHint()) .toSize());
    move    (settings.value("pos",  pos())      .toPoint());

    //Do tabs
    forEachTab([](BaseSpriteTab * ptab)
    {
         ProgramSettings::Instance().ReadSettings(ptab->objectName(), [ptab](QSettings &){ptab->readSettings();});
    });
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
    ContentsSelectionManager::Instance().resetModel();
    ContentManager & manager = ContentManager::Instance();
    ui->tv_sprcontent->setModel(&ContentManager::Instance());
    ui->tv_sprcontent->setSelectionModel(ContentsSelectionManager::SelectionModel());
    ui->tv_sprcontent->setRootIndex(QModelIndex());
    connect(sprcnt, SIGNAL(showProgress(QFuture<void>&)), this, SLOT(ShowProgressDiag(QFuture<void>&)) );
    setupListView();
    updateActions();

    //Display!
    if(manager.getContainer()->nodeChildCount() > 0)
    {
        selectTreeViewNode(manager.index(0,0));
        DisplayTabForElement(m_curItem);
    }

    ui->tv_sprcontent->update();
    ui->tv_sprcontent->repaint();
    setWindowTitle(QString());
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

    //#TODO: Maybe automate this?
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

    else if(item->nodeDataTypeName() == ElemName_UnknownItem)
        ShowATab(ui->tabUnknown, manager.modelIndexOf(item));
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
            UpdateFileDialogProjectPath(manager.getContainerParentDir());
            m_lastSavePath = path;
            SetupUIForNewContainer(manager.getContainer());
        }
        else
        {
            Warn(QString("Couldn't open \"`%1\"!").arg(path), QString("File \"%1\" does not exists!").arg(path));
            qWarning() << "File \"" <<path <<"\" does not exists!";
        }
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn(QString("Couldn't open \"`%1\"!").arg(path), QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::LoadContainer(): loading file \"" <<path <<"\" failed with exception: " << excepttext;
        PrepareForNewContainer(); //Cleanup on failure
        DisplayStartScreen();
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
        {
            setWindowFilePath(manager.getContainer()->GetContainerSrcPath());
            UpdateFileDialogProjectPath(manager.getContainerParentDir());
        }
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn(QString("Couldn't save \"`%1\"!").arg(path), QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::SaveContainer(): Saving file \"" <<path <<"\" failed with exception: \'" << excepttext << "\'";
    }
}

void MainWindow::SaveAs(const QString &path)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        if (!path.isEmpty())
        {
            qInfo() <<"Saving file " <<path <<"!\n";
            int wrotelen = manager.SaveContainer(path);
            ShowStatusMessage( QString(tr("Wrote %1 bytes!")).arg(wrotelen) );
            qInfo() <<path <<" saved!\n";
            m_lastSavePath = path;
            UpdateFileDialogProjectPath(manager.getContainerParentDir());
        }
        else
            qWarning() << "Got an empty path!\n";
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn(QString("Couldn't save \"`%1\"!").arg(path), QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::SaveAs(): Saving file \"" <<path <<"\" failed with exception: \'" << excepttext << "\'";
    }
}

void MainWindow::ExportContainer(const QString &path, const QString &exportType)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        manager.ExportContainer(path, exportType);
        updateActions();
        QFileInfo finf(path);
        UpdateFileDialogExportPath(finf.dir().absolutePath());
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn(QString("Couldn't export \"`%1\"!").arg(path), QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::ExportContainer(): Exporting file \"" <<path <<"\" as type \"" <<exportType <<"\" failed with exception: \'" << excepttext << "\'";
    }
}

void MainWindow::ImportContainer(const QString &path)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        manager.ImportContainer(path);
        SetupUIForNewContainer(manager.getContainer());
        QFileInfo finf(path);
        UpdateFileDialogImportPath(finf.dir().absolutePath());
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn(QString("Couldn't import \"`%1\"!").arg(path), QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::ImportContainer(): Importing file \"" <<path <<"\" failed with exception: \'" << excepttext << "\'";
        PrepareForNewContainer(); //Cleanup on failure
        DisplayStartScreen();
    }
}

void MainWindow::CloseContainer()
{
    HideAllTabs();
    ContentManager::Instance().CloseContainer();
    DisplayStartScreen();
    setWindowFilePath(QString());
}

void MainWindow::NewContainer(QString cnttype)
{
    try
    {
        ContentManager & manager = ContentManager::Instance();
        CloseContainer();
        //Pop new container dialog if no type passed
        if(cnttype.isNull())
        {
            //dialog
            DialogNew newdiag(this);
            int result = newdiag.exec();
            if(result == QMessageBox::Cancel)
                return;
            cnttype = newdiag.getContentType();
            manager.NewContainer(cnttype);
            if(cnttype == ContentName_Sprite)
            {
                SpriteContainer * cnt = static_cast<SpriteContainer*>(manager.getContainer());
                //Set sprite specific stuff
                cnt->SetContainerType(newdiag.getSprFormatType());
                cnt->SetExpectedCompression(newdiag.getSprCompression());
            }
            else
                Q_ASSERT(false); //If this triggers, there's a missing entry for the container type here!!
        }
        else
            manager.NewContainer(cnttype);
        SetupUIForNewContainer(manager.getContainer());
    }
    catch(const std::exception & e)
    {
        QString excepttext = GetNestedExceptionsText(e);
        Warn("Couldn't create new container!", QString("Encountered exception:\n%1").arg(excepttext));
        qWarning() << "MainWindow::NewContainer(): Creating new container failed with exception: \'" << excepttext << "\'";
        PrepareForNewContainer(); //Cleanup on failure
        DisplayStartScreen();
    }
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

void MainWindow::addMultiItemActions(const QString & /*itemname*/)
{
    //Add container menu
    ContentManager & manager = ContentManager::Instance();
    BaseContainer * cnt = manager.getContainer();
    m_pContainerMenu.reset(cnt->MakeActionMenu(ui->menuBar));
    m_pContainerMenuAction = ui->menuBar->addMenu(m_pContainerMenu.data());
    ui->menuBar->update();
}

void MainWindow::remMultiItemActions()
{
    if(m_pContainerMenuAction)
        ui->menuBar->removeAction(m_pContainerMenuAction);
    if(m_pContainerMenu)
        m_pContainerMenu.reset();
}

ContentManager & MainWindow::getManager()
{
    return ContentManager::Instance();
}

BaseContainer *MainWindow::getContainer()
{
    return ContentManager::Instance().getContainer();
}

QPixmap MainWindow::RenderNoImageSvg()
{
    QImage image(QString(":/imgs/resources/imgs/noimg.png"));
    return qMove(QPixmap::fromImage(image, Qt::ImageConversionFlag::ColorOnly | Qt::ImageConversionFlag::AvoidDither));
}

void MainWindow::on_action_Open_triggered()
{
    ContentManager & manager = ContentManager::Instance();
    QString currentDir = GetFileDialogDefaultPath();
    QString currentCntType = manager.getContainerFileFilter();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), currentDir, AllSupportedGameFileFilter(), &currentCntType);

    if(fileName.isNull())
        return;

    //Check if a sprite was opened and ask to save changes!
    //DO OPEN FILE HERE!
    if( manager.isContainerLoaded() )
    {
        qDebug() << "MainWindow::on_action_Open_triggered(): Asking for saving changes!\n";
        //Ask to save or discard changes!!
        int choice = AskSaveChanges();
        if(choice == QMessageBox::Cancel)
            return;
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
    QString currentDir = manager.getContainerParentDir();
    QString currentCntType = manager.getContainerFileFilter();

    QString filetypestr;
    if(!SupportedFileFiltersByTypename.contains(cnttype))
    {
        Warn(tr("Invalid Type!"), tr("The container you're trying to save doesn't seems to have a type somehow. Try changing the type of container."));
        return;
    }
    filetypestr = SupportedFileFiltersByTypename[manager.getContainerType()];
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File As"), currentDir, filetypestr, &currentCntType);
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
        NewContainer(ContentName_Sprite);
        ShowStatusMessage(tr("New sprite!"));
    }
    catch (const std::exception & e)
    {
        Warn("Couldn't add new sprite!", QString("Encountered exception:\n%1").arg(e.what()));
        qWarning() << "MainWindow::on_actionNewSprite_triggered(): failed with exception: \'" << GetNestedExceptionsText(e) << "\'";
    }
}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    try
    {
        NewContainer(ContentName_Sprite);
        ContentManager & manager = ContentManager::Instance();
        SpriteContainer * cnt = static_cast<SpriteContainer*>(manager.getContainer());
        cnt->SetContainerType(SpriteContainer::eContainerType::PACK); //Set to pack
        setupListView();
        updateActions();
        ShowStatusMessage(tr("New sprite pack!"));
    }
    catch (const std::exception & e)
    {
        Warn("Couldn't create new sprite pack file!", QString("Encountered exception:\n%1").arg(e.what()));
        qWarning() << "MainWindow::on_actionNewSprite_Pack_File_triggered(): failed with exception: \'" << GetNestedExceptionsText(e) << "\'";
    }  
}

void MainWindow::setupListView()
{
    ContentManager & manager = ContentManager::Instance();
    ui->tv_sprcontent->setRootIsDecorated(true);
    if(manager.isMultiItemContainer())
    {
        ui->tv_sprcontent->viewport()->update();
    }
    else
    {
        ui->tv_sprcontent->collapseAll();
    }
    ui->tv_sprcontent->update();
}

int MainWindow::AskSaveChanges(bool dosave)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Icon::Question);
    msgBox.setText("Really close this file?");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int result = msgBox.exec();
    if(dosave && result == QMessageBox::Save)
        on_action_Save_triggered();
    return result;
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
    //#TODO: Make message queue
    ui->statusBar->showMessage( msg, 8000);
}

void MainWindow::ShowStatusErrorMessage(const QString &msg)
{
    //#TODO: Make message queue
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
        qWarning() << "MainWindow::HandleItemRemoval(): failed with exception: \'" << GetNestedExceptionsText(e) << "\'";
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
    ui->tv_sprcontent->viewport()->update();
}

void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    ContentManager & manager = ContentManager::Instance();
    TreeNode * pcur = manager.getItem(index);
    if(!pcur)
        return;
    m_curItem = index;
    DisplayTabForElement(index);
    ui->tv_sprcontent->viewport()->update();
}

void MainWindow::on_tv_sprcontent_expanded(const QModelIndex &index)
{
    try
    {
        TreeNode * pcur = static_cast<TreeNode*>(index.internalPointer());
        if(!pcur)
            return;
        qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Opening proper tab!\n";
        m_curItem = index;
        DisplayTabForElement(index);
        ui->tv_sprcontent->viewport()->update();
    }
    catch(const std::exception & e)
    {
        qWarning() << "MainWindow::on_tv_sprcontent_expanded(): failed with exception: \'" << GetNestedExceptionsText(e) << "\'";
    }
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

    TVSpritesContextMenu * menu = new TVSpritesContextMenu(this, entry, ui->tv_sprcontent);
    //connect( menu, SIGNAL(afterclosed()), menu, SLOT(deleteLater()) );
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

eTreeElemDataType MainWindow::currentEntryType()const
{
    const TreeNode * elem = reinterpret_cast<TreeNode*>(ui->tv_sprcontent->currentIndex().internalPointer());
    if(elem)
        return elem->nodeDataTy();
    return eTreeElemDataType::None;
}

QModelIndexList MainWindow::currentlySelectedItems()const
{
    QItemSelectionModel * pmodel = ui->tv_sprcontent->selectionModel();
    return pmodel->selectedRows();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //        if (userReallyWantsToQuit())
    //        {
    ProgramSettings::Instance().ReadSettings(objectName(), std::bind(&MainWindow::writeSettings, this, std::placeholders::_1) );
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
    m_progress.exec();
}

void MainWindow::updateCoordinateBar(const QPointF &pos)
{
    m_pStatusCoordinates->setText(QString{"( %1, %2 )"}.arg(qRound(pos.x())).arg(qRound(pos.y())));
}

void MainWindow::updateCurrentTab()
{
    ui->stkEditor->currentWidget()->update();
}

void MainWindow::OnContentRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    QModelIndex newindex;
    if(!parent.isValid())
    {
        //Deleting a top level sub-item
        if( !(m_curItem.row() >= first) && !(m_curItem.row() <= last)) //De-select if we removed the current item
            return; //Do nothing
    }
    else
    {
        //Deleting a child of a top level sub-item
        newindex = parent; //Just select the parent node
    }
    selectTreeViewNode(newindex);
    if(newindex.isValid())
        DisplayTabForElement(newindex);
    else
        HideAllTabs();
}

void MainWindow::OnContentRowsAboutToBeMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    QModelIndex newindex;
    if(!parent.isValid())
    {
        //Moving a top level sub-item
        if( !(m_curItem.row() >= start && m_curItem.row() <= end) ||
            !(destination == parent && m_curItem.row() >= row)) //De-select if we moved the current item
            return; //Do nothing
    }
    else
    {
        //Moving a child of a top level sub-item
        newindex = parent; //Just select the parent node
    }
    selectTreeViewNode(newindex);
    if(newindex.isValid())
        DisplayTabForElement(newindex);
    else
        HideAllTabs();
}

void MainWindow::OnRowInserted(const QModelIndex &parent, int /*first*/, int last)
{
    ContentManager & manager = ContentManager::Instance();
    QModelIndex newcurrent = manager.index(last, 0, parent);
    selectTreeViewNode(newcurrent);
    if(newcurrent.isValid())
        DisplayTabForElement(newcurrent);
}

void MainWindow::selectTreeViewNode(const TreeNode * node)
{
    ContentManager & manager = ContentManager::Instance();
    QModelIndex index = manager.modelIndexOf(node);
    selectTreeViewNode(index);
}

void MainWindow::selectTreeViewNode(const QModelIndex &index)
{
    m_curItem = index;
    ui->tv_sprcontent->setCurrentIndex(m_curItem);
}

void MainWindow::openTreeViewNode(const TreeNode *node)
{
    if(!node)
    {
        qWarning() << "MainWindow::openTreeViewNode(): Tried to display tab for a null pointer!";
        return;
    }
    selectTreeViewNode(node);
    DisplayTabForElement(m_curItem);
    {
        QSignalBlocker blk(ui->tv_sprcontent);
        if(!ui->tv_sprcontent->isExpanded(m_curItem.parent()))
            ui->tv_sprcontent->expand(m_curItem.parent());
    }
    ui->tv_sprcontent->viewport()->update();
}

void MainWindow::openTreeViewNode(const QModelIndex &index)
{
    if(!index.isValid())
    {
        qWarning() << "MainWindow::openTreeViewNode(): Tried to display tab for bad model index!";
        return;
    }
    selectTreeViewNode(index);
    DisplayTabForElement(index);
    {
        QSignalBlocker blk(ui->tv_sprcontent);
        if(!ui->tv_sprcontent->isExpanded(index.parent()))
            ui->tv_sprcontent->expand(index.parent());
    }
    ui->tv_sprcontent->viewport()->update();
}

void MainWindow::on_actionSprite_Sheet_Auto_Importer_triggered()
{
    if(currentSprite())
    {
        DialogImportWizard dlg(this, currentSprite());
        dlg.exec();
    }
    else
    {
        ShowStatusErrorMessage(tr("No active sprites"));
        Q_ASSERT(false);
    }
}

void MainWindow::on_actionAdvanced_triggered()
{
    try
    {
        NewContainer();
    }
    catch (const std::exception & e)
    {
        Warn("Encountered a problem creating the new container!", QString("Encountered exception:\n%1").arg(GetNestedExceptionsText(e)));
        qWarning() << "MainWindow::on_actionAdvanced_triggered(): failed with exception: \'" << e.what() << "\'";
    }
}

void MainWindow::on_action_Settings_triggered()
{
    DialogSettings diag(this);
    diag.exec();
    update();
    updateActions();
    ui->stkEditor->currentWidget()->update();
}

void MainWindow::ProcessArguments(const QCommandLineParser &parser)
{
    //#TODO: Expand on this
    QString openFilePath = cmdline_processing::ParseSourceFilepath(parser);
    if(!openFilePath.isEmpty())
        LoadContainer(openFilePath);
}
