#include "errorhelper.hpp"
#include <src/ui/mainwindow.hpp>

ErrorHelper & ErrorHelper::getInstance()
{
    static ErrorHelper instance;
    return instance;
}

void ErrorHelper::setMainWindow(MainWindow *main)
{
    pmain = main;
}

void ErrorHelper::sendErrorMessage(const QString &msg)
{
    if(pmain)
        pmain->ShowStatusErrorMessage(msg);
}

void ErrorHelper::sendWarningMessage(const QString &msg)
{
    if(pmain)
        pmain->ShowStatusErrorMessage(msg);
}

ErrorHelper::ErrorHelper()
    :pmain(nullptr)
{
}
