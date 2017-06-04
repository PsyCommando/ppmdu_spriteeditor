#include "sprite_img.hpp"
#include <QImage>
#include <QVariant>
#include <QString>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QPixmap>
#include <QFontMetrics>

#include <src/sprite.h>

//
//
//
const size_t               FramesHeaderNBColumns = static_cast<unsigned int>(eFramesColumnsType::HeaderNBColumns);
const std::vector<QString> FramesHeaderColumnNames
{
    QString(("")),

    //The rest below is for the parts/step to assemble the frame!
    QString(("Img ID")),
    QString(("Unk#0")),
    QString(("Offset")),
    QString(("Flip")),
    QString(("Rotation & Scaling")),
    QString(("Palette")),
    QString(("Priority")),
    QString(("Char Name")),
};

const char * MFrame::PropPartID = "framePartID";



//
//
//


Sprite *ImageContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

Sprite *Image::parentSprite()
{
    return static_cast<ImageContainer*>(parentNode())->parentSprite();
}

Sprite *PaletteContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

QVariant Image::imgData(int column, int role)const
{
    QVariant res;
    switch(column)
    {
    case 0: //preview
        if( role == Qt::DecorationRole )
            res.setValue(makeImage(parentSprite()->getPalette()));
        else if( role == Qt::SizeHintRole )
            res.setValue( QSize(m_img.size().width() *2, m_img.size().height() *2) );
        break;
    case 1:
        if( role == Qt::DisplayRole || role == Qt::EditRole )
            res.setValue(getImageUID());
        break;
    case 1: //depth
        res.setValue(QString("%1bpp").arg(m_depth));
        break;
    case 2: //resolution
        res.setValue(QString("%1x%2").arg(m_img.width()).arg(m_img.height()));
    };
    return std::move(res);
}

Sprite *FramesContainer::parentSprite()
{
    return static_cast<Sprite*>(parentNode());
}

