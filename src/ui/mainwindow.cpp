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
#include <src/ui/diagsingleimgcropper.hpp>
#include <src/ui/dialogabout.hpp>
#include <src/ui/paletteeditor.hpp>
#include <src/ui/tvspritescontextmenu.hpp>
#include <src/ui/errorhelper.hpp>

static const QString MAIN_WINDOW_TITLE = "PMD2 Sprite Muncher v%1";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_imgNoImg(":/imgs/resources/imgs/noimg.png"),
    m_aboutdiag(this),
    m_progress(this),
    m_settings("./settings.ini",QSettings::Format::IniFormat)
{
    //Resources alloc
    m_pStatusFileType.reset(new QLabel("     "));
    setWindowTitle(MAIN_WINDOW_TITLE.arg(QString(GIT_MAJORMINOR_VERSION)));

    //Setup error helper
    ErrorHelper::getInstance().setMainWindow(this);

    //UI init
    ui->setupUi(this);

    //Setup references to main window in tabs
    ui->tabAnimTable->setMainWindow(this);
    ui->tabSequence->setMainWindow(this);

    //Parse settings!
    readSettings();

    ui->tv_sprcontent->setModel( &spr_manager::SpriteManager::Instance() );
    ui->statusBar->addPermanentWidget(m_pStatusFileType.data());

    ui->spbFrmPartXOffset->setRange(0, fmt::step_t::XOFFSET_MAX);
    ui->spbFrmPartYOffset->setRange(0, fmt::step_t::YOFFSET_MAX);

    ui->spbimgunk2->setRange (0, std::numeric_limits<uint32_t>::max());
    ui->spbimgunk14->setRange(0, std::numeric_limits<uint16_t>::max());

    DisplayStartScreen();
}

MainWindow::~MainWindow()
{
    qDebug("MainWindow::~MainWindow()\n");

    //Gotta do some special handling to avoid things accessing sprites from breaking things
    ui->tabAnimTable->OnDestruction();
    ui->tabSequence->OnDestruction();

    //Get rid of the ui
    delete ui;

    //Delete the sprite container before the Qt framework is unloaded!
    spr_manager::SpriteManager::Instance().CloseContainer();

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

    //Stop any animations
//    HideAnimSequencePage();

    //Remove tabs from the widget
//    ui->stkEditor->removeWidget(ui->tabAnimTable);
//    ui->stkEditor->removeWidget(ui->tabEfx);
//    ui->stkEditor->removeWidget(ui->tabFrame);
//    ui->stkEditor->removeWidget(ui->tabSequence);
//    ui->stkEditor->removeWidget(ui->tabProperties);
//    ui->stkEditor->removeWidget(ui->tabWelcome);
//    ui->stkEditor->removeWidget(ui->tabImages);

    //Hide all removed tabs
    //ui->tabEfx->hide();
    //ui->tabFrame->hide();
    //ui->tabProperties->hide();
    //ui->tabWelcome->hide();
    //ui->tabImages->hide();

    //New tabs
    if(ui->stkEditor->currentWidget() == ui->tabSequence)
        ui->tabSequence->OnHideTab();
    if(ui->stkEditor->currentWidget() == ui->tabAnimTable)
        ui->tabAnimTable->OnHideTab();

    //Clear the property tab's preview
    ui->lbl_imgpreview->clear();
    ui->lbl_imgpreview->setPixmap(m_imgNoImg);

    ui->stkEditor->setCurrentWidget(ui->tabEmpty);
    //ui->stkEditor->setUpdatesEnabled(true);
}

void MainWindow::ShowATab(QWidget *ptab)
{
    Q_ASSERT(ptab);
    qDebug() << "MainWindow::ShowATab(): Hiding tabs!\n";
    HideAllTabs();
    qDebug() << "MainWindow::ShowATab(): Displaying tab!\n";
    ui->stkEditor->setCurrentWidget(ptab);
    qDebug() << "MainWindow::ShowATab(): Tab displayed!\n";
    ui->stkEditor->update(ui->stkEditor->rect());
}

void MainWindow::ShowATab(BaseSpriteTab *ptab)
{
    //#TODO: Placeholder for the refactor!!!
    Q_ASSERT(ptab);
    ShowATab(ptab);
}

void MainWindow::DisplayStartScreen()
{
    qDebug() << "MainWindow::DisplayStartScreen(): Showing start screen!\n";
    ShowATab(ui->tabWelcome);
    ShowStatusMessage(tr("Welcome! May you encounter no bugs today!"));
}

void MainWindow::DisplayEffectsPage(Sprite *spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabEfx);
}

