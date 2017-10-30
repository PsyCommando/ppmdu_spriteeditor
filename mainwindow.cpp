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
    setWindowTitle(QString("PMD2 Sprite Muncher v%1").arg(QString(GIT_MAJORMINOR_VERSION)));

    //UI init
    ui->setupUi(this);

    //Parse settings!
    readSettings();

    //Connect stuff
    connect( ui->chkAnimSeqLoop,&QCheckBox::toggled,    &m_previewrender, &SceneRenderer::loopChanged );
    connect( ui->btnSeqPlay,    &QPushButton::clicked,  &m_previewrender, &SceneRenderer::startAnimUpdates );
    connect( ui->btnSeqStop,    &QPushButton::clicked,  &m_previewrender, &SceneRenderer::stopAnimUpdates );

    connect( &m_previewrender, &SceneRenderer::rangechanged, [&](int min, int max)
    {
        //Set this so we don't end up with messed up logic!
        ui->spinCurFrm->blockSignals(true);
        ui->spinCurFrm->setRange(min, max);
        ui->spinCurFrm->blockSignals(false);

        ui->sldrAnimSeq->blockSignals(true);
        ui->sldrAnimSeq->setRange(min, max);
        ui->sldrAnimSeq->blockSignals(false);
    });

    connect( &m_previewrender, &SceneRenderer::framechanged, [&](int frm)
    {
        //Set this so we don't end up with messed up logic!
        ui->spinCurFrm->blockSignals(true);
        ui->spinCurFrm->setValue(frm);
        ui->spinCurFrm->blockSignals(false);

        ui->sldrAnimSeq->blockSignals(true);
        ui->sldrAnimSeq->setValue(frm);
        ui->sldrAnimSeq->blockSignals(false);
    });

    connect(ui->sldrAnimSeq, &QSlider::sliderMoved, [&](int val)
    {
        ui->spinCurFrm->blockSignals(true);
        ui->spinCurFrm->setValue(val);
        ui->spinCurFrm->blockSignals(false);
        m_previewrender.setCurrentFrame(val);
    });

    ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
    ui->tv_sprcontent->setModel( &spr_manager::SpriteManager::Instance() );
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

    ui->tabMain->removeWidget(ui->tabAnimTable);
    ui->tabMain->removeWidget(ui->tabEfx);
    ui->tabMain->removeWidget(ui->tabFrame);
    ui->tabMain->removeWidget(ui->tabSequence);
    ui->tabMain->removeWidget(ui->tabProperties);
    ui->tabMain->removeWidget(ui->tabWelcome);
    ui->tabMain->removeWidget(ui->tabImages);

    ui->tabAnimTable->hide();
    ui->tabEfx->hide();
    ui->tabFrame->hide();
    ui->tabSequence->hide();
    ui->tabProperties->hide();
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
    ui->tabMain->insertWidget(0, ptab );
    qDebug() << "MainWindow::ShowATab(): Adding tab to be displayed!\n";
    ptab->show();
    //ptab->setFocus();
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
    ui->tblOverview->setModel(spr->overviewModel());

    ShowATab(ui->tabProperties);
}

void MainWindow::DisplayMFramePage(Sprite *spr, MFrame * frm)
{
    Q_ASSERT(spr && frm);
    qDebug() << "MainWindow::DisplayMFramePage(): Showing frame page!\n";
    ui->tblframeparts->setModel(frm->getModel());
    ui->tblframeparts->setItemDelegate(&(frm->itemDelegate()));
    ui->tblframeparts->setEditTriggers(QTableView::EditTrigger::AllEditTriggers);
    ui->tblframeparts->setSizeAdjustPolicy(QTableView::SizeAdjustPolicy::AdjustToContentsOnFirstShow);
    ShowATab(ui->tabFrame);
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

    //Setup the callbacks
    connect(frm->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &,const QModelIndex &, const QVector<int>&)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsInserted, [&](const QModelIndex & /*parent*/, int /*first*/, int /*last*/)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsMoved, [&](const QModelIndex & /*parent*/,
                                                                int /*start*/,
                                                                int /*end*/,
                                                                const QModelIndex & /*destination*/,
                                                                int /*row*/)
    {
        m_frmeditor->updateParts();
        ui->gvFrame->update();
    });
    connect(frm->getModel(), &QAbstractItemModel::rowsRemoved, [&](const QModelIndex & /*parent*/,
                                                                int /*first*/,
                                                                int /*last*/)
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
        ui->spbFrmZoom->setValue(val + ui->spbFrmZoom->value());
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
}

