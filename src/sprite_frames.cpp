#include "sprite_img.hpp"
#include <src/sprite_frames.hpp>
#include <src/sprite.hpp>


const QString MinusOneImgRes(":/resources/imgs/minus_one.png");

//*******************************************************************
//  MFramePart
//*******************************************************************

MFramePart::MFramePart(TreeElement *parent)
    :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags())
{
    setNodeDataTy(eTreeElemDataType::framepart);
    m_data.attr0 = 0;
    m_data.attr1 = 0;
    m_data.attr2 = 0;
    m_data.setXOffset(255);
    m_data.setYOffset(255);
    m_data.setFrameIndex(-1);
}

MFramePart::MFramePart(TreeElement *parent, const fmt::step_t &part)
    :partparent_t(parent, Qt::ItemFlag::ItemIsEditable | DEFFlags()), m_data(part)
{
    setNodeDataTy(eTreeElemDataType::framepart);
}

MFramePart::~MFramePart()
{
}

bool MFramePart::operator==(const MFramePart &other) const
{
    return m_data.attr0 == other.m_data.attr0 &&
           m_data.attr1 == other.m_data.attr1 &&
           m_data.attr2 == other.m_data.attr2;
}

bool MFramePart::operator!=(const MFramePart &other) const
{
    return !operator==(other);
}

Sprite * MFramePart::parentSprite()
{
    return static_cast<MFrame*>(parentNode())->parentSprite();
}

const Sprite *MFramePart::parentSprite() const
{
    return const_cast<MFramePart*>(this)->parentSprite();
}

int MFramePart::nodeColumnCount() const                 {return FramesHeaderNBColumns;}

Qt::ItemFlags MFramePart::nodeFlags(int column) const
{
    if(column == static_cast<int>(eFramesColumnsType::Preview) )
        return Qt::ItemFlags(m_flags).setFlag(Qt::ItemFlag::ItemIsEditable, false); //The preview is never editable!
    return m_flags;
}

QVariant MFramePart::nodeData(int column, int role) const
{
    if( role != Qt::DisplayRole &&
        role != Qt::DecorationRole &&
        role != Qt::SizeHintRole &&
        role != Qt::EditRole )
        return QVariant();

    switch(static_cast<eFramesColumnsType>(column))
    {
    case eFramesColumnsType::Preview:       return dataImgPreview(role);
    case eFramesColumnsType::ImgID:         return dataImgId(role);
    case eFramesColumnsType::TileNum:       return dataTileNum(role);
    case eFramesColumnsType::PaletteID:     return dataPaletteID(role);
    case eFramesColumnsType::Unk0:          return dataUnk0(role);
    case eFramesColumnsType::Offset:        return dataOffset(role);
    case eFramesColumnsType::Flip:          return dataFlip(role);
    case eFramesColumnsType::RotNScaling:   return dataRotNScaling(role);
    case eFramesColumnsType::Mosaic:        return dataMosaic(role);
    case eFramesColumnsType::Mode:          return dataMode(role);
    case eFramesColumnsType::Priority:      return dataPriority(role);

    case eFramesColumnsType::direct_VFlip:   return dataDirectVFlip(role);
    case eFramesColumnsType::direct_HFlip:   return dataDirectHFlip(role);
    case eFramesColumnsType::direct_XOffset: return dataDirectXOffset(role);
    case eFramesColumnsType::direct_YOffset: return dataDirectYOffset(role);

        //Undefined cases
    default:
    break;
    };

    return QVariant();
}

