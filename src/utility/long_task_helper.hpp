#ifndef LONG_TASK_HELPER_H
#define LONG_TASK_HELPER_H
#include <QFuture>
#include <QFutureSynchronizer>
#include <QFutureWatcher>
#include <QObject>
#include <QThread>
#include <src/ui/windows/dialogprogressbar.hpp>

/*
 * Utility for displaying progress and running "long" async tasks.
 * So for example, in gui mode, running a long task would popups a progress bar.
*/


//Class for running a task, and if running in gui mode, the progress dialog is displayed
class TaskProgressWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TaskProgressWatcher)
public:
    TaskProgressWatcher(QList<QFuture<void>> && tasks, QObject * parent = nullptr);
    virtual ~TaskProgressWatcher();

    inline bool isSilent()const;

public slots:
    void runTask();
    void setSilent(bool silent);

signals:
    void taskCompleted();
    void updateProgress(int percent);

private:
    QList<QFuture<void>>                m_futures;
    QFutureSynchronizer<void>           m_futsync;
    QScopedPointer<DialogProgressBar>   m_progress;
    QScopedPointer<QThread>             m_dialogThread;
    bool                                m_silent    {false};
};

#endif // LONG_TASK_HELPER_H