void MainWindow::SetupUIForNewContainer(spr_manager::SpriteContainer * sprcnt)
{
    ui->tv_sprcontent->setModel( & spr_manager::SpriteManager::Instance() );
    connect(sprcnt, SIGNAL(showProgress(QFuture<void>&)), this, SLOT(ShowProgressDiag(QFuture<void>&)) );
    setupListView();
    updateActions();

    //Display!
    if(sprcnt && sprcnt->hasChildren())
    {
        m_cursprite = sprcnt->modelIndexOfNode(&sprcnt->GetSprite(0));
        DisplayPropertiesPage(&sprcnt->GetSprite(0));
    }
    ui->tv_sprcontent->repaint();
    setWindowFilePath(sprcnt->GetContainerSrcPath());
}

void MainWindow::setupFrameEditPageForPart(MFrame */*frm*/, MFramePart */*part*/)
{
//    if(!frm || !part)
//    {
//        qWarning("MainWindow::setupFrameEditPageForPart(): Got null frame or part! Skipping!");
//        HideAllTabs(); //Reset UI to try to avoid further issues
//        ui->tv_sprcontent->setCurrentIndex(QModelIndex()); //Reset currently selected sprite
//        ShowStatusErrorMessage("An error occured. UI reset! Please notify devs!");
//        return;
//    }
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
    lambdaClearModel(ui->tblframeparts);
    lambdaClearModel(ui->tblProperties);
    //lambdaClearModel(ui->tblseqfrmlst);
    lambdaClearModel(ui->tblviewImages);

    ui->tabSequence->PrepareForNewContainer();
    ui->tabAnimTable->PrepareForNewContainer();
}

void MainWindow::LoadContainer(const QString &path)
{
    PrepareForNewContainer();

    //Pre-check if file exists
    if(QFile::exists(path))
    {
        //Open
        qInfo() <<"MainWindow::LoadContainer() : " <<path <<"!\n";
        spr_manager::SpriteContainer * sprcnt = spr_manager::SpriteManager::Instance().OpenContainer(path);
        qInfo() <<"\nLoaded!\n";
        m_lastSavePath = path;
        SetupUIForNewContainer(sprcnt);
    }
    else
    {
        qInfo() << "Failed to open file \"" <<path <<"\"!";
    }
}

void MainWindow::SaveContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().SaveContainer(path);
    updateActions();
    if(spr_manager::SpriteManager::Instance().getContainer())
        setWindowFilePath(spr_manager::SpriteManager::Instance().getContainer()->GetContainerSrcPath());
}

void MainWindow::ExportContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().ExportContainer(path);
    updateActions();
}

void MainWindow::ImportContainer(const QString &path)
{
    spr_manager::SpriteContainer * sprcnt = spr_manager::SpriteManager::Instance().ImportContainer(path);

    SetupUIForNewContainer(sprcnt);
}

void MainWindow::CloseContainer()
{
    HideAllTabs();
    spr_manager::SpriteManager::Instance().CloseContainer();
    DisplayStartScreen();
    setWindowFilePath(QString());
}

void MainWindow::updateActions()
{
    qInfo() <<"MainWindow::updateActions(): Updating!\n";
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    m_pStatusFileType->setText(sprman.getContentShortName());

    if( sprman.ContainerIsPackFile() )
    {
        qInfo() <<"MainWindow::updateActions(): Adding pack file specific menu entries!\n";
        m_pActionAddSprite.reset(ui->menu_Edit->addAction(QString("Add sprite.."), this, &(MainWindow::OnActionAddSprite)));
        m_pActionRemSprite.reset(ui->menu_Edit->addAction(QString("Remove sprite.."), this, &(MainWindow::OnActionRemSprite)));
    }
    else
    {
        qInfo() <<"MainWindow::updateActions(): Removing pack file specific menu entries!\n";
        ui->menu_Edit->removeAction(m_pActionAddSprite.data());
        ui->menu_Edit->removeAction(m_pActionRemSprite.data());
        m_pActionAddSprite.reset(nullptr);
        m_pActionRemSprite.reset(nullptr);
    }
}

spr_manager::SpriteManager & MainWindow::getManager()
{
    return spr_manager::SpriteManager::Instance();
}

spr_manager::SpriteContainer *MainWindow::getContainer()
{
    return spr_manager::SpriteManager::Instance().getContainer();
}

void MainWindow::OnActionAddSprite()
{
    qInfo() <<"MainWindow::OnActionAddSprite(): Adding sprite!\n";
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    sprman.AddSpriteToContainer(Sprite(sprman.getContainer()));
}

void MainWindow::OnActionRemSprite()
{
    qInfo() <<"MainWindow::OnActionRemSprite(): Removing sprite!\n";
    spr_manager::SpriteManager::Instance().RemSpriteFromContainer(m_cursprite);
}

