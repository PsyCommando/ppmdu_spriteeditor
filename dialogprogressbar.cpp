#include "dialogprogressbar.hpp"
#include "ui_dialogprogressbar.h"

DialogProgressBar::DialogProgressBar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProgressBar)
{
    ui->setupUi(this);
    setModal(true);
}

//DialogProgressBar::DialogProgressBar(const QFuture<int> &fut, QWidget *parent):
//    QDialog(parent),
//    ui(new Ui::DialogProgressBar)
//{
//    ui->setupUi(this);
//    setModal(true);
//    connect( &m_watcher, SIGNAL(progressTextChanged(QString)),  ui->lblcurrentop,   SLOT(setText(QString)) );
//    connect( &m_watcher, SIGNAL(progressValueChanged(int)),     ui->progressBar,    SLOT(setValue(int)) );
//    connect( &m_watcher, SIGNAL(progressRangeChanged(int,int)), ui->progressBar,    SLOT(setRange(int,int)) );
//    connect( &m_watcher, SIGNAL(finished()),                    this,               SLOT(finished()) );
//    m_watcher.setFuture(fut);
//}

DialogProgressBar::~DialogProgressBar()
{
    delete ui;
}

void DialogProgressBar::finished()
{
    close();
}

void DialogProgressBar::setFuture(QFuture<void> & fut )
{
    //m_curfut = fut;
    connect( &m_watcher, SIGNAL(progressTextChanged(QString)),  ui->lblcurrentop,   SLOT(setText(QString)) );
    connect( &m_watcher, SIGNAL(progressValueChanged(int)),     ui->progressBar,    SLOT(setValue(int)) );
    connect( &m_watcher, SIGNAL(progressRangeChanged(int,int)), ui->progressBar,    SLOT(setRange(int,int)) );
    connect( &m_watcher, SIGNAL(finished()),                    this,               SLOT(finished()) );
    m_watcher.setFuture(fut);
    update();
}

void DialogProgressBar::on_btncancel_clicked()
{
    m_watcher.cancel();
    m_watcher.waitForFinished();
    close();
}

void DialogProgressBar::on_btnpause_toggled(bool checked)
{
    if(checked)
        m_watcher.resume();
    else
        m_watcher.pause();
}
