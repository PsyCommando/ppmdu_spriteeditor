#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>
#include <QSvgRenderer>
#include <QSpinBox>
#include <QTimer>
#include <dialogexport.hpp>
#include <diagsingleimgcropper.hpp>
#include <dialogabout.hpp>
#include <paletteeditor.hpp>

const QString PaletteFilter = spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::RIFF_Pal) +
                              ";;" +
                              spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::TEXT_Pal) +
                              ";;" +
                              spr_manager::GetPaletteFileFilterString(spr_manager::ePaletteDumpType::GIMP_PAL);


QPixmap ImageToPixmap( QImage && img, const QSize & sz )
{
    return qMove(QPixmap::fromImage(img, Qt::ImageConversionFlag::AvoidDither | Qt::ImageConversionFlag::ColorOnly).scaled( sz, Qt::KeepAspectRatio));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_previewrender(true),
    m_aboutdiag(this),
    m_imgNoImg(":/imgs/resources/imgs/noimg.png"),
    m_progress(this),
    m_settings("./settings.ini",QSettings::Format::IniFormat)
{
    //Resources alloc
    m_pStatusFileType.reset(new QLabel("     "));
    //UI init
    ui->setupUi(this);

    //Parse settings!
    readSettings();

    //Connect stuff
    connect( ui->chkAnimSeqLoop, &QCheckBox::toggled, &m_previewrender, &SceneRenderer::loopChanged );
    connect( ui->btnSeqPlay, &QPushButton::clicked, &m_previewrender, &SceneRenderer::startAnimUpdates );
    connect( ui->btnSeqStop, &QPushButton::clicked, &m_previewrender, &SceneRenderer::stopAnimUpdates );

    connect( &m_previewrender, SIGNAL(rangechanged(int,int)), ui->sldrAnimSeq, SLOT(setRange(int,int)) );
    connect( &m_previewrender, SIGNAL(framechanged(int)), ui->sldrAnimSeq, SLOT(setValue(int)) );
    connect( &m_previewrender, SIGNAL(framechanged(int)), ui->spinCurFrm, SLOT(setValue(int)) );

    connect( ui->spinCurFrm, SIGNAL(valueChanged(int)), &m_previewrender, SLOT(setCurrentFrame(int)) );
    connect( ui->sldrAnimSeq, SIGNAL(valueChanged(int)), &m_previewrender, SLOT(setCurrentFrame(int)) );

    ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
    ui->tv_sprcontent->setModel( & spr_manager::SpriteManager::Instance() );
    ui->statusBar->addPermanentWidget(m_pStatusFileType.data());

    ui->spbFrmPartXOffset->setRange(0, fmt::step_t::XOFFSET_MAX);
    ui->spbFrmPartYOffset->setRange(0, fmt::step_t::YOFFSET_MAX);

    DisplayStartScreen();
    InitAnimScene();
}

MainWindow::~MainWindow()
{
    qDebug("MainWindow::~MainWindow()\n");
    delete ui;

    //Delete the sprite container before the Qt framework is unloaded!
    spr_manager::SpriteManager::Instance().CloseContainer();
}

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
    //Hide all
    ui->tabMain->setUpdatesEnabled(false);

    //Stop any animations
    InitAnimScene();

    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabanims));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabeffects));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabframeseditor));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabseq));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabproperties));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabWelcome));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabImages));

    ui->tabanims->hide();
    ui->tabeffects->hide();
    ui->tabframeseditor->hide();
    ui->tabseq->hide();
    ui->tabproperties->hide();
    ui->tabWelcome->hide();
    ui->tabImages->hide();

    //ui->tblframeparts->setModel(nullptr);
    ui->lbl_imgpreview->clear();
    ui->lbl_imgpreview->setPixmap(m_imgNoImg);

    ui->tabMain->setUpdatesEnabled(true);
}

