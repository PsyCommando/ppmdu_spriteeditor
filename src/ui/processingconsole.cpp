#include "processingconsole.hpp"
#include "ui_processingconsole.h"
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <src/ppmdu/fmts/wa_sprite.hpp>

ProcessingConsole::ProcessingConsole(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProcessingConsole)
{
    using namespace std::placeholders;
    ui->setupUi(this);

    //Setup binding for calling the message handler
    m_handler = std::bind(&ProcessingConsole::MessageHandler, this, _1,  _2, _3);
    qInstallMessageHandler(*m_handler.target<QtMessageHandler>());
}

ProcessingConsole::~ProcessingConsole()
{
    //Clear message binding
    qInstallMessageHandler(0);
    delete ui;
}

void ProcessingConsole::WriteLine(const QString &line)
{
    Write(line + "\n");
}

void ProcessingConsole::Write(const QString &line)
{
    m_text.append(line);
    ui->txtConsoleOut->document()->setHtml(m_text);
}

void ProcessingConsole::Clear()
{
    m_text.clear();
    ui->txtConsoleOut->clear();
    ui->txtConsoleOut->document()->setHtml(m_text);
}

void ProcessingConsole::ProcessArguments(const QCommandLineParser &parser)
{
    //#TODO: Expand on this
    QString openFilePath = cmdline_processing::ParseSourceFilepath(parser);
}

void ProcessingConsole::MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    QString preffix;
    switch (type)
    {
    case QtDebugMsg:
        preffix = "Debug:";
        break;
    case QtWarningMsg:
        preffix = "Warning:";
        break;
    case QtCriticalMsg:
        preffix = "Critical:";
        break;
    case QtFatalMsg:
        preffix = "Fatal:";
        break;
    case QtInfoMsg:
    default:
        break;
    }

    QString newline;
    if(type != QtInfoMsg)
    {
        newline = QString("%1 %2 (%3:%4, %5)\n").arg(preffix).arg(localMsg.constData()).arg(file).arg(context.line).arg(function);
    }
    else
    {
        newline = QString("%1\n").arg(localMsg.constData());
    }
    m_textBuffer.push_back(newline);
    ui->txtConsoleOut->update();
    emit bufferUpdated();
}

