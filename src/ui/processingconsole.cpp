#include "processingconsole.hpp"
#include "ui_processingconsole.h"

ProcessingConsole::ProcessingConsole(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProcessingConsole)
{
    ui->setupUi(this);
}

ProcessingConsole::~ProcessingConsole()
{
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
