#include "errorhelper.hpp"
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
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
    QMutexLocker ql(&m_msgmtx);
    qWarning() << "<Error Logged> : \"" << msg <<"\"";
    if(pmain)
        pmain->ShowStatusErrorMessage(msg);
}

void ErrorHelper::sendWarningMessage(const QString &msg)
{
    QMutexLocker ql(&m_msgmtx);
    qWarning() << "<Warning Logged> : \"" << msg <<"\"";
    if(pmain)
        pmain->ShowStatusErrorMessage(msg);
}

ErrorHelper::ErrorHelper()
    :pmain(nullptr)
{
}