void MainWindow::ShowATab(QWidget *ptab)
{
    Q_ASSERT(ptab);
    HideAllTabs();
    qDebug() << "MainWindow::ShowATab(): Hiding tabs!\n";
    ui->tabMain->insertTab(0, ptab, ptab->windowTitle() );
    qDebug() << "MainWindow::ShowATab(): Adding tab to be displayed!\n";
    ptab->show();
    ptab->setFocus();
    qDebug() << "MainWindow::ShowATab(): Tab displayed!\n";
}

void MainWindow::DisplayStartScreen()
{
    qDebug() << "MainWindow::DisplayStartScreen(): Showing start screen!\n";
    ShowATab(ui->tabWelcome);
    ShowStatusMessage(tr("Welcome! May you encounter no bugs today!"));
}

void MainWindow::DisplayPropertiesPage(Sprite * spr)
{
    Q_ASSERT(spr);
    qDebug() << "MainWindow::DisplayPropertiesPage(): Showing properties tab!\n";

    spr_manager::SpriteContainer * pcnt = spr_manager::SpriteManager::Instance().getContainer();
    ui->tv_sprcontent->setCurrentIndex(pcnt->index(pcnt->indexOfNode(spr), 0, QModelIndex(), &spr_manager::SpriteManager::Instance() ));
    if( !spr->wasParsed() )
        spr->ParseSpriteData();

    //display preview only if we have image data!
    if( spr->hasImageData() )
        ui->lblPropPreview->setPixmap(spr->MakePreviewFrame().scaled( ui->lblPropPreview->size(), Qt::KeepAspectRatio) );
    else
        ui->lblPropPreview->setPixmap(m_imgNoImg);

    ui->tblProperties->setModel(spr->propHandler()->model());
    ui->tblProperties->setItemDelegate(spr->propHandler()->delegate());

    //display palette preview
    ui->lbl_test_palette->setPixmap(spr->MakePreviewPalette());

    //Setup stats
    ui->tblOverview;

    ShowATab(ui->tabproperties);
}

void MainWindow::DisplayMFramePage(Sprite *spr, MFrame * frm)
{
    Q_ASSERT(spr && frm);
    qDebug() << "MainWindow::DisplayMFramePage(): Showing frame page!\n";
    ui->tblframeparts->setModel(frm->getModel());
    ui->tblframeparts->setItemDelegate(&(frm->itemDelegate()));
    ui->tblframeparts->setEditTriggers(QTableView::EditTrigger::AllEditTriggers);
    ui->tblframeparts->setSizeAdjustPolicy(QTableView::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
    ShowATab(ui->tabframeseditor);
    ui->tblframeparts->resizeRowsToContents();
    ui->tblframeparts->resizeColumnsToContents();
    ui->tblframeparts->setCurrentIndex( ui->tblframeparts->model()->index(0, 0, QModelIndex()) );

    //Setup the frame editor in the viewport!
    m_frmeditor.reset( new FrameEditor(frm) );
    ui->gvFrame->setScene( &m_frmeditor->getScene() );
    m_frmeditor->initScene(ui->chkColorPartOutlines->isChecked(),
                           ui->chkFrmMiddleMarker->isChecked(),
                           ui->chkFrmTransparency->isChecked());
    ui->gvFrame->repaint();

    if(ui->tblframeparts->currentIndex().isValid())
        setupFrameEditPageForPart( frm, static_cast<MFramePart*>(ui->tblframeparts->currentIndex().internalPointer()) );


//    ui->spbFrmPartXOffset->setValue();
//    ui->spbFrmPartYOffset->setValue();

    //Setup the callbacks
    connect(frm->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &,const QModelIndex &, const QVector<int>&)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });

    connect(ui->spbFrmZoom, qOverload<int>(&QSpinBox::valueChanged), [&](int val)->void
    {
        qreal sc = val * 0.01; //scale the value from 0 to 1 +
        ui->gvFrame->setTransform(QTransform::fromScale(sc, sc));
    });

    connect(m_frmeditor.data(), &FrameEditor::zoom, [&](int val)->void
    {
        //qreal sc = val * 0.01; //scale the value from 0 to 1 +
        ui->spbFrmZoom->setValue(val + ui->spbFrmZoom->value());
        //ui->gvFrame->setTransform(QTransform::fromScale(sc, sc));
    });

    //Init checkboxes state
    connect(ui->chkColorPartOutlines, &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawOutlines);
    connect(ui->chkFrmMiddleMarker,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setDrawMiddleGuide);
    connect(ui->chkFrmTransparency,   &QCheckBox::toggled, m_frmeditor.data(), &FrameEditor::setTransparencyEnabled);

    //Map model's columns to some of the controls
    m_frmdatmapper.reset(new QDataWidgetMapper);
    m_frmdatmapper->setModel(frm->getModel());
    m_frmdatmapper->addMapping(ui->spbFrmPartXOffset,  static_cast<int>(eFramesColumnsType::direct_XOffset) );
    m_frmdatmapper->addMapping(ui->spbFrmPartYOffset,  static_cast<int>(eFramesColumnsType::direct_YOffset) );
    m_frmdatmapper->addMapping(ui->btnFrmVFlip,        static_cast<int>(eFramesColumnsType::direct_VFlip) );
    m_frmdatmapper->addMapping(ui->btnFrmHFlip,        static_cast<int>(eFramesColumnsType::direct_HFlip) );
    m_frmdatmapper->toFirst();
    connect(ui->tblframeparts, &QTableView::clicked, m_frmdatmapper.data(), &QDataWidgetMapper::setCurrentModelIndex);