void MainWindow::DisplayAnimSequencePage(Sprite *spr, AnimSequence * aniseq)
{
    Q_ASSERT(spr && aniseq);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Showing anim sequence page!\n";
    ShowATab(ui->tabSequence);
    //m_previewrender.setScene(spr, aniseq->nodeIndex());
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Instanciated anim viewer!\n";

    //ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
    //ui->gvAnimSeqViewport->centerOn(m_previewrender.getAnimSprite());
    //m_previewrender.getAnimSprite()->setScale(2.0);
    ui->tblseqfrmlst->setModel(aniseq->getModel());
    ui->tblseqfrmlst->setItemDelegate(aniseq->getDelegate());
    ui->tblseqfrmlst->resizeRowsToContents();
    ui->tblseqfrmlst->resizeColumnsToContents();

    //connect( aniseq->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    //{
    //    m_previewrender.reloadAnim();
    //    ui->gvAnimSeqViewport->update();
    //});
    InstallAnimPreview(ui->gvAnimSeqViewport, spr, aniseq);
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Scene set!\n";
}

void MainWindow::InstallAnimPreview(QGraphicsView * viewport, Sprite *spr, AnimSequence * aniseq)
{
    qDebug() << "MainWindow::InstallAnimPreview(): Displaying animation..\n";
    m_previewrender.setScene(spr, aniseq->nodeIndex());
    viewport->setScene(&m_previewrender.getAnimScene());
    viewport->centerOn(m_previewrender.getAnimSprite());
    m_previewrender.getAnimSprite()->setScale(2.0);

    connect( aniseq->getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    {
        m_previewrender.reloadAnim();
        viewport->update();
    });
}

void MainWindow::DisplayAnimTablePage(Sprite * spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabAnimTable);
    ui->tvAnimTbl->setModel(spr->getAnimTable().getModel());
    ui->tvAnimTblAnimSeqs->setModel(nullptr);
    ui->lvAnimTblAnimSeqList->setModel(spr->getAnimSequences().getPickerModel());

    connect( spr->getAnimTable().getModel(), &QAbstractItemModel::dataChanged, [&](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
    {
        ui->tvAnimTbl->repaint();
        ui->tvAnimTblAnimSeqs->repaint();
    });

    connect(ui->tvAnimTbl, &QTableView::activated, [spr,this](const QModelIndex &index)
    {
        Q_ASSERT(spr);
        AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(index)));
        Q_ASSERT(grp);
        ui->tvAnimTblAnimSeqs->setModel(grp->getModel());
    });

    connect(ui->tvAnimTbl, &QTableView::clicked, [spr,this](const QModelIndex &index)
    {
        Q_ASSERT(spr);
        AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(index)));
        Q_ASSERT(grp);
        ui->tvAnimTblAnimSeqs->setModel(grp->getModel());
    });

    connect(ui->tvAnimTblAnimSeqs, &QTableView::clicked, [&](const QModelIndex & index)
    {
        if( ui->chkAnimTblAutoPlay->isChecked() )
        {
            Q_ASSERT(spr);
            AnimGroup * grp = const_cast<AnimGroup *>(static_cast<const AnimGroup *>(spr->getAnimTable().getItem(ui->tvAnimTbl->currentIndex())));
            Q_ASSERT(grp);
            AnimSequence * seq = spr->getAnimSequence( grp->seqSlots()[index.row()] );
            InstallAnimPreview(ui->gvAnimTablePreview, spr, seq);
        }
    });

    connect(ui->lvAnimTblAnimSeqList, &QTreeView::clicked, [&](const QModelIndex & index)
    {
        if( ui->chkAnimTblAutoPlay->isChecked() )
        {
            Q_ASSERT(spr);
            InstallAnimPreview(ui->gvAnimTablePreview, spr, spr->getAnimSequence(index.row()));
        }
    });
}

void MainWindow::DisplayPalettePage(Sprite *spr)
{
    Q_ASSERT(spr);
    //ShowATab(ui->tabpal);
}