QImage MFramePart::drawPart(bool transparencyenabled) const
{
    int imgindex = -1;

    if(m_data.getFrameIndex() < 0)
    {
        qInfo("MFramePart::drawPart(): was asked to draw a -1 frame!!\n");

        if(nodeIndex() > 0)
        {
            const MFrame * frm = static_cast<const MFrame*>(parentNode());
            if(!frm)
            {
                qWarning("MFramePart::drawPart(): Can't access parent frame!!\n");
                return QImage(); //Can't do much here!
            }

            //find previous non -1 frame part!
            for( int cntlkb = nodeIndex(); cntlkb >= 0; --cntlkb )
            {
                const fmt::step_t * p = frm->getPart(cntlkb);
                Q_ASSERT(p);
                if(p->getFrameIndex() >= 0)
                {
                    imgindex = p->getFrameIndex();
                    break;
                }
            }
        }

        //If we didn't find a replacement frame, just return an empty image.
        if(imgindex < 0)
            return QImage();
        //Otherwise continue with the valid index!
    }
    else
    {
        imgindex = m_data.getFrameIndex();
    }

    //auto res = m_data.GetResolution();
    QImage imgo;
    const Sprite* spr = parentSprite();
    Q_ASSERT(spr);
    const Image * pimg = spr->getImage( imgindex );
    if(!pimg)
    {
        qWarning("MFramePart::drawPart(): Invalid image reference!!\n");
        return QImage();
    }

    if(spr->unk13() == 1)
        qDebug("MFramePart::drawPart(): This part probably won't be drawn correctly, since it appears to be set to 1D mapping!\n");


    if(transparencyenabled)
    {
        QVector<QRgb> newpal = spr->getPalette();
        if(newpal.size() > 0)
            newpal.front() = (~(0xFF << 24)) & newpal.front(); //Format is ARGB
        imgo = pimg->makeImage(newpal);
    }
    else
    {
        imgo = pimg->makeImage(spr->getPalette());
    }

    imgo = imgo.convertToFormat(QImage::Format::Format_ARGB32_Premultiplied);
    applyTransforms(imgo);
    return imgo;
}

void MFramePart::importPart(const fmt::step_t &part)
{
    m_data = part;
}

fmt::step_t MFramePart::exportPart() const
{
    return m_data;
}

fmt::step_t &MFramePart::getPartData()
{
    return m_data;
}

const fmt::step_t &MFramePart::getPartData() const
{
    return m_data;
}

QSize MFramePart::calcTextSize(const QString &str)
{
    static QFontMetrics fm(QFont("Sergoe UI", 9));
    return QSize(fm.horizontalAdvance(str), fm.height());
}