//    connect(ui->spbFrmPartXOffset, qOverload<int>(&QSpinBox::valueChanged), [&](int val)
//    {
//        QModelIndex mi = ui->tblframeparts->currentIndex();
//        mi = frm->getModel()->index(mi.row(), static_cast<int>(eFramesColumnsType::Offset), mi.parent());
//        QVariant newoffset;
//        newoffset.setValue<QPair<int,int>>( QPair<int,int>( val, ui->spbFrmPartYOffset->value()) );
//        frm->getModel()->setData( mi, newoffset);
//        m_frmeditor->updateParts();
//    });

//    connect(ui->spbFrmPartYOffset, qOverload<int>(&QSpinBox::valueChanged), [&](int val)
//    {
//        QModelIndex mi = ui->tblframeparts->currentIndex();
//        mi = frm->getModel()->index(mi.row(), static_cast<int>(eFramesColumnsType::Offset), mi.parent());
//        QVariant newoffset;
//        newoffset.setValue<QPair<int,int>>( QPair<int,int>(ui->spbFrmPartXOffset->value(), val) );
//        frm->getModel()->setData( mi, newoffset);
//        m_frmeditor->updateParts();
//    });


    //connect(m_frmeditor.data(), &FrameEditor::partMoved, ui->tblframeparts, &QTableView::repaint );
}

void MainWindow::DisplayAnimSequencePage(Sprite *spr, AnimSequence * aniseq)
{
    Q_ASSERT(spr && aniseq);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Showing anim sequence page!\n";
    ShowATab(ui->tabseq);
    m_previewrender.setScene(spr, aniseq->nodeIndex());
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Instanciated anime viewer!\n";
    ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
    ui->gvAnimSeqViewport->centerOn(m_previewrender.getAnimSprite());
    m_previewrender.getAnimSprite()->setScale(2.0);
    ui->tblseqfrmlst->setModel(&aniseq->getModel());

    qDebug() << "MainWindow::DisplayAnimSequencePage(): Scene set!\n";
}

void MainWindow::DisplayAnimTablePage(Sprite * spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabanims);
}

void MainWindow::DisplayPalettePage(Sprite *spr)
{
    Q_ASSERT(spr);
    //ShowATab(ui->tabpal);
}

void MainWindow::DisplayEffectsPage(Sprite *spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabeffects);
}

