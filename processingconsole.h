#ifndef PROCESSINGCONSOLE_H
#define PROCESSINGCONSOLE_H

#include <QMainWindow>
#include <QString>
#include <QDebug>

namespace Ui {
class ProcessingConsole;
}

class ProcessingConsole : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProcessingConsole(QWidget *parent = 0);
    ~ProcessingConsole();

    void WriteLine( const QString & line );
    void Write( const QString & line );
    void Clear();

private:
    Ui::ProcessingConsole *ui;
    QString                m_text;
};

#endif // PROCESSINGCONSOLE_H