QPixmap MainWindow::RenderNoImageSvg()
{
    QImage image(QString(":/imgs/resources/imgs/noimg.png"));
    return qMove(QPixmap::fromImage(image, Qt::ImageConversionFlag::ColorOnly | Qt::ImageConversionFlag::AvoidDither));
}

void MainWindow::on_action_Open_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            QString("%1;;%2;;%3;;%4")
            .arg(WanFileFilter())
            .arg(WatFileFilter())
            .arg(PACKFileFilter())
            .arg(AllFileFilter()) );

    if(fileName.isNull())
        return;

    //Check if a sprite was opened and ask to save changes!
    //DO OPEN FILE HERE!
    if( sprman.IsContainerLoaded() )
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
    TreeElement * pcur = static_cast<TreeElement*>(index.internalPointer());
    if(!pcur)
        return;

    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Expanding! Disabling updates!\n";
    //ui->tv_sprcontent->setUpdatesEnabled(false);
    pcur->OnExpanded();
    //ui->tv_sprcontent->dataChanged(index, index); //Make sure we display expandable items properly if we just loaded them!
    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Changing current index!\n";
    //ui->tv_sprcontent->setCurrentIndex(index);

//    ui->tv_sprcontent->setUpdatesEnabled(false);
//    ui->tv_sprcontent->expand(index);
//    ui->tv_sprcontent->setUpdatesEnabled(true);

    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Opening proper tab!\n";
    switch( pcur->getNodeDataTy() )
    {
        //Open the appropriate tab
    case eTreeElemDataType::sprite:
        {
            Sprite * spr = static_cast<Sprite*>(pcur);
            DisplayPropertiesPage(spr);
            break;
        }
    default:
        break;
    };
    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Enabling updates!\n";
    //ui->tv_sprcontent->setUpdatesEnabled(true);
    ui->tv_sprcontent->viewport()->update();
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
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();

    QString filetypestr;
    switch(sprman.GetType())
    {
    case spr_manager::SpriteContainer::eContainerType::PACK:
        {
            filetypestr = PACKFileFilter();
            break;
        }
    case spr_manager::SpriteContainer::eContainerType::WAN:
        {
            filetypestr = WanFileFilter();
            break;
        }
    case spr_manager::SpriteContainer::eContainerType::WAT:
        {
            filetypestr = WatFileFilter();
            break;
        }
    default:
        {
            Warn(tr("Invalid Type!"), tr("The container you're trying to save doesn't seems to have a type somehow. Try changing the type of container."));
            return;
        }
    };
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
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    spr_manager::SpriteContainer * sprcnt = sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::WAN );

    if(sprcnt)
        DisplayPropertiesPage(&sprcnt->GetSprite(0));

    //Display!
    updateActions();
    setupListView();
    ShowStatusMessage(tr("New sprite!"));
}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    //spr_manager::SpriteManager   & sprman = spr_manager::SpriteManager::Instance();
    //spr_manager::SpriteContainer * sprcnt = sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::PACK );

    //Display!
    updateActions();
    setupListView();
    ShowStatusMessage(tr("New pack file!"));
}

void MainWindow::setupListView()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    if( sprman.ContainerIsSingleSprite() )
    {
        ui->tv_sprcontent->setRootIsDecorated(false);
        ui->tv_sprcontent->expandToDepth(1);
        ui->tv_sprcontent->viewport()->update();
        ui->tv_sprcontent->collapseAll();
        ui->tv_sprcontent->viewport()->update();
        ui->tv_sprcontent->expandToDepth(0);
    }
    else
    {
        ui->tv_sprcontent->setRootIsDecorated(true);
        ui->tv_sprcontent->collapseAll();
    }
}

void MainWindow::SaveAs(const QString &path)
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
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
    if(currentSpriteModelIndex() == spriteidx)
    {
        HideAllTabs(); //Deselect everything if we're deleting the current entry
        ui->tblframeparts->setModel(nullptr);
        QModelIndex parentidx = ui->tv_sprcontent->currentIndex().parent();
        if(parentidx.isValid())
            ui->tv_sprcontent->setCurrentIndex(parentidx);
        else
            ui->tv_sprcontent->setCurrentIndex(QModelIndex());
    }
    ui->tv_sprcontent->model()->removeRow( spriteidx.row(), spriteidx.parent() );
    ShowStatusMessage( QString(tr("Entry removed!")) );
}

void MainWindow::on_tv_sprcontent_activated(const QModelIndex &index)
{
    this->on_tv_sprcontent_clicked(index);
}

