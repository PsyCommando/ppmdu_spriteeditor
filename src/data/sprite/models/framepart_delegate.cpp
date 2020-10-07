#include "framepart_delegate.hpp"
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <src/data/sprite/sprite.hpp>
#include <src/data/sprite/frame.hpp>
#include <src/data/sprite/framepart.hpp>
#include <src/data/sprite/imagescontainer.hpp>

const QString MinusOneImgRes(":/resources/imgs/minus_one.png");
const QString MFramePart_NoDrawFrame{"ID:-1 No Draw Frame"};

//=====================================================================================
//  MFrameDelegate
//=====================================================================================
MFramePartDelegate::MFramePartDelegate(MFrame *frm, QObject *parent)
    :QStyledItemDelegate(parent), m_minusone(MinusOneImgRes)
{
    m_pfrm = frm;
}

MFramePartDelegate::MFramePartDelegate(const MFramePartDelegate & cp)
    :QStyledItemDelegate(cp.parent()), m_minusone(MinusOneImgRes)
{
    operator=(cp);
}

MFramePartDelegate::MFramePartDelegate(MFramePartDelegate && mv)
    :QStyledItemDelegate(mv.parent()), m_minusone(MinusOneImgRes)
{
    operator=(mv);
}

MFramePartDelegate & MFramePartDelegate::operator=(const MFramePartDelegate & cp)
{
    setParent(cp.parent());
    m_pfrm    = cp.m_pfrm;
    return *this;
}

MFramePartDelegate & MFramePartDelegate::operator=(MFramePartDelegate && mv)
{
    setParent(mv.parent());
    m_pfrm    = mv.m_pfrm;
    mv.m_pfrm = nullptr;
    return *this;
}

QSize MFramePartDelegate::sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
{
    if(!index.isValid())
    {
        Q_ASSERT(false);
        return QSize();
    }

    const QAbstractItemModel * model = index.model();
    if(model)
        return model->data(index, Qt::SizeHintRole).toSize();
    else
        return QSize();
}


MFramePartDelegate::~MFramePartDelegate()
{
}

QWidget *MFramePartDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QWidget *pedit = nullptr;
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::ImgID:
        pedit = makeImgSelect(parent, index);
    break;
    case eFramePartColumnsType::TileNum:
        pedit = makeTileIdSelect(parent, index.row());
    break;
    case eFramePartColumnsType::PaletteID:
        pedit = makePaletteIDSelect(parent, index.row());
    break;
    case eFramePartColumnsType::Offset:
        pedit = makeOffsetSelect(parent, index.row());
    break;
    case eFramePartColumnsType::Flip:
        pedit = makeFlipSelect(parent, index.row());
    break;
    case eFramePartColumnsType::RotNScaling:
        pedit = makeRotNScalingSelect(parent, index.row());
    break;
    case eFramePartColumnsType::Mode:
        pedit = makeModeSelect(parent, index.row());
    break;
    case eFramePartColumnsType::Priority:
        pedit = makePrioritySelect(parent, index.row());
    break;

    //Undefined cases
    //case eFramePartColumnsType::Preview:
    //case eFramePartColumnsType::TotalSize:
    //case eFramePartColumnsType::Unk0:
    //case eFramePartColumnsType::Mosaic:
    default:
        //nothing here!
        return QStyledItemDelegate::createEditor(parent,option,index);
        break;
    };

    //Common properties
    if(pedit != nullptr)
    {
        pedit->setAutoFillBackground(true);
        pedit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        pedit->setProperty(MFrame::PropPartID, index.row());
    }
    return pedit;
}

void MFramePartDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::ImgID:
        {
            //QComboBox   *pimglist = editor->findChild<QComboBox*>(ImgSelCmbBoxName());
            //QPushButton *pbtn     = editor->findChild<QPushButton*>(ImgSelBtnName());
            QComboBox * pimglist = static_cast<QComboBox*>(editor);
            Q_ASSERT(pimglist/* && pbtn*/);

            if(part->getFrameIndex() >= 0)
            {
//                pimglist->setEnabled(true);
                pimglist->setCurrentIndex(part->getFrameIndex() + 1); //add one, because 0 is reserved!!
                //pbtn->setChecked(false);
            }
            else
            {
//                pimglist->setDisabled(true);
                pimglist->setCurrentIndex(0); //index 0 is -1 frame!
                //pbtn->setChecked(true);
            }
            break;
        }
    case eFramePartColumnsType::Offset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(OffsetXSpinBoxName());
            QSpinBox *py = editor->findChild<QSpinBox*>(OffsetYSpinBoxName());
            Q_ASSERT(px && py);
            px->setValue(part->getXOffset());
            py->setValue(part->getYOffset());
            break;
        }
    case eFramePartColumnsType::Flip:
        {
            QCheckBox *pvf = editor->findChild<QCheckBox*>(VFlipChkBoxName());
            QCheckBox *phf = editor->findChild<QCheckBox*>(HFlipChkBoxName());
            Q_ASSERT(pvf && phf);

            if(part->isRotAndScalingOn())
            {
                pvf->setDisabled(true);
                phf->setDisabled(true);
                pvf->setToolTip(QString(tr("VFlip is disabled when rotation and scaling is turned on!")));
                phf->setToolTip(QString(tr("HFlip is disabled when rotation and scaling is turned on!")));
                pvf->setChecked(false);
                phf->setChecked(false);
            }
            else
            {
                pvf->setEnabled(true);
                phf->setEnabled(true);
                pvf->setToolTip(QString());
                phf->setToolTip(QString());
                pvf->setChecked(part->isVFlip());
                phf->setChecked(part->isHFlip());
            }
            break;
        }
    case eFramePartColumnsType::RotNScaling:
        {
            QCheckBox *prns = editor->findChild<QCheckBox*>(RotNScaleChkBoxName());
            Q_ASSERT(prns);
            prns->setChecked(part->isRotAndScalingOn());
            break;
        }
    case eFramePartColumnsType::PaletteID:
        {
            QSpinBox *ppalid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ppalid);
            ppalid->setValue(part->getPalNb());
            break;
        }
    case eFramePartColumnsType::Mode:
        {
            QComboBox *pobjmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pobjmode);
            pobjmode->setCurrentIndex(static_cast<int>(part->getObjMode()));
            break;
        }
    case eFramePartColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            pprio->setCurrentIndex(part->getPriority());
            break;
        }
    case eFramePartColumnsType::TileNum:
        {
            QSpinBox *ptid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ptid);
            ptid->setValue(part->getTileNum());
            break;
        }

        //Undefined cases
    //case eFramePartColumnsType::Preview:
    //case eFramePartColumnsType::TotalSize:
    //case eFramePartColumnsType::Unk0:
    default:
        QStyledItemDelegate::setEditorData(editor,index);
        break;
    };

    //
    //        int value = index.model()->data(index, Qt::EditRole).toInt();

    //        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    //        spinBox->setValue(value);
}

void MFramePartDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramePartColumnsType>(index.column()))
    {
    case eFramePartColumnsType::ImgID:
        {
            //QComboBox   *pimglist = editor->findChild<QComboBox*>(ImgSelCmbBoxName());
            QComboBox   *pimglist = static_cast<QComboBox*>(editor);
            //QPushButton *pbtn     = editor->findChild<QPushButton*>(ImgSelBtnName());
            Q_ASSERT(pimglist /*&& pbtn*/);
//            if(pbtn->isChecked())
//                model->setData(index, (-1), Qt::EditRole);
//            else
//                model->setData(index, pimglist->currentIndex(), Qt::EditRole);

            if(pimglist->currentIndex() == 0)
            {
                model->setData(index, -1, Qt::EditRole);
            }
            else
            {
                model->setData(index, (pimglist->currentIndex() - 1), Qt::EditRole);
            }
            break;
        }
    case eFramePartColumnsType::Offset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(OffsetXSpinBoxName());
            QSpinBox *py = editor->findChild<QSpinBox*>(OffsetYSpinBoxName());
            Q_ASSERT(px && py);
            px->interpretText();
            py->interpretText();
            QVariant val;
            val.setValue(QPair<int,int>(px->value(), py->value()));
            model->setData(index, val, Qt::EditRole);
            break;
        }
    case eFramePartColumnsType::Flip:
        {
            QCheckBox *pvf = editor->findChild<QCheckBox*>(VFlipChkBoxName());
            QCheckBox *phf = editor->findChild<QCheckBox*>(HFlipChkBoxName());
            Q_ASSERT(pvf && phf);
            if(pvf->isEnabled() && phf->isEnabled())
            {
                //We only save the value if the rot&scaling is disabled, aka when the checkboxes are enabled!
                QVariant val;
                val.setValue(QPair<bool,bool>(pvf->isChecked(), phf->isChecked()));
                model->setData(index, val, Qt::EditRole);
            }
            break;
        }
    case eFramePartColumnsType::RotNScaling:
        {
            QCheckBox *prns = editor->findChild<QCheckBox*>(RotNScaleChkBoxName());
            Q_ASSERT(prns);
            model->setData(index, prns->isChecked(), Qt::EditRole);
            //#TODO: add saving of additional rns data
            break;
        }
    case eFramePartColumnsType::PaletteID:
        {
            QSpinBox *ppalid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ppalid);
            ppalid->interpretText();
            model->setData(index, ppalid->value(), Qt::EditRole);
            break;
        }
    case eFramePartColumnsType::Mode:
        {
            QComboBox *pobjmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pobjmode);
            model->setData(index, pobjmode->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramePartColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            model->setData(index, pprio->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramePartColumnsType::TileNum:
        {
            QSpinBox *ptid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ptid);
            ptid->interpretText();
            model->setData(index, ptid->value(), Qt::EditRole);
            break;
        }

        //Undefined cases
    //case eFramePartColumnsType::Preview:
    //case eFramePartColumnsType::TotalSize:
    //case eFramePartColumnsType::Unk0:
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
        break;
    };

    //        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    //        spinBox->interpretText();
    //        int value = spinBox->value();

    //        model->setData(index, value, Qt::EditRole);
}

void MFramePartDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

QWidget *MFramePartDelegate::makeImgSelect(QWidget *parent, const QModelIndex & index) const
{
    const TreeNodeModel     *model      = static_cast<const TreeNodeModel*>(index.model());
    QComboBox               *imglstb    = new QComboBox(parent);
    const ImageContainer    *pcnt       = &(model->getOwnerSprite()->getImages());
    const Sprite            *spr        = model->getOwnerSprite();
    imglstb->setIconSize( QSize(32,32) );
    imglstb->setStyleSheet(pcnt->ComboBoxStyleSheet());

    //Fill the combobox!

    //Add nodraw frame
    imglstb->addItem( QPixmap::fromImage(m_minusone), MFramePart_NoDrawFrame, QVariant(-1)); //Set user data to -1
    //Add actual images!
    for( int cntimg = 0; cntimg < pcnt->nodeChildCount(); ++cntimg )
    {
        const Image* pimg = pcnt->getImage(cntimg);
        QPixmap pmap = QPixmap::fromImage(pimg->makeImage(spr->getPalette()));
        QString text = pimg->getImageDescription();
        imglstb->addItem( QIcon(pmap), text, QVariant(pimg->getID()) ); //Set user data to image's UID
    }
    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    return imglstb;
}

