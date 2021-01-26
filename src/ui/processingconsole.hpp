#ifndef PROCESSINGCONSOLE_H
#define PROCESSINGCONSOLE_H

#include <QMainWindow>
#include <QString>
#include <QDebug>
#include <QCommandLineParser>
#include <functional>
#include <src/ui/arguments_handler.hpp>

namespace Ui {
class ProcessingConsole;
}

class ProcessingConsole : public QMainWindow, public cmdline_processing::ArgumentsHandler
{
    Q_OBJECT

public:
    explicit ProcessingConsole(QWidget *parent = 0);
    ~ProcessingConsole();

    void WriteLine( const QString & line );
    void Write( const QString & line );
    void Clear();

    // ArgumentsHandler interface
public:
    void ProcessArguments(const QCommandLineParser &parser) override;

private:
    void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

signals:
     void bufferUpdated();

private:
    Ui::ProcessingConsole *ui;
    QString                m_text;
    std::function<void(QtMsgType,const QMessageLogContext&,const QString &)> m_handler;
    QStringList         m_textBuffer;
};

#endif // PROCESSINGCONSOLE_H