MFrame::MFrame(TreeElement *parent)
    :paren_t(parent), m_delegate(this)
{
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame::MFrame(const MFrame &cp)
    :paren_t(cp), m_delegate(this)
{
    operator=(cp);
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame::MFrame(MFrame &&mv)
    :paren_t(mv), m_delegate(this)
{
    operator=(mv);
    setNodeDataTy(eTreeElemDataType::frame);
}

MFrame &MFrame::operator=(const MFrame &cp)
{
    paren_t::operator=(cp);
    m_delegate = qMove(MFrameDelegate(this));
    return *this;
}

MFrame &MFrame::operator=(MFrame &&mv)
{
    paren_t::operator=(mv);
    m_delegate = qMove(MFrameDelegate(this));
    return *this;
}

bool MFrame::operator==(const MFrame &other) const  {return this == &other;}

bool MFrame::operator!=(const MFrame &other) const  {return !operator==(other);}





fmt::step_t *MFrame::getPart(int id)
{
    if(id < 0)
    {
        qDebug("MFrame::getPart(): Got negative part ID!\n");
        return nullptr;
    }
    else if( id >= m_container.size())
    {
        qCritical("MFrame::getPart(): Got partid out of range!(%1)\n", id);
    }
    else
    {
        return &(m_container[id].getPartData());
    }
}

const fmt::step_t *MFrame::getPart(int id)const
{
    return const_cast<MFrame*>(this)->getPart(id);
}

Sprite *MFrame::parentSprite()
{
    return static_cast<FramesContainer*>(parentNode())->parentSprite();
}

QPixmap MFrame::AssembleFrameToPixmap(int xoffset, int yoffset, QRect *out_area) const
{
    return qMove( QPixmap::fromImage(AssembleFrame(xoffset, yoffset, out_area)) );
}

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect * out_area, bool makebgtransparent) const
{
    Sprite * pspr = const_cast<MFrame*>(this)->parentSprite();
    //QRect dim;

    //#TODO: Implement checks for the other paramters for a frame, and for mosaic and etc!
    QImage      imgres(512,512, QImage::Format_ARGB32_Premultiplied);
    QPainter    painter(&imgres);
    QRect       bounds = calcFrameBounds();

    //Set first pal color as bg color!
    painter.setBackground( QColor(pspr->getPalette().front()) );
    painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

    QVector<QRgb> pal = pspr->getPalette();
    //Make first color transparent
    if(makebgtransparent)
    {

        QColor firstcol(pal.front());
        firstcol.setAlpha(0);
        pal.front() = firstcol.rgba();
    }
    //Draw all the parts of the frame
    const fmt::step_t * plast = nullptr; //A reference on the last valid frame, so we can properly copy it when encountering a -1 frame!
    for( const MFramePart & pwrap : m_container )
    {
        const fmt::step_t & part = pwrap.getPartData();
        //auto res = part.GetResolution();
        Image* pimg = pspr->getImage(part.getFrameIndex()); // returns null if -1 frame or out of range!
        QImage pix;
        if(!pimg && plast) //check for -1 frames
        {
            Image* plastimg = pspr->getImage(plast->getFrameIndex());
            pix = qMove(plastimg->makeImage(pal));
        }
        else if(pimg)
        {
            pix = qMove(pimg->makeImage(pal));
            plast = &part;
        }

        //pix.setMask(pix.createMaskFromColor( QColor(pspr->getPalette().front()), Qt::MaskMode::MaskInColor ));

        if(part.isHFlip())
            pix = qMove( pix.transformed( QTransform().scale(-1, 1) ) );
        if(part.isVFlip())
            pix = qMove( pix.transformed( QTransform().scale(1, -1) ) );

        int finalx = (part.getXOffset());
        int finaly = (part.getYOffset());
        painter.drawImage(xoffset + finalx, yoffset + finaly, pix );
    }

    //imgres.save("./mframeassemble.png", "png");

//    dim.setX(xoffset - bounds.x());
//    dim.setY(yoffset - bounds.y());
//    dim.setWidth(bounds.width());
//    dim.setHeight(bounds.height());

    if(out_area)
        *out_area = bounds;

    return imgres.copy( xoffset + bounds.x(),
                        yoffset + bounds.y(),
                        bounds.width(),
                        bounds.height() );
}

QRect MFrame::calcFrameBounds() const
{
    int smallestx = 512;
    int biggestx = 0;
    int smallesty  = 256;
    int biggesty  = 0;

    for( const MFramePart & pwrap : m_container )
    {
        const fmt::step_t & part = pwrap.getPartData();

        auto imgres = part.GetResolution();
        int xoff = part.getXOffset();
        int yoff = part.getYOffset();

        if( xoff < smallestx)
            smallestx = xoff;
        if( (xoff + imgres.first) >= biggestx )
            biggestx = (xoff + imgres.first);

        if( yoff < smallesty)
            smallesty = yoff;
        if( (yoff + imgres.second) >= biggesty )
            biggesty = (yoff + imgres.second);
    }

    return QRect( smallestx, smallesty, (biggestx - smallestx), (biggesty - smallesty) );
}

int MFrame::nodeColumnCount() const {return FramesHeaderNBColumns;}

QVariant MFrame::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    if( column < 0 || column >= static_cast<int>(eFramesColumnsType::NBColumns) )
        return QVariant();

    switch(static_cast<eFramesColumnsType>(column))
    {
    case eFramesColumnsType::Preview:
        {
            if(role == Qt::DisplayRole)
                return QString("frame#%1").arg(nodeIndex());
            if(role != Qt::DecorationRole)
                break;
            return qMove(QVariant(AssembleFrame(0,0)));
        }
//    case eFramesColumnsType::TotalSize:
//        {
//            if(role != Qt::DisplayRole)
//                break;
//            ///#TODO: Write the total resolution of the assembled frame!
//            break;
//        }
    default:
        {
        }
    };

    return QVariant();
}