QWidget *MFramePartDelegate::makeFlipSelect(QWidget *parent, int /*row*/) const
{
    QFrame      *pselect    = new QFrame(parent);
    QBoxLayout  *play       = new QBoxLayout(QBoxLayout::Direction::LeftToRight, pselect);
    QCheckBox   *boxvflip   = new QCheckBox(QString(tr("V")),pselect);
    QCheckBox   *boxhflip   = new QCheckBox(QString(tr("H")),pselect);

    pselect->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding);
    boxvflip->setObjectName(VFlipChkBoxName());
    boxhflip->setObjectName(HFlipChkBoxName());
    pselect->setLayout(play);
    play->addWidget(boxvflip,0);
    play->addWidget(boxhflip,0);
    pselect->setMinimumWidth(72);
    pselect->setContentsMargins(1, 1, 1, 1);
    play->setContentsMargins(0,0,0,0);
    return pselect;
}

QWidget *MFramePartDelegate::makeOffsetSelect(QWidget *parent, int /*row*/) const
{
    QFrame      *pselect = new QFrame(parent);
    QBoxLayout  *play    = new QBoxLayout(QBoxLayout::Direction::LeftToRight, pselect);
    QSpinBox    *pxoff   = new QSpinBox(pselect);
    QSpinBox    *pyoff   = new QSpinBox(pselect);

    pselect->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding);
    pxoff->setObjectName(OffsetXSpinBoxName());
    pyoff->setObjectName(OffsetYSpinBoxName());
    pxoff->setRange(0, fmt::step_t::XOFFSET_MAX);
    pyoff->setRange(0, fmt::step_t::YOFFSET_MAX);

    pselect->setLayout(play);
    play->addWidget(pxoff);
    play->addWidget(pyoff);
    pselect->setContentsMargins(1, 1, 1, 1);
    play->setContentsMargins(0,0,0,0);
    pselect->setFocusProxy(pxoff);
    return pselect;
}

QWidget *MFramePartDelegate::makeRotNScalingSelect(QWidget *parent, int /*row*/) const
{
    QFrame      *pselect    = new QFrame(parent);
    QBoxLayout  *play       = new QBoxLayout(QBoxLayout::Direction::LeftToRight, pselect);
    QCheckBox   *rotnscale  = new QCheckBox(QString(tr("On")),pselect);
    QPushButton *btnset     = new QPushButton(QString(tr("...")), pselect);

    pselect->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Expanding);
    rotnscale-> setObjectName(RotNScaleChkBoxName());
    btnset->    setObjectName(RotNScaleBtnName());

    connect(rotnscale,  &QCheckBox::toggled,    btnset, &QPushButton::setEnabled);
    connect(btnset,     &QPushButton::clicked,  [=](bool)
    {
        //#TODO: Make the edit dialog for rotation and scaling!!
        QMessageBox::warning(parent, QString(tr("Unimplemented")), QString(tr("Feature not implemented yet!!! Sorry ^^;")),
                             QMessageBox::Ok,
                             QMessageBox::NoButton);
    });

    pselect->setLayout(play);
    play->addWidget(rotnscale);
    play->addWidget(btnset);
    pselect->setContentsMargins(1, 1, 1, 1);
    play->setContentsMargins(0,0,0,0);
    return pselect;
}

QWidget *MFramePartDelegate::makePaletteIDSelect(QWidget *parent, int /*row*/) const
{
    //#TODO: Make this into a combo box to pick the palette and get a preview!
    QSpinBox *ppalid = new QSpinBox(parent);
    ppalid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ppalid->setRange(0, fmt::step_t::PALID_MAX);
    return ppalid;
}

QWidget *MFramePartDelegate::makePrioritySelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(FRAME_PART_PRIORITY_NAMES);
    return pselect;
}

QWidget *MFramePartDelegate::makeTileIdSelect(QWidget *parent, int /*row*/) const
{
    QSpinBox *ptileid = new QSpinBox(parent);
    ptileid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ptileid->setRange(0, fmt::step_t::TILENUM_MAX);
    return ptileid;
}

QWidget *MFramePartDelegate::makeModeSelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(FRAME_PART_MODE_NAMES);
    pselect->setMinimumWidth(100);
    return pselect;
}