void MainWindow::DisplayEffectsPage(Sprite *spr)
{
    Q_ASSERT(spr);
    ShowATab(ui->tabEfx);
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
        //spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
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
    setWindowFilePath(sprcnt->GetContainerSrcPath());
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


// *********************************
//  Properties Tab
// *********************************
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


// *********************************
//  Image Tab
// *********************************
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

void MainWindow::ShowProgressDiag(QFuture<void> &task)
{
    m_progress.setFuture(task);
    m_progress.setModal(true);
    m_progress.show();
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

void MainWindow::on_btnImagesExport_clicked()
{
    Image * pimg = nullptr;
    if(ui->tblviewImages->currentIndex().isValid())
        pimg = static_cast<Image*>( ui->tblviewImages->currentIndex().internalPointer() );

    if(!pimg)
    {
        ShowStatusErrorMessage(tr("Error: No image selected for export!"));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
                        tr("Export Image"),
                        QString(),
                        "PNG image (*.png)");

    if(filename.isNull())
        return;

    QImage img = qMove( pimg->makeImage(pimg->parentSprite()->getPalette()) );

    if(img.save( filename, "PNG" ))
        ShowStatusMessage(QString(tr("Exported image to %1!")).arg(filename));
    else
        ShowStatusErrorMessage(tr("Couldn't export, saving failed!"));
}

// *********************************
//  Frame Tab
// *********************************
void MainWindow::on_btnFrmRmPart_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }
    MFramePart* ppart = static_cast<MFramePart*>(ind.internalPointer());
    MFrame    * pfrm  = static_cast<MFrame*>(ppart->parentNode());
    Q_ASSERT(ppart && pfrm);

    ui->tblframeparts->setCurrentIndex(QModelIndex());
    if(pfrm->removeChildrenNodes(ind.row(), 1))
        ShowStatusMessage(tr("Removed part!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmAdPart_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();

    Q_ASSERT(curframe);
    int insertpos = 0;

    if(ind.isValid())
        insertpos = ind.row();
    else
        insertpos = (curframe->nodeChildCount() > 0)? (curframe->nodeChildCount() - 1) : 0;

    if(curframe->getModel()->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended part!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmMvUp_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    int destrow = (ind.row() > 0)? ind.row() - 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curframe->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Part moved up!"));
        else
            ShowStatusErrorMessage(tr("Failed to move part!"));
    }

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmMvDown_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    int destrow = (ind.row() < curframe->nodeChildCount()-2 )? ind.row() + 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curframe->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Part moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move part!"));
    }

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_btnFrmDup_clicked()
{
    QModelIndex ind = ui->tblframeparts->currentIndex();
    MFrame      *curframe = currentFrame();
    Q_ASSERT(curframe);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No part selected!"));
        return;
    }

    MFramePart tmppart = *(static_cast<MFramePart*>(ind.internalPointer()));

    int insertpos = ind.row();
    if(curframe->getModel()->insertRow(insertpos))
    {
        MFramePart * pnewfrm = static_cast<MFramePart *>(curframe->getItem( curframe->getModel()->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = tmppart;
        ShowStatusMessage(tr("Duplicated part!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
}

void MainWindow::on_cmbFrmQuickPrio_currentIndexChanged(int index)
{
    Q_ASSERT(false); //#TODO: Make this work!

    m_frmeditor->updateParts();
    ui->gvFrame->update();
    ui->tblframeparts->update();
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


// *********************************
//  Anim Sequence Tab
// *********************************
void MainWindow::on_btnSeqAddFrm_clicked()
{
    QModelIndex     ind       = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    int insertpos = 0;

    if(ind.isValid())
        insertpos = ind.row();
    else
        insertpos = (curseq->nodeChildCount() > 0)? (curseq->nodeChildCount() - 1) : 0;

    if(curseq->getModel()->insertRow(insertpos))
    {
        ShowStatusMessage(tr("Appended animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Insertion failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqRemFrm_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No frame selected!"));
        return;
    }
    AnimFrame       * pafrm = static_cast<AnimFrame*>(ind.internalPointer());
    AnimSequence    * pseq  = static_cast<AnimSequence*>(pafrm->parentNode());
    Q_ASSERT(pseq && pafrm);

    ui->tblseqfrmlst->setCurrentIndex(QModelIndex());
    if(pseq->removeChildrenNodes(ind.row(), 1))
        ShowStatusMessage(tr("Removed animation frame!"));
    else
        ShowStatusErrorMessage(tr("Removal failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqMvUp_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() > 0)? ind.row() - 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curseq->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqMvDown_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    int destrow = (ind.row() < curseq->nodeChildCount()-1 )? ind.row() + 1 : ind.row();
    if(destrow != ind.row())
    {
        if(curseq->getModel()->moveRow(ind.parent(), ind.row(), ind.parent(), destrow))
            ShowStatusMessage(tr("Animation frame moved down!"));
        else
            ShowStatusErrorMessage(tr("Failed to move frame!"));
    }

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
}

void MainWindow::on_btnSeqDup_clicked()
{
    QModelIndex ind = ui->tblseqfrmlst->currentIndex();
    AnimSequence    *curseq = currentAnimSequence();
    Q_ASSERT(curseq);
    if(!ind.isValid())
    {
        ShowStatusErrorMessage(tr("No animation frame selected!"));
        return;
    }

    AnimFrame tmpfrm = *(static_cast<AnimFrame*>(ind.internalPointer()));

    int insertpos = ind.row();
    if(curseq->getModel()->insertRow(insertpos))
    {
        AnimFrame * pnewfrm = static_cast<AnimFrame *>(curseq->getItem( curseq->getModel()->index(insertpos, 0, QModelIndex()) ));
        Q_ASSERT(pnewfrm);
        (*pnewfrm) = tmpfrm;
        ShowStatusMessage(tr("Duplicated animation frame!"));
    }
    else
        ShowStatusErrorMessage(tr("Duplication failed!"));

    m_previewrender.reloadAnim();
    ui->tblseqfrmlst->update();
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


//
//
//
void MainWindow::on_btnAnimTblMoveSeq_clicked()
{

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






void MainWindow::on_tblframeparts_clicked(const QModelIndex &index)
{

}



void MainWindow::on_tv_sprcontent_activated(const QModelIndex &index)
{
    this->on_tv_sprcontent_clicked(index);
}
