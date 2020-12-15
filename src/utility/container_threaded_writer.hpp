#ifndef THREADEDWRITER_HPP
#define THREADEDWRITER_HPP
#include <QObject>
#include <QSaveFile>
#include <QMutex>
#include <QFuture>
#include <src/data/base_container.hpp>

//#FIXME: Generalize this so that the actual container handles the writing, and this only handles the threading!!!

/*
    ContainerThreadedWriter
        Writes the content of a container on a separate thread.
*/

class ContainerThreadedWriter : public QObject
{
    Q_OBJECT
public:
    QScopedPointer<QSaveFile>         savefile;
    BaseContainer                   * container;
    QMutex                            mtxdata;
    int                               bywritten;
    QFuture<void>                     op1;
    QFuture<void>                     op2;

    explicit ContainerThreadedWriter(QSaveFile * sfile, BaseContainer * cnt);
    virtual ~ContainerThreadedWriter();

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
