#ifndef DIALOGPROGRESSBAR_HPP
#define DIALOGPROGRESSBAR_HPP

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>

namespace Ui {
class DialogProgressBar;
}

class DialogProgressBar : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProgressBar(QWidget *parent = 0);
    ~DialogProgressBar();

public slots:
    void finished();
    void setFuture(QFuture<void> &fut);

private slots:
    void on_btncancel_clicked();
    void on_btnpause_toggled(bool checked);

private:
    Ui::DialogProgressBar *ui;
    QFutureWatcher<void>   m_watcher;
    QFuture<void>          m_curfut;
};

#endif // DIALOGPROGRESSBAR_HPP
