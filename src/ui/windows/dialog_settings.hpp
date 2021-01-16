#ifndef DIALOG_SETTINGS_HPP
#define DIALOG_SETTINGS_HPP
#include <QSettings>
#include <QDialog>

namespace Ui {
class DialogSettings;
}

/*
 * Small dialog to allow the user to change program-wide settings
*/
class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = nullptr);
    ~DialogSettings();

private slots:
    void writeSettings(QSettings & settings);
    void readSettings(QSettings & settings);

    void on_DialogSettings_accepted();

private:
    void commitSettings();
private:
    Ui::DialogSettings *ui;
};

#endif // DIALOG_SETTINGS_HPP
