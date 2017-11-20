#include "dialogabout.hpp"
#include "ui_dialogabout.h"
#include <QString>

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);
    setWindowTitle( QString("About Sprite Muncher %1").arg(QString(GIT_VERSION)) );
    ui->lblVersion->setText(QString("<p><span style=\" font-size:9pt; font-weight:600;\">Sprite Muncher v%1</span></p>").arg(QString(GIT_VERSION)));
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