void MainWindow::DisplayAnimGroupPage(Sprite *spr)
{
    Q_ASSERT(spr);
    //ShowATab(ui->tabanimgrp);
}

void MainWindow::DisplayImagePage(Sprite *spr, Image * img)
{
//    Q_ASSERT(spr && img);
//    qDebug() << "MainWindow::DisplayImagePage(): Displaying image page!\n";
//    ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(spr->getPalette()), ui->lbl_imgpreview->size()));
//    qDebug() << "MainWindow::DisplayImagePage(): Pixmap assigned!\n";
//    ShowATab(ui->tabimage);
}

void MainWindow::DisplayImageListPage(Sprite *spr, ImageContainer *pimgs, Image *img)
{
    Q_ASSERT(spr && pimgs);
    qDebug() << "MainWindow::DisplayImageListPage(): Displaying images list page!\n";
    ui->tblviewImages->setModel(pimgs->getModel());
    qDebug() << "MainWindow::DisplayImageListPage(): Model set!\n";
    ShowATab(ui->tabImages);
    ui->tblviewImages->resizeRowsToContents();
    ui->tblviewImages->resizeColumnsToContents();

    if(img)
    {
        //select specified image!
        spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
        QModelIndex ind = ui->tblviewImages->model()->index(pimgs->indexOfNode(img), 0);
        ui->tblviewImages->setCurrentIndex(ind);
        on_tblviewImages_clicked(ind);
    }
}

void MainWindow::SetupUIForNewContainer(spr_manager::SpriteContainer * sprcnt)
{
    ui->tv_sprcontent->setModel( & spr_manager::SpriteManager::Instance() );
    connect(sprcnt, SIGNAL(showProgress(QFuture<void>&)), this, SLOT(ShowProgressDiag(QFuture<void>&)) );
    setupListView();
    updateActions();

    //Display!
    if(sprcnt && sprcnt->hasChildren())
        DisplayPropertiesPage(&sprcnt->GetSprite(0));
    ui->tv_sprcontent->repaint();
}

void MainWindow::setupFrameEditPageForPart(MFrame *frm, MFramePart *part)
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
    lambdaClearModel(ui->tblseqfrmlst);
    lambdaClearModel(ui->tblviewImages);
}

void MainWindow::LoadContainer(const QString &path)
{
    PrepareForNewContainer();
    //Open
    qInfo() <<"MainWindow::LoadContainer() : " <<path <<"!\n";
    spr_manager::SpriteContainer * sprcnt = spr_manager::SpriteManager::Instance().OpenContainer(path);
    qInfo() <<"\nLoaded!\n";
    m_lastSavePath = path;

    SetupUIForNewContainer(sprcnt);
}

void MainWindow::SaveContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().SaveContainer(path);
    updateActions();
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
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    Q_ASSERT(false);
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

    //Check if a sprite was open and ask to save changes!
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
    ui->tv_sprcontent->setUpdatesEnabled(false);
    pcur->OnExpanded();
    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Changing current index!\n";
    ui->tv_sprcontent->setCurrentIndex(index);

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
    ui->tv_sprcontent->setUpdatesEnabled(true);
    ui->tv_sprcontent->viewport()->update();
}

//void MainWindow::on_tv_sprcontent_itemClicked(QTreeWidgetItem *item, int column)
//{

//}

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

void MainWindow::on_action_Export_triggered()
{
    DialogExport exp(this);
    exp.setModal(true);
    exp.show();
    //QSaveFile;
}

//void MainWindow::on_actionSprite_triggered()
//{

//}

//void MainWindow::on_actionSprite_Pack_File_triggered()
//{

//}

void MainWindow::on_actionUndo_triggered()
{

}

void MainWindow::on_actionRedo_triggered()
{

}

void MainWindow::on_action_Settings_triggered()
{

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
    spr_manager::SpriteManager   & sprman = spr_manager::SpriteManager::Instance();
    spr_manager::SpriteContainer * sprcnt = sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::PACK );

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