QVariant MFrame::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
            role != Qt::DecorationRole &&
            role != Qt::SizeHintRole &&
            role != Qt::EditRole)
        return QVariant();

    const MFramePart *part = static_cast<const MFramePart*>(getItem(index));
    if(part)
        return part->nodeData(index.column(), role);
    return QVariant();
}

QVariant MFrame::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || section >= FramesHeaderNBColumns )
        return QVariant();

    if( role == Qt::DisplayRole )
    {
        if( orientation == Qt::Orientation::Horizontal)
            return FramesHeaderColumnNames[section];
    }
    else if(role == Qt::SizeHintRole)
    {
        if( orientation == Qt::Orientation::Horizontal)
        {
            QFontMetrics fm(QFont("Sergoe UI",9));
            return QSize(fm.width(FramesHeaderColumnNames[section])+4, fm.height()+4);
        }
    }
    return QVariant();
}

bool MFrame::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || role != Qt::EditRole)
        return false;

    fmt::step_t *ppart = getPart(index.row());
    Q_ASSERT(ppart);
    bool bok = false;

    switch(static_cast<eFramesColumnsType>(index.column()))
    {
    case eFramesColumnsType::ImgID:
        {
            ppart->setFrameIndex(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::Offset:
        {
            bok = value.canConvert<QPair<int,int>>();
            if(bok)
            {
                QPair<int,int> offs = value.value<QPair<int,int>>();
                ppart->setXOffset(offs.first);
                ppart->setYOffset(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert offset value to a QPair!\n");
            break;
        }
    case eFramesColumnsType::Flip:
        {
            bok = value.canConvert<QPair<bool,bool>>();
            if(bok)
            {
                QPair<bool,bool> offs = value.value<QPair<bool,bool>>();
                ppart->setVFlip(offs.first);
                ppart->setHFlip(offs.second);
            }
            else
                qDebug("MFrame::setData(): Couldn't convert flip value to a QPair!\n");
            break;
        }
    case eFramesColumnsType::RotNScaling:
        {
            ppart->setRotAndScaling(value.toBool());
            bok = true; //gotta set this to true because toBool doesn't return success or not
            break;
        }
    case eFramesColumnsType::PaletteID:
        {
            ppart->setPalNb(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::Priority:
        {
            ppart->setPriority(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::CharName:
        {
            ppart->setTileNum(value.toInt(&bok));
            break;
        }

    //direct access columns
    case eFramesColumnsType::direct_HFlip:
        {
            ppart->setHFlip(value.toBool());
            break;
        }
    case eFramesColumnsType::direct_VFlip:
        {
            ppart->setVFlip(value.toBool());
            break;
        }
    case eFramesColumnsType::direct_XOffset:
        {
            ppart->setXOffset(value.toUInt(&bok));
            break;
        }
    case eFramesColumnsType::direct_YOffset:
        {
            ppart->setYOffset(value.toUInt(&bok));
            break;
        }
        //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
    default:
        return false;
    };

    if(bok && index.model())
        const_cast<QAbstractItemModel*>(index.model())->dataChanged(index, index, QVector<int>{role});
    return bok;
}

int MFrame::columnCount(const QModelIndex &parent) const
{
    return static_cast<int>(eFramesColumnsType::NBColumns);
}






//
//
//
ImagesManager::ImagesManager(ImageContainer *parent)
    :QAbstractItemModel(), m_parentcnt(parent)
{}

ImagesManager::~ImagesManager()
{
    qDebug("ImagesManager::~ImagesManager()\n");
}

QModelIndex ImagesManager::index(int row, int column, const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    TreeElement *childItem  = parentItem->nodeChild(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ImagesManager::parent(const QModelIndex &child) const
{
    TreeElement *childItem = const_cast<ImagesManager*>(this)->getItem(child);
    TreeElement *parentItem = childItem->parentNode();
    Q_ASSERT(parentItem != nullptr);

    if (parentItem == m_parentcnt)
        return QModelIndex();

    return createIndex(parentItem->nodeIndex(), 0, parentItem);
}

int ImagesManager::rowCount(const QModelIndex &parent) const
{
    TreeElement *parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    return parentItem->nodeChildCount();
}

int ImagesManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<Image*>(parent.internalPointer())->nbimgcolumns();
    else
        return m_parentcnt->nodeColumnCount();
}

bool ImagesManager::hasChildren(const QModelIndex &parent) const
{
    TreeElement * parentItem = const_cast<ImagesManager*>(this)->getItem(parent);
    if(parentItem)
        return parentItem->nodeChildCount() > 0;
    else
        return false;
}

QVariant ImagesManager::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(hasChildren(QModelIndex()));
    if (!index.isValid())
        return QVariant("root");

    if (role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole)
        return QVariant();

    Image *img = static_cast<Image*>( const_cast<ImagesManager*>(this)->getItem(index));
    return img->imgData(index.column(), role);
}

QVariant ImagesManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole )
        return QVariant();

    if( orientation == Qt::Orientation::Vertical )
    {
        return std::move(QVariant( QString("%1").arg(section) ));
    }
    else if( orientation == Qt::Orientation::Horizontal )
    {
        switch(section)
        {
        case 0:
            return std::move(QVariant( QString("") ));
        case 1:
            return std::move(QVariant( QString("Bit Depth") ));
        case 2:
            return std::move(QVariant( QString("Resolution") ));
        };
    }
    return QVariant();
}

bool ImagesManager::insertRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, row, row + count - 1);
    success = parentItem->insertChildrenNodes(row, count);
    endInsertRows();

    return success;
}

bool ImagesManager::removeRows(int row, int count, const QModelIndex &parent)
{
    TreeElement *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildrenNodes(row, count);
    endRemoveRows();

    return success;
}

bool ImagesManager::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    Q_ASSERT(false);
    return false;
}

TreeElement *ImagesManager::getItem(const QModelIndex &index)
{
    if (index.isValid())
    {
        TreeElement *item = static_cast<TreeElement*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_parentcnt;
}

MFrameDelegate::MFrameDelegate(MFrame *frm, QObject *parent)
    :m_pfrm(frm), QStyledItemDelegate(parent)
{
}

MFrameDelegate::~MFrameDelegate()
{
}

QWidget *MFrameDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QWidget *pedit = nullptr;
    switch(static_cast<eFramesColumnsType>(index.column()))
    {
    case eFramesColumnsType::ImgID:
        pedit = makeImgSelect(parent, index.row());
    break;
    case eFramesColumnsType::Offset:
        pedit = makeOffsetSelect(parent, index.row());
    break;
    case eFramesColumnsType::Flip:
        pedit = makeFlipSelect(parent, index.row());
    break;
    case eFramesColumnsType::RotNScaling:
        pedit = makeRotNScalingSelect(parent, index.row());
    break;
    case eFramesColumnsType::PaletteID:
        pedit = makePaletteIDSelect(parent, index.row());
    break;
    case eFramesColumnsType::Priority:
        pedit = makePrioritySelect(parent, index.row());
    break;
    case eFramesColumnsType::CharName:
        pedit = makeTileIdSelect(parent, index.row());
    break;

        //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
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

void MFrameDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramesColumnsType>(index.column()))
    {
    case eFramesColumnsType::ImgID:
        {
            QComboBox   *pimglist = editor->findChild<QComboBox*>(ImgSelCmbBoxName());
            QPushButton *pbtn     = editor->findChild<QPushButton*>(ImgSelBtnName());
            Q_ASSERT(pimglist && pbtn);

            if(part->getFrameIndex() >= 0)
            {
                pimglist->setEnabled(true);
                pimglist->setCurrentIndex(part->getFrameIndex());
                pbtn->setChecked(false);
            }
            else
            {
                pimglist->setDisabled(true);
                pbtn->setChecked(true);
            }
            break;
        }
    case eFramesColumnsType::Offset:
        {
            QSpinBox *px = editor->findChild<QSpinBox*>(OffsetXSpinBoxName());
            QSpinBox *py = editor->findChild<QSpinBox*>(OffsetYSpinBoxName());
            Q_ASSERT(px && py);
            px->setValue(part->getXOffset());
            py->setValue(part->getYOffset());
            break;
        }
    case eFramesColumnsType::Flip:
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
    case eFramesColumnsType::RotNScaling:
        {
            QCheckBox *prns = editor->findChild<QCheckBox*>(RotNScaleChkBoxName());
            Q_ASSERT(prns);
            prns->setChecked(part->isRotAndScalingOn());
            break;
        }
    case eFramesColumnsType::PaletteID:
        {
            QSpinBox *ppalid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ppalid);
            ppalid->setValue(part->getPalNb());
            break;
        }
    case eFramesColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            pprio->setCurrentIndex(part->getPriority());
            break;
        }
    case eFramesColumnsType::CharName:
        {
            QSpinBox *ptid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ptid);
            ptid->setValue(part->getTileNum());
            break;
        }

        //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
    default:
        QStyledItemDelegate::setEditorData(editor,index);
        break;
    };

    //
    //        int value = index.model()->data(index, Qt::EditRole).toInt();

    //        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    //        spinBox->setValue(value);
}

void MFrameDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    fmt::step_t * part = m_pfrm->getPart(index.row());
    Q_ASSERT(part);

    //Populate the columns
    switch(static_cast<eFramesColumnsType>(index.column()))
    {
    case eFramesColumnsType::ImgID:
        {
            QComboBox   *pimglist = editor->findChild<QComboBox*>(ImgSelCmbBoxName());
            QPushButton *pbtn     = editor->findChild<QPushButton*>(ImgSelBtnName());
            Q_ASSERT(pimglist && pbtn);
            if(pbtn->isChecked())
                model->setData(index, (-1), Qt::EditRole);
            else
                model->setData(index, pimglist->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramesColumnsType::Offset:
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
    case eFramesColumnsType::Flip:
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
    case eFramesColumnsType::RotNScaling:
        {
            QCheckBox *prns = editor->findChild<QCheckBox*>(RotNScaleChkBoxName());
            Q_ASSERT(prns);
            model->setData(index, prns->isChecked(), Qt::EditRole);
            //#TODO: add saving of additional rns data
            break;
        }
    case eFramesColumnsType::PaletteID:
        {
            QSpinBox *ppalid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ppalid);
            ppalid->interpretText();
            model->setData(index, ppalid->value(), Qt::EditRole);
            break;
        }
    case eFramesColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            model->setData(index, pprio->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramesColumnsType::CharName:
        {
            QSpinBox *ptid = static_cast<QSpinBox*>(editor);
            Q_ASSERT(ptid);
            ptid->interpretText();
            model->setData(index, ptid->value(), Qt::EditRole);
            break;
        }

        //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
    default:
        QStyledItemDelegate::setModelData(editor, model, index);
        break;
    };

    //        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    //        spinBox->interpretText();
    //        int value = spinBox->value();

    //        model->setData(index, value, Qt::EditRole);
}

void MFrameDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}