void MFramePart::applyTransforms(QImage &srcimg) const
{
    srcimg = srcimg.transformed( QTransform().scale( m_data.isHFlip()? -1 : 1,
                                                    m_data.isVFlip()? -1 : 1) ) ;
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

QVariant MFramePart::dataImgId(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("Img#%1").arg(static_cast<int>(m_data.getFrameIndex()));
    }
    else if(role == Qt::EditRole)
    {
        //Just output image id
        return static_cast<int>(m_data.getFrameIndex());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataImgId(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 150 );
        //sz.setHeight(sz.height() + 72);
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataUnk0(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.unk0);
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataUnk0(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("(%1, %2)").arg(m_data.getXOffset()).arg(m_data.getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<int,int>(m_data.getXOffset(), m_data.getYOffset()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataOffset(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectXOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(m_data.getXOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(m_data.getXOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectXOffset(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectYOffset(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return QString("%1").arg(m_data.getYOffset());
    }
    else if(role == Qt::EditRole)
    {
        return QVariant(m_data.getYOffset());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectYOffset(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        QString flipval;
        if( !m_data.isVFlip() && !m_data.isHFlip() )
            flipval = "Not flipped";
        else
        {
            if(m_data.isVFlip())
                flipval += "V ";
            if(m_data.isHFlip())
                flipval += "H ";
        }
        return flipval;
    }
    else if(role == Qt::EditRole)
    {
        QVariant res;
        res.setValue(QPair<bool,bool>(m_data.isVFlip(), m_data.isHFlip()));
        return res;
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataFlip(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectVFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isVFlip();
    }
    else if(role == Qt::EditRole)
    {
        return m_data.isVFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectVFlip(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataDirectHFlip(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isHFlip();
    }
    else if(role == Qt::EditRole)
    {
        return m_data.isHFlip();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataDirectHFlip(Qt::DisplayRole).toString());
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataRotNScaling(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return m_data.isRotAndScalingOn();
    }
    else if(role == Qt::EditRole)
    {
        //#TODO: will need a custom struct here to properly send over RnS data!
        return m_data.isRotAndScalingOn();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataRotNScaling(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 80 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataPaletteID(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getPalNb());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataPaletteID(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataPriority(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return MFrameDelegate::prioritiesNames().at(m_data.getPriority());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataPriority(Qt::DisplayRole).toString());
        sz.setWidth( sz.width() + 50 );
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataTileNum(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getTileNum());
    }
    else if(role == Qt::SizeHintRole)
    {
        return calcTextSize(dataTileNum(Qt::DisplayRole).toString());
    }
    return QVariant();
}

QVariant MFramePart::dataMosaic(int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_data.isMosaicOn();
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMosaic(Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 20);
        return sz;
    }
    return QVariant();
}

QVariant MFramePart::dataMode(int role) const
{
    if(role == Qt::DisplayRole)
    {
        return MFrameDelegate::modeNames().at(static_cast<int>(m_data.getObjMode()));
    }
    else if(role == Qt::EditRole)
    {
        return static_cast<int>(m_data.getObjMode());
    }
    else if(role == Qt::SizeHintRole)
    {
        QSize sz = calcTextSize(dataMode(Qt::DisplayRole).toString());
        sz.setWidth(sz.width() + 60);
        return sz;
    }
    return QVariant();
}


//=====================================================================================
//  MFrameDelegate
//=====================================================================================
MFrameDelegate::MFrameDelegate(const MFrameDelegate & cp)
    :QStyledItemDelegate(cp.parent()), m_minusone(MinusOneImgRes)
{
    operator=(cp);
}

MFrameDelegate::MFrameDelegate(MFrameDelegate && mv)
    :QStyledItemDelegate(mv.parent()), m_minusone(MinusOneImgRes)
{
    operator=(mv);
}

MFrameDelegate & MFrameDelegate::operator=(const MFrameDelegate & cp)
{
    setParent(cp.parent());
    m_pfrm    = cp.m_pfrm;
    return *this;
}

MFrameDelegate & MFrameDelegate::operator=(MFrameDelegate && mv)
{
    setParent(mv.parent());
    m_pfrm    = mv.m_pfrm;
    mv.m_pfrm = nullptr;
    return *this;
}

MFrameDelegate::MFrameDelegate(MFrame *frm, QObject *parent)
    :QStyledItemDelegate(parent), m_minusone(MinusOneImgRes)
{
    m_pfrm = frm;
}

QSize MFrameDelegate::sizeHint( const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
{
    TreeElement * pnode = static_cast<TreeElement *>(index.internalPointer());
    if(pnode)
        return pnode->nodeData(index.column(), Qt::SizeHintRole).toSize();
    else
        return QSize();
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
    case eFramesColumnsType::TileNum:
        pedit = makeTileIdSelect(parent, index.row());
    break;
    case eFramesColumnsType::PaletteID:
        pedit = makePaletteIDSelect(parent, index.row());
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
    case eFramesColumnsType::Mode:
        pedit = makeModeSelect(parent, index.row());
    break;
    case eFramesColumnsType::Priority:
        pedit = makePrioritySelect(parent, index.row());
    break;

    //Undefined cases
    //case eFramesColumnsType::Preview:
    //case eFramesColumnsType::TotalSize:
    //case eFramesColumnsType::Unk0:
    //case eFramesColumnsType::Mosaic:
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
    case eFramesColumnsType::Mode:
        {
            QComboBox *pobjmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pobjmode);
            pobjmode->setCurrentIndex(static_cast<int>(part->getObjMode()));
            break;
        }
    case eFramesColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            pprio->setCurrentIndex(part->getPriority());
            break;
        }
    case eFramesColumnsType::TileNum:
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
    case eFramesColumnsType::Mode:
        {
            QComboBox *pobjmode = static_cast<QComboBox*>(editor);
            Q_ASSERT(pobjmode);
            model->setData(index, pobjmode->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramesColumnsType::Priority:
        {
            QComboBox *pprio = static_cast<QComboBox*>(editor);
            Q_ASSERT(pprio);
            model->setData(index, pprio->currentIndex(), Qt::EditRole);
            break;
        }
    case eFramesColumnsType::TileNum:
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

QWidget *MFrameDelegate::makeImgSelect(QWidget *parent, int /*row*/) const
{
    QComboBox      *imglstb = new QComboBox(parent);
    ImageContainer * pcnt   = &(m_pfrm->parentSprite()->getImages());
    imglstb->setIconSize( QSize(32,32) );
    imglstb->setStyleSheet(pcnt->ComboBoxStyleSheet());

    //Fill the combobox!

    //Add nodraw frame
    imglstb->addItem( QPixmap::fromImage(m_minusone), "ID:-1 No Draw Frame", QVariant(-1)); //Set user data to -1
    //Add actual images!
    for( int cntimg = 0; cntimg < pcnt->nodeChildCount(); ++cntimg )
    {
        Image* pimg = pcnt->getImage(cntimg);
        QPixmap pmap = QPixmap::fromImage(pimg->imgDataCondensed(Qt::DecorationRole).value<QImage>());
        QString text = pimg->imgDataCondensed(Qt::DisplayRole).toString();
        imglstb->addItem( QIcon(pmap), text, QVariant(pimg->getID()) ); //Set user data to image's UID
    }
    imglstb->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
    return imglstb;
}

QWidget *MFrameDelegate::makeFlipSelect(QWidget *parent, int /*row*/) const
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





QWidget *MFrameDelegate::makeOffsetSelect(QWidget *parent, int /*row*/) const
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





QWidget *MFrameDelegate::makeRotNScalingSelect(QWidget *parent, int /*row*/) const
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

QWidget *MFrameDelegate::makePaletteIDSelect(QWidget *parent, int /*row*/) const
{
    //#TODO: Make this into a combo box to pick the palette and get a preview!
    QSpinBox *ppalid = new QSpinBox(parent);
    ppalid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ppalid->setRange(0, fmt::step_t::PALID_MAX);
    return ppalid;
}

QWidget *MFrameDelegate::makePrioritySelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(MFrameDelegate::prioritiesNames());
    return pselect;
}

QWidget *MFrameDelegate::makeTileIdSelect(QWidget *parent, int /*row*/) const
{
    QSpinBox *ptileid = new QSpinBox(parent);
    ptileid->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    ptileid->setRange(0, fmt::step_t::TILENUM_MAX);
    return ptileid;
}

QWidget *MFrameDelegate::makeModeSelect(QWidget *parent, int /*row*/) const
{
    QComboBox *pselect = new QComboBox(parent);
    pselect->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    pselect->addItems(MFrameDelegate::modeNames());
    pselect->setMinimumWidth(100);
    return pselect;
}

//=====================================================================================
//  MFrame
//=====================================================================================
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
    m_delegate = MFrameDelegate(this);
    return *this;
}

MFrame &MFrame::operator=(MFrame &&mv)
{
    paren_t::operator=(mv);
    m_delegate = MFrameDelegate(this);
    return *this;
}

bool MFrame::operator==(const MFrame &other) const
{
    return this == &other;
}

bool MFrame::operator!=(const MFrame &other) const
{
    return !operator==(other);
}

fmt::step_t *MFrame::getPart(int id)
{
    if(id < 0)
    {
        qDebug("MFrame::getPart(): Got negative part ID!\n");
        return nullptr;
    }
    else if( id >= m_container.size())
    {
        qCritical("MFrame::getPart(): Got partid out of range!(%d)\n", id);
    }
    return &(m_container[id].getPartData());
}

const fmt::step_t *MFrame::getPart(int id)const
{
    return const_cast<MFrame*>(this)->getPart(id);
}

Sprite *MFrame::parentSprite()
{
    return static_cast<FramesContainer*>(parentNode())->parentSprite();
}

QPixmap MFrame::AssembleFrameToPixmap(int xoffset, int yoffset, QRect cropto, QRect *out_area) const
{
    return qMove( QPixmap::fromImage(AssembleFrame(xoffset, yoffset, cropto, out_area)) );
}

QImage MFrame::AssembleFrame(int xoffset, int yoffset, QRect cropto, QRect * out_area, bool makebgtransparent) const
{
    Sprite * pspr = const_cast<MFrame*>(this)->parentSprite();
    if(pspr->getPalette().empty()) //no point drawing anything..
        return QImage();

    //#TODO: Implement checks for the other paramters for a frame, and for mosaic and etc!
    QImage      imgres(512,512, QImage::Format_ARGB32_Premultiplied);
    QPainter    painter(&imgres);
    QRect       bounds = calcFrameBounds();

    //Try to make the bounds match cropto if smaller
    if( !cropto.isNull() )
    {
        if(bounds.x() > cropto.x())
            bounds.setX( cropto.x() );
        if(bounds.y() > cropto.y())
            bounds.setY( cropto.y() );

        if(bounds.width() < cropto.width())
            bounds.setWidth( cropto.width() );
        if(bounds.height() < cropto.height())
            bounds.setHeight( cropto.height() );
    }



    QVector<QRgb> pal = pspr->getPalette();
    //Make first color transparent
    if(makebgtransparent)
    {
        QColor firstcol(pal.front());
        firstcol.setAlpha(0);
        pal.front() = firstcol.rgba();
        painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    }
    else
    {
        //Set first pal color as bg color!
        painter.setBackground( QColor(pspr->getPalette().front()) );
        painter.setBackgroundMode(Qt::BGMode::OpaqueMode);

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
            pix = plastimg->makeImage(pal);
        }
        else if(pimg)
        {
            pix = pimg->makeImage(pal);
            plast = &part;
        }

        //pix.setMask(pix.createMaskFromColor( QColor(pspr->getPalette().front()), Qt::MaskMode::MaskInColor ));

        if(part.isHFlip())
            pix = pix.transformed( QTransform().scale(-1, 1) );
        if(part.isVFlip())
            pix = pix.transformed( QTransform().scale(1, -1) );

        int finalx = (part.getXOffset());
        int finaly = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset(); //simulate wrap-around past 256 Y
        painter.drawImage(xoffset + finalx, yoffset + finaly, pix );
    }

    if(out_area)
        *out_area = bounds;

//    imgres.save("./mframeassemble.png", "png");

//    imgres.copy( xoffset + bounds.x(),
//                        yoffset + bounds.y(),
//                        bounds.width(),
//                        bounds.height() ).save("./mframeassemble_cropped.png", "png");

    return imgres.copy( xoffset + bounds.x(),
                        yoffset + bounds.y(),
                        bounds.width(),
                        bounds.height() );
}

QRect MFrame::calcFrameBounds() const
{
    int smallestx = 512;
    int biggestx = 0;
    int smallesty  = 512;
    int biggesty  = 0;

    for( const MFramePart & pwrap : m_container )
    {
        const fmt::step_t & part = pwrap.getPartData();

        auto imgres = part.GetResolution();
        int xoff = part.getXOffset();
        int yoff = (part.getYOffset() < 128)? part.getYOffset() + 255 : part.getYOffset();

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
            return QVariant(AssembleFrame(0,0, QRect()));
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

QVariant MFrame::frameDataCondensed(int role) const
{
    if(role == Qt::DecorationRole)
    {
        return QPixmap::fromImage(AssembleFrame(0,0,QRect()));
    }
    else if(role == Qt::DisplayRole)
    {
        return QString("FrameID:%1").arg(getFrameUID());
    }
    else if(role == Qt::EditRole)
    {
        return getFrameUID();
    }
    else if(role == Qt::SizeHintRole)
    {
        QFontMetrics fm(QFont("Sergoe UI",9));
        return QSize(fm.horizontalAdvance(frameDataCondensed(role == Qt::DisplayRole).toInt()+32),
                     qMax(fm.height() + 32, calcFrameBounds().height()) );
    }
    return QVariant();
}

QVariant MFrame::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(section < 0 || static_cast<size_t>(section) >= FramesHeaderColumnNames.size() )
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
            return QSize(fm.horizontalAdvance(FramesHeaderColumnNames[section])+4, fm.height()+4);
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
    case eFramesColumnsType::Mode:
        {
            ppart->setObjMode(static_cast<fmt::step_t::eObjMode>(value.toInt(&bok)));
            break;
        }
    case eFramesColumnsType::Priority:
        {
            ppart->setPriority(value.toInt(&bok));
            break;
        }
    case eFramesColumnsType::TileNum:
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
    {
        const_cast<QAbstractItemModel*>(index.model())->dataChanged(index, index, QVector<int>{role});
    }
    return bok;
}

int MFrame::columnCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return 0;
    return static_cast<int>(eFramesColumnsType::NBColumns);
}