void MainWindow::InitAnimScene()
{
    m_previewrender.Reset();
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



void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    TreeElement * pcur = static_cast<TreeElement*>(index.internalPointer());
    if(!pcur)
        return;

    pcur->OnClicked();

    switch( pcur->getNodeDataTy() )
    {
        //Open the appropriate tab
    case eTreeElemDataType::sprite:
        {
            Sprite * spr = static_cast<Sprite*>(pcur);
            DisplayPropertiesPage(spr);
            break;
        }
    case eTreeElemDataType::palette:
        {
            PaletteContainer    * pal = static_cast<PaletteContainer*>(pcur);
            Sprite              * spr = pal->parentSprite();
            DisplayPalettePage(spr);
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
            DisplayAnimTablePage(spr);
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
            DisplayAnimSequencePage(spr,seq);
            break;
        }
    case eTreeElemDataType::animGroup:
        {
            AnimGroup   * grp = static_cast<AnimGroup*>(pcur);
            Sprite      * spr = grp->parentSprite();
            DisplayAnimGroupPage(spr);
            break;
        }
    default:
        HideAllTabs();
    };
    ui->tv_sprcontent->viewport()->update();
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

    //QMenu * menu = new QMenu(ui->tv_sprcontent);
    spr_manager::SpriteContainer * pcnt = getContainer();
    Q_ASSERT(pcnt);
    TreeElement * elem = static_cast<TreeElement*>(entry.internalPointer());
    Q_ASSERT(elem);

    TVSpritesContextMenu * menu = new TVSpritesContextMenu( this, entry, ui->tv_sprcontent);
    connect( menu, SIGNAL(afterclosed()), menu, SLOT(deleteLater()) );
    return menu;
}

void MainWindow::on_btnSeqPlay_clicked()
{
//    if(!m_curanim)
//        return;
    qDebug() << "MainWindow::on_btnSeqPlay_clicked(): Pressed play!\n";
    //m_curanim->Play();
}

void MainWindow::on_btnSeqStop_clicked()
{
//    if(!m_curanim)
//        return;
    qDebug() << "MainWindow::on_btnSeqStop_clicked(): Pressed stop!\n";
    //m_curanim->Stop();
}

void MainWindow::on_chkAnimSeqLoop_toggled(bool checked)
{
//    if(!m_curanim)
//        return;
    //m_curanim->loop(checked);
}

void MainWindow::on_btnImageCrop_clicked()
{
    Image * pimg = static_cast<Image*>(ui->tv_sprcontent->currentIndex().internalPointer());

    if(pimg)
    {
        DiagSingleImgCropper cropper(this, pimg);
        cropper.setModal(true);
        cropper.show();
    }
    else
    {
        QApplication::beep();
        ui->statusBar->setStatusTip("Can't crop! No valid image selected!");
        qWarning("MainWindow::on_btnImageCrop_clicked(): Crop clicked, but no valid images was selected!");
    }
}

void MainWindow::on_tblviewImages_clicked(const QModelIndex &index)
{
    Image * img = static_cast<Image *>(index.internalPointer());
    if(!index.internalPointer() || !img)
    {
        ui->lbl_imgpreview->setPixmap(m_imgNoImg);
        return;
    }
    ui->lbl_imgpreview->setPixmap(ImageToPixmap(img->makeImage(img->parentSprite()->getPalette()), ui->lbl_imgpreview->size()));

    //#TODO: Update image details if needed
}

void MainWindow::on_tblframeparts_clicked(const QModelIndex &/*index*/)
{
//    setupFrameEditPageForPart(currentFrame(), currentTblFrameParts());
}

Sprite * MainWindow::currentSprite()
{
    TreeElement * elem = static_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());
    if(!elem)
        return nullptr;
    return elem->parentSprite();
}

