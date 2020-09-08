#ifndef ERRORHELPER_HPP
#define ERRORHELPER_HPP
#include <QString>
#include <QPointer>

class MainWindow;
/*
 * Helper class for helping sub-classes displaying error messages via the main window of the application .
*/
class ErrorHelper
{
public:
    static ErrorHelper& getInstance();

    /*
     * Sets the main ui window that will be displaying the errors.
    */
    void setMainWindow(MainWindow * main);

    /*
     * Displays an error message to the user.
    */
    void sendErrorMessage(const QString & msg);

    /*
     * Displays a warning message to the user.
    */
    void sendWarningMessage(const QString & msg);

private:
    ErrorHelper();

    QPointer<MainWindow> pmain;
};

#endif // ERRORHELPER_HPP