void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    TreeElement * pcur = spr_manager::SpriteManager::Instance().getItem(index);
    if(!pcur)
        return;

    ui->tv_sprcontent->update();

    //Handle the clicking
    pcur->OnClicked();

    //Check if the parent sprite loaded
    Sprite * pspr = pcur->parentSprite();
    if(!pspr)
    {
        qFatal("MainWindow::on_tv_sprcontent_clicked() : Something very wrong is going on with the sprite");
        return;
    }
    if(!pspr->wasParsed())
        pspr->ParseSpriteData(); //Make sure its parsed before displaying!!

    if(pcur->getNodeDataTy() < eTreeElemDataType::INVALID &&
            pcur->getNodeDataTy() != eTreeElemDataType::None &&
            pcur->parentSprite())
    {
        m_cursprite = spr_manager::SpriteManager::Instance().modelIndexOf(pcur->parentSprite());
    }
    else
        m_cursprite = QModelIndex();

    switch( pcur->getNodeDataTy() )
    {
        //Open the appropriate tab
    case eTreeElemDataType::sprite:
        {
            Sprite * spr = static_cast<Sprite*>(pcur);
            DisplayPropertiesPage(spr);
            break;
        }
    case eTreeElemDataType::effectOffsets:
        {
            EffectOffsetContainer * efx = static_cast<EffectOffsetContainer*>(pcur);
            Sprite * spr = efx->parentSprite();
            DisplayEffectsPage(spr);
            break;
        }
    case eTreeElemDataType::animTable:
        {
            AnimTable   * tbl = static_cast<AnimTable*>(pcur);
            Sprite      * spr = tbl->parentSprite();
            ui->stkEditor->setCurrentWidget(ui->tabAnimTable);
            ui->tabAnimTable->OnShowTab(spr, index);
            break;
        }
    case eTreeElemDataType::frame:
        {
            MFrame * frm = static_cast<MFrame*>(pcur);
            Sprite * spr = frm->parentSprite();
            DisplayMFramePage(spr, frm);
            break;
        }
    case eTreeElemDataType::image:
        {
            Image           * img   = static_cast<Image*>(pcur);
            ImageContainer  * imgs  = static_cast<ImageContainer*>(img->parentNode());
            Sprite          * spr   = img->parentSprite();
            DisplayImageListPage(spr, imgs, img);
            break;
        }
    case eTreeElemDataType::images:
        {
            ImageContainer  * imgs  = static_cast<ImageContainer*>(pcur);
            Sprite          * spr   = imgs->parentSprite();
            DisplayImageListPage(spr,imgs);
            break;
        }
    case eTreeElemDataType::animSequence:
        {
            AnimSequence    * seq   = static_cast<AnimSequence*>(pcur);
            Sprite          * spr   = seq->parentSprite();
            ui->stkEditor->setCurrentWidget(ui->tabSequence);
            ui->tabSequence->OnShowTab(spr, index);
            break;
        }
    case eTreeElemDataType::palette:
    case eTreeElemDataType::animGroup:
    default:
        HideAllTabs();
    };
    //ui->tv_sprcontent->viewport()->update();
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
    spr_manager::SpriteContainer * pcnt = getContainer();
    Q_ASSERT(pcnt);
    TreeElement * elem = static_cast<TreeElement*>(entry.internalPointer());
    Q_ASSERT(elem);

    TVSpritesContextMenu * menu = new TVSpritesContextMenu( this, entry, ui->tv_sprcontent);
    connect( menu, SIGNAL(afterclosed()), menu, SLOT(deleteLater()) );
    return menu;
}

Sprite * MainWindow::currentSprite()
{
    return reinterpret_cast<Sprite *>(m_cursprite.internalPointer());
//    TreeElement * elem = static_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());
//    if(!elem)
//        return nullptr;

//    Sprite * spr = elem->parentSprite();
//    if(!spr)
//    {
//        //This mean, we've selected the sprite itself, so return that!
//        spr = static_cast<Sprite*>(elem);
//    }
    //    return spr;
}

QModelIndex MainWindow::currentSpriteModelIndex()
{
    return m_cursprite;
}

MFrame *MainWindow::currentFrame()
{
    TreeElement * elem = reinterpret_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());

    //In cases where we're in the treeview on a child entry, get the parent!
    if(elem && elem->getNodeDataTy() == eTreeElemDataType::framepart)
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
    TreeElement * elem = reinterpret_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());
    if(elem)
        return elem->getNodeDataTy();
    return eTreeElemDataType::None;
}

MFramePart * MainWindow::currentTblFrameParts()
{
    MFramePart * elem = reinterpret_cast<MFramePart*>(ui->tblframeparts->currentIndex().internalPointer());
    return elem;
}

Image * MainWindow::currentTblImages()
{
    Image * elem = reinterpret_cast<Image*>(ui->tblviewImages->currentIndex().internalPointer());
    return elem;
}


void MainWindow::ShowProgressDiag(QFuture<void> &task)
{
    m_progress.setFuture(task);
    m_progress.setModal(true);
    m_progress.show();
}


void MainWindow::on_tblframeparts_clicked(const QModelIndex &/*index*/)
{

}
