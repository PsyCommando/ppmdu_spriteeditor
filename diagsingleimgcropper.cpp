#include "diagsingleimgcropper.hpp"
#include "ui_diagsingleimgcropper.h"
#include <src/sprite.h>

DiagSingleImgCropper::DiagSingleImgCropper(QWidget *parent, Image * pimg) :
    QDialog(parent),
    ui(new Ui::DiagSingleImgCropper),
    m_pimg(pimg)
{
    ui->setupUi(this);
}

DiagSingleImgCropper::~DiagSingleImgCropper()
{
    delete ui;
}
