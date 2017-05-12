#include "diagsingleimgcropper.hpp"
#include "ui_diagsingleimgcropper.h"

DiagSingleImgCropper::DiagSingleImgCropper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiagSingleImgCropper)
{
    ui->setupUi(this);
}

DiagSingleImgCropper::~DiagSingleImgCropper()
{
    delete ui;
}
