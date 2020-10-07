#ifndef THREADEDWRITER_HPP
#define THREADEDWRITER_HPP
#include <QObject>
#include <QSaveFile>
#include <QMutex>
#include <QFuture>
#include <src/data/base_container.hpp>

/*
    ThreadedWriter
        Used so the event loop of the mainwindow can continue looping while we do long operations!
*/

class ThreadedWriter : public QObject
{
    Q_OBJECT
public:
    QScopedPointer<QSaveFile>         savefile;
    BaseContainer                   * container;
    QMutex                            mtxdata;
    int                               bywritten;
    QFuture<void>                     op1;
    QFuture<void>                     op2;

    explicit ThreadedWriter(QSaveFile * sfile, BaseContainer * cnt);
    virtual ~ThreadedWriter();

public slots:
    void WritePack();
    void WriteSprite();
    //void OnFinished();

signals:
    void finished();
    void finished(int); //int is the nb of bytes written!

private:
    void _WriteSprite();
    void _WritePack();
};

#endif // THREADEDWRITER_HPP