MFrame *MainWindow::currentFrame()
{
    TreeElement * elem = static_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());

    //In cases where we're in the treeview on a child entry, get the parent!
    if(elem && elem->getNodeDataTy() == eTreeElemDataType::framepart)
        return static_cast<MFrame*>(elem->parentNode());

    return static_cast<MFrame*>(elem);
}

Image *MainWindow::currentImage()
{
    Image * elem = static_cast<Image*>(ui->tv_sprcontent->currentIndex().internalPointer());
    return elem;
}

AnimSequence *MainWindow::currentAnimSequence()
{
    AnimSequence * elem = static_cast<AnimSequence*>(ui->tv_sprcontent->currentIndex().internalPointer());
    return elem;
}

eTreeElemDataType MainWindow::currentEntryType()
{
    TreeElement * elem = static_cast<TreeElement*>(ui->tv_sprcontent->currentIndex().internalPointer());
    if(elem)
        return elem->getNodeDataTy();
    return eTreeElemDataType::None;
}

MFramePart * MainWindow::currentTblFrameParts()
{
    MFramePart * elem = static_cast<MFramePart*>(ui->tblframeparts->currentIndex().internalPointer());
    return elem;
}

Image * MainWindow::currentTblImages()
{
    Image * elem = static_cast<Image*>(ui->tblviewImages->currentIndex().internalPointer());
    return elem;
}

void MainWindow::on_btnExportPalette_clicked()
{
    using namespace spr_manager;
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprites to dump from!")) );
        return;
    }

    QString selectedfilter;
    ePaletteDumpType type;
    qInfo("MainWindow::on_btnExportPalette_clicked(): Exporting palette!");
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Save Palette Dump As"),
                        QString(),
                        PaletteFilter,
                        &selectedfilter);

    if(filename.isNull())
        return;

    if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal) &&
       !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal), Qt::CaseInsensitive))
    {
        //filename.append(".pal");
        type = ePaletteDumpType::RIFF_Pal;
    }
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal) &&
            !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal), Qt::CaseInsensitive))
    {
        //filename.append(".txt");
        type = ePaletteDumpType::TEXT_Pal;
    }
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL) &&
            !filename.endsWith(GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL), Qt::CaseInsensitive))
    {
        //filename.append(".gpl");
        type = ePaletteDumpType::GIMP_PAL;
    }

    Q_ASSERT(type < ePaletteDumpType::INVALID);

    try
    {
        spr_manager::SpriteManager::Instance().DumpPalette(spr, filename, type);
    }
    catch(const std::exception & e)
    {
        ShowStatusErrorMessage(QString(tr("Error: %1")).arg(e.what()) );
        return;
    }
    catch(...)
    {
        std::rethrow_exception(std::current_exception());
        return;
    }

    ShowStatusMessage( QString(tr("Palette dumped!")) );
}

void MainWindow::on_btnImportPalette_clicked()
{
    using namespace spr_manager;
    Sprite * spr = currentSprite();
    if( !spr )
    {
        ShowStatusErrorMessage(QString(tr("No sprite to import to!")) );
        return;
    }

    QString selectedfilter;
    ePaletteDumpType type;
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Import Palette File"),
                                                    QString(),
            PaletteFilter + ";;" + GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL), //allow loading a PNG for its palette!
                                                    &selectedfilter );
    if(filename.isNull())
        return;

    if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::RIFF_Pal))
        type = ePaletteDumpType::RIFF_Pal;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::TEXT_Pal))
        type = ePaletteDumpType::TEXT_Pal;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::GIMP_PAL))
        type = ePaletteDumpType::GIMP_PAL;
    else if(selectedfilter == GetPaletteFileFilterString(ePaletteDumpType::PNG_PAL))
        type = ePaletteDumpType::PNG_PAL;
    Q_ASSERT(type < ePaletteDumpType::INVALID);

    try
    {
        spr_manager::SpriteManager::Instance().ImportPalette(spr, filename, type);
    }
    catch(const std::exception & e)
    {
        ShowStatusErrorMessage(QString(tr("Error: %1")).arg(e.what()) );
        return;
    }
    catch(...)
    {
        std::rethrow_exception(std::current_exception());
        return;
    }

    ShowStatusMessage( QString(tr("Palette imported!")) );

    //Refresh property page
    DisplayPropertiesPage(spr);
}

