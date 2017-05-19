#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QThread>
#include <QThreadPool>

#include <dialogexport.hpp>
#include <diagsingleimgcropper.hpp>
#include <dialogabout.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_previewrender(true)
{
    m_pStatusFileType.reset(new QLabel("     "));
    ui->setupUi(this);
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
    DisplayStartScreen();
    InitAnimScene();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabpal));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabseq));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabproperties));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabWelcome));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabanimgrp));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabimage));
    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabImages));

    ui->tabanims->hide();
    ui->tabeffects->hide();
    ui->tabframeseditor->hide();
    ui->tabpal->hide();
    ui->tabseq->hide();
    ui->tabproperties->hide();
    ui->tabWelcome->hide();
    ui->tabanimgrp->hide();
    ui->tabimage->hide();
    ui->tabImages->hide();

    ui->tabMain->setUpdatesEnabled(true);
}

void MainWindow::ShowATab(QWidget *ptab)
{
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
}

void MainWindow::DisplayPropertiesPage(Sprite * spr)
{
    qDebug() << "MainWindow::DisplayPropertiesPage(): Showing properties tab!\n";
    ui->lblPropPreview->setPixmap(spr->MakePreviewFrame().scaled( ui->lblPropPreview->size(), Qt::KeepAspectRatio) );
    ui->lbl_test_palette->setPixmap(spr->MakePreviewPalette());
    ShowATab(ui->tabproperties);
}

void MainWindow::DisplayAnimFramePage(Sprite *spr)
{
    ShowATab(ui->tabframeseditor);
}

void MainWindow::DisplayAnimSequencePage(Sprite *spr, AnimSequence * aniseq)
{
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Showing anim sequence page!\n";
    ShowATab(ui->tabseq);
    //m_curanim.reset(new AnimViewerManager(&m_animscene, ui->chkAnimSeqLoop->isChecked()));
    //m_curanim->setSequence(aniseq, spr);
    m_previewrender.setScene(spr, aniseq->childNumber());
    qDebug() << "MainWindow::DisplayAnimSequencePage(): Instanciated anime viewer!\n";
    ui->gvAnimSeqViewport->setScene(&m_previewrender.getAnimScene());
    ui->gvAnimSeqViewport->centerOn(m_previewrender.getAnimSprite());
    m_previewrender.getAnimSprite()->setScale(2.0);
    ui->tblseqfrmlst->setModel(&aniseq->getModel());

    qDebug() << "MainWindow::DisplayAnimSequencePage(): Scene set!\n";
}

void MainWindow::DisplayAnimTablePage(Sprite * spr)
{
    ShowATab(ui->tabanims);
}

void MainWindow::DisplayPalettePage(Sprite *spr)
{
    ShowATab(ui->tabpal);
}

void MainWindow::DisplayEffectsPage(Sprite *spr)
{
    ShowATab(ui->tabeffects);
}

void MainWindow::DisplayAnimGroupPage(Sprite *spr)
{

    ShowATab(ui->tabanimgrp);
}

void MainWindow::DisplayImagePage(Sprite *spr, Image * img)
{
    qDebug() << "MainWindow::DisplayImagePage(): Displaying image page!\n";
    QPixmap apixm = qMove(img->makePixmap(spr->getPalette()).scaled( ui->lbl_imgpreview->size(), Qt::KeepAspectRatio));
    qDebug() << "MainWindow::DisplayImagePage(): Pixmap generated!\n";
    ui->lbl_imgpreview->setPixmap(apixm);
    qDebug() << "MainWindow::DisplayImagePage(): Pixmap assigned!\n";
    ShowATab(ui->tabimage);
}

void MainWindow::DisplayImageListPage(Sprite *spr, ImageContainer *pimgs)
{
    qDebug() << "MainWindow::DisplayImageListPage(): Displaying images list page!\n";
    ui->tblviewImages->setModel(pimgs->getModel());
    qDebug() << "MainWindow::DisplayImageListPage(): Model set!\n";
    ShowATab(ui->tabImages);
}

void MainWindow::LoadContainer(const QString &path)
{
    spr_manager::SpriteManager::Instance().OpenContainer(path);
    updateActions();
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
    spr_manager::SpriteManager::Instance().ImportContainer(path);
    updateActions();
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
}

void MainWindow::on_action_Open_triggered()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            tr("Pack Files (*.bin);;WAN Sprite (*.wan *.wat)"));

    if (!fileName.isEmpty())
    {
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

        //Open
        qInfo() <<"Opening file " <<fileName <<"!\n";
        spr_manager::SpriteContainer * sprcnt = sprman.OpenContainer(fileName);
        qInfo() <<fileName <<" loaded!\n";
        m_lastSavePath = fileName;

        //Display!
        if(sprcnt && sprcnt->hasChildren())
            DisplayPropertiesPage(&sprcnt->GetSprite(0));
        setupListView();
        updateActions();
    }
    else
        qWarning() << "Got an empty path!\n";
}