QWidget *MFrameDelegate::makeImgSelect(QWidget *parent, int row) const
{
    //Make a combo box or a button to pick an image
    QFrame      *frm        = new QFrame(parent);
    QBoxLayout  *vertlayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, frm);
    QComboBox   *imglstb    = new QComboBox(frm);
    QPushButton *dupbtn     = new QPushButton(QString(tr("Prev")), frm);
    imglstb->setObjectName(ImgSelCmbBoxName());
    dupbtn-> setObjectName(ImgSelBtnName());
    dupbtn->setCheckable(true);
    //        connect( imglstb, SIGNAL(currentIndexChanged(int)), this,   SLOT(selectedImageChanged(int)) );
    //        connect( dupbtn,  SIGNAL(toggled(bool)),            this,   SLOT(selectedImageToggledMinOne(bool)) );

    connect( dupbtn, &QPushButton::toggled, imglstb, &QComboBox::setDisabled );
    imglstb->setModel(m_pfrm->parentSprite()->getImages().getModel());

    imglstb->setIconSize( QSize(32,32) );

    frm->setLayout(vertlayout);
    vertlayout->addWidget(imglstb, 2);
    vertlayout->addWidget(dupbtn, 0);
    frm->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    frm->setMinimumHeight(72);
    dupbtn->setMinimumWidth(32);
    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    dupbtn->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
    frm->setContentsMargins(0, 0, 0, 0);
    vertlayout->setContentsMargins(0, 0, 0, 0);
    return frm;
}