void MainWindow::on_btnEditPalette_clicked()
{
    //FUCK THIS FOR NOW
    //I SPENT ALL DAY ON IT AND MANAGED TO DESTROY IT COMPLETELY
    //I GUESS PEOPLE WILL EDIT THE PALETTES USING AN EXTERNAL TOOL!
    //C:

//    Sprite * spr = currentSprite();
//    if( !spr )
//    {
//        ShowStatusErrorMessage(QString(tr("No valid sprite to edit the palette of!")) );
//        return;
//    }

//    PaletteEditor paledit(this);
//    paledit.setModal(true);
//    paledit.setPalModel(spr->getPaletteModel());
//    paledit.exec();
}

void MainWindow::on_btnFrmExport_clicked()
{
    const MFrame * pfrm = m_frmeditor->getFrame();
    if(!pfrm)
    {
        ShowStatusErrorMessage(tr("Couldn't export, no frame loaded!"));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;


    QImage img( qMove(pfrm->AssembleFrame(0, 0, pfrm->calcFrameBounds() )) );
    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported assembled frame to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}

void MainWindow::on_btnSeqExport_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Images Sequence : Pick name+path first image!"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
    {
        return;
    }

    int rmpast = filename.size() - filename.lastIndexOf('.');
    if( rmpast > 0 && rmpast < filename.size() )
        filename.chop(rmpast);
    QVector<QImage> sequence = m_previewrender.DumpSequence();

    if(sequence.isEmpty())
    {
        ShowStatusErrorMessage(tr("Error: No sequence was loaded for export!"));
    }

    int cntimg = 0;
    for(; cntimg < sequence.size(); ++cntimg )
        sequence[cntimg].save( QString("%1_%2.png").arg(filename).arg(cntimg) );

    ShowStatusMessage(QString(tr("Exported %1 images!")).arg(cntimg));
}

void MainWindow::on_btnImagesExport_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;

    Image * pimg = nullptr;

    if(ui->tblviewImages->currentIndex().isValid())
        pimg = static_cast<Image*>( ui->tblviewImages->currentIndex().internalPointer() );

    if(!pimg)
    {
        ShowStatusErrorMessage(tr("Error: No image selected for export!"));
        return;
    }

    QImage img = qMove( pimg->makeImage(pimg->parentSprite()->getPalette()) );
    img.save(filename, "PNG");
}

//===================================================================================================================
// TVSpritesContextMenu
//===================================================================================================================
TVSpritesContextMenu::TVSpritesContextMenu( MainWindow * mainwindow,
                                            const QModelIndex & item,
                                            QWidget * parent)
    :QMenu(parent),
      m_pitem(static_cast<TreeElement*>(item.internalPointer())),
      m_itemidx(item),
      m_pmainwindow(mainwindow)
{
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


    if(m_pmainwindow->ui->tv_sprcontent->currentIndex() == m_itemidx)
    {
        m_pmainwindow->HideAllTabs(); //Deselect everything if we're deleting the current entry
        m_pmainwindow->ui->tblframeparts->setModel(nullptr);
        QModelIndex parentidx = m_pmainwindow->ui->tv_sprcontent->currentIndex().parent();
        if(parentidx.isValid())
            m_pmainwindow->ui->tv_sprcontent->setCurrentIndex(parentidx);
        else
            m_pmainwindow->ui->tv_sprcontent->setCurrentIndex(QModelIndex());
    }
    m_pmainwindow->ui->tv_sprcontent->model()->removeRow( m_itemidx.row(), m_itemidx.parent() );

    m_pmainwindow->ShowStatusMessage( QString(tr("Entry removed!")) );
    close();
}