void MainWindow::on_action_Quit_triggered()
{
    qDebug() <<"Quitting!\n";
    QApplication::instance()->exit();
    qDebug() <<"After exit call!\n";
}

void MainWindow::on_tv_sprcontent_itemSelectionChanged()
{

}

void MainWindow::on_tv_sprcontent_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    qDebug() << "MainWindow::on_tv_sprcontent_currentItemChanged(): ";
    if(current)
        qDebug() <<"Selected element #" <<current->parent()->indexOfChild(current) <<"!\n";
    else
        qDebug() <<"Unselected!\n";
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
    switch( pcur->getDataTy() )
    {
        //Open the appropriate tab
    case eTreeElemDataType::sprite:
        {
            Sprite * spr = static_cast<Sprite*>(pcur);
            DisplayPropertiesPage(spr);
            break;
        }
    };
    qDebug() << "MainWindow::on_tv_sprcontent_expanded(): Enabling updates!\n";
    ui->tv_sprcontent->setUpdatesEnabled(true);
}

void MainWindow::on_tv_sprcontent_itemClicked(QTreeWidgetItem *item, int column)
{

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
            filetypestr = tr("Pack Files (*.bin)");
            break;
        }
    case spr_manager::SpriteContainer::eContainerType::WAN:
        {
            filetypestr = tr("WAN Sprite (*.wan)");
            break;
        }
    case spr_manager::SpriteContainer::eContainerType::WAT:
        {
            filetypestr = tr("WAT Sprite (*.wat)");
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

void MainWindow::on_actionSprite_triggered()
{

}

void MainWindow::on_actionSprite_Pack_File_triggered()
{

}

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
    DialogAbout abt;
    abt.show();
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
}

void MainWindow::on_actionNewSprite_Pack_File_triggered()
{
    spr_manager::SpriteManager   & sprman = spr_manager::SpriteManager::Instance();
    spr_manager::SpriteContainer * sprcnt = sprman.NewContainer( spr_manager::SpriteContainer::eContainerType::PACK );

    //Display!
    updateActions();
    setupListView();
}

void MainWindow::setupListView()
{
    spr_manager::SpriteManager & sprman = spr_manager::SpriteManager::Instance();
    if( sprman.ContainerIsSingleSprite() )
    {
        ui->tv_sprcontent->setRootIsDecorated(false);
        ui->tv_sprcontent->expandToDepth(1);
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
        sprman.SaveContainer(path);
        qInfo() <<path <<" saved!\n";
        m_lastSavePath = path;
    }
    else
        qWarning() << "Got an empty path!\n";
}

void MainWindow::InitAnimScene()
{
//    if( m_curanim )
//    {
//        m_curanim->Stop();
//        m_animscene.removeItem( m_curanim->getPixPtr() );
//        m_curanim.reset();
//    }

//    m_animscene.clear();
//    m_animscene.setSceneRect(-256, -256, 512, 512 );
//    m_animscene.setBackgroundBrush(QBrush(Qt::darkGray));
//    m_animscene.addLine(0, -256, 0, 256);
//    m_animscene.addLine(-256, 0, 256, 0);
    m_previewrender.Reset();
}

void MainWindow::on_tv_sprcontent_clicked(const QModelIndex &index)
{
    TreeElement * pcur = static_cast<TreeElement*>(index.internalPointer());
    if(!pcur)
        return;

    pcur->OnClicked();

    switch( pcur->getDataTy() )
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
            DisplayAnimFramePage(spr);
            break;
        }
    case eTreeElemDataType::image:
        {
            Image  * img = static_cast<Image*>(pcur);
            Sprite * spr = img->parentSprite();
            DisplayImagePage(spr,img);
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
}

void MainWindow::on_tv_sprcontent_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex entry = ui->tv_sprcontent->indexAt(pos);
    if( entry.isValid() )
    {
        //This is triggered when clicking on an item
        //remove/copy/duplicate/etc
    }
    else
    {
        //This is triggered when clicking in the white unused space
        //Add sprite/frame/sequence/image/etc
    }
}

void MainWindow::on_tblviewImagesTest_doubleClicked(const QModelIndex &index)
{
    Image * pcur = static_cast<Image*>(index.internalPointer());

    if(!pcur)
        return;

    DisplayImagePage( pcur->parentSprite(), pcur );
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