QWidget *MFrameDelegate::makeFlipSelect(QWidget *parent, int row) const
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





QWidget *MFrameDelegate::makeOffsetSelect(QWidget *parent, int row) const
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
    return pselect;
}





QWidget *MFrameDelegate::makeRotNScalingSelect(QWidget *parent, int row) const
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

QWidget *MFrameDelegate::makePaletteIDSelect(QWidget *parent, int row) const
{
    QSpinBox *ppalid = new QSpinBox(parent);
    ppalid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ppalid->setRange(0, fmt::step_t::PALID_MAX);
    return ppalid;
}

QWidget *MFrameDelegate::makePrioritySelect(QWidget *parent, int row) const
{

    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(MFrameDelegate::prioritiesNames());
    return pselect;
}

QWidget *MFrameDelegate::makeTileIdSelect(QWidget *parent, int row) const
{
    QSpinBox *ptileid = new QSpinBox(parent);
    ptileid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ptileid->setRange(0, fmt::step_t::TILENUM_MAX);
    return ptileid;
}

//
//  MFramePart
//

Sprite * MFramePart::parentSprite()
{
    return static_cast<MFrame*>(parentNode())->parentSprite();
}

QImage MFramePart::drawPart() const
{
    if(m_data.getFrameIndex() < 0)
    {
        qWarning("MFramePart::drawPart() was asked to draw a -1 frame!!\n");
        return QImage();
    }

    //auto res = m_data.GetResolution();
    QImage imgo;
    const Sprite* spr = parentSprite();
    Q_ASSERT(spr);
    const Image * pimg = spr->getImage( m_data.getFrameIndex() );
    if(!pimg)
    {
        qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
        return QImage();
    }

    if(spr->unk13() == 1)
        qDebug("MFramePart::drawPart(): This part probably won't be drawn correctly, since it appears to be set to 1D mapping!\n");

    imgo = pimg->makeImage(spr->getPalette());
    imgo = qMove(imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied));
    applyTransforms(imgo);
    return qMove(imgo);
}

void MFramePart::applyTransforms(QImage &srcimg) const
{
    srcimg = qMove( srcimg.transformed( QTransform().scale( m_data.isHFlip()? -1 : 1,
                                                           m_data.isVFlip()? -1 : 1) ) );
}

QVariant MFramePart::dataImgPreview(int role) const
{
    Sprite *parentspr = const_cast<MFramePart*>(this)->parentSprite();
    if(role == Qt::DecorationRole)
    {
        //#TODO: Draw only this part/step
        if( m_data.getFrameIndex() >= 0 && m_data.getFrameIndex() < parentspr->getImages().nodeChildCount() )
        {
            Image * pimg = parentspr->getImage(m_data.getFrameIndex());
            if(!pimg)
                qCritical("MFrame::dataImgPreview(): Invalid image at index %d!\n", m_data.getFrameIndex());
            else
                return QVariant(pimg->makeImage(parentspr->getPalette()));
        }
        else
        {
            //#TODO: draw last step
            return QVariant("COPY PREV");
        }
    }
    else if(role == Qt::SizeHintRole)
    {
        if( m_data.getFrameIndex() >= 0 && m_data.getFrameIndex() < parentspr->getImages().nodeChildCount() )
        {
            Image * pimg = parentspr->getImage(m_data.getFrameIndex());
            return (pimg->getImageSize());
//            return (pimg->getImageSize()).width();
        }
    }
    return QVariant();
}


