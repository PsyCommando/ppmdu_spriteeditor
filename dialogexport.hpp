#ifndef DIALOGEXPORT_HPP
#define DIALOGEXPORT_HPP

#include <QDialog>
#include <QSaveFile>

namespace Ui {
class DialogExport;
}

class DialogExport : public QDialog
{
    Q_OBJECT

public:
    explicit DialogExport(QWidget *parent = 0);
    ~DialogExport();

private:
    Ui::DialogExport *ui;
    QSaveFile         m_saver;
};

#endif // DIALOGEXPORT_HPP
