#include "long_task_helper.hpp"
#include <QApplication>


TaskProgressWatcher::TaskProgressWatcher(QList<QFuture<void>> && tasks, QObject * parent)
    :QObject(parent)
{
    m_futures = tasks;
    Q_FOREACH(const QFuture<void> & task, m_futures)
    {
        m_futsync.addFuture(task);
    }

}

TaskProgressWatcher::~TaskProgressWatcher()
{

}

bool TaskProgressWatcher::isSilent() const
{
    return m_silent;
}

void TaskProgressWatcher::runTask()
{
    if(!m_silent)
    {
        m_dialogThread.reset(new QThread);
        DialogProgressBar *progress = new DialogProgressBar();
        connect(this, &TaskProgressWatcher::updateProgress, progress, &DialogProgressBar::OnProgressChanged);
        connect(this, &TaskProgressWatcher::taskCompleted, progress, &DialogProgressBar::deleteLater);
        progress->moveToThread(m_dialogThread.data());
        m_dialogThread->start();
        progress->exec();
    }

    m_futsync.waitForFinished();
    emit taskCompleted();
    if(m_dialogThread)
    {
        m_dialogThread->terminate();
        m_dialogThread->wait();
    }
}

void TaskProgressWatcher::setSilent(bool silent)
{
    m_silent = silent;
}
