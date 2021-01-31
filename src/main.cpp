#include <src/ui/mainwindow.hpp>
#include <src/ui/processingconsole.hpp>
#include <QApplication>
#include <QList>
#include <QString>
#include <QDebug>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTime>
#include <src/utility/randomgenhelper.hpp>
#include <src/utility/program_settings.hpp>
#include <src/ui/arguments_handler.hpp>
#include <src/utility/ui_helpers.hpp>

const char * ORG_NAME = "PPMDU";
const char * APP_NAME = "Sprite Cruncher";

/*
 * Because Qt doesn't handle exceptions from its message handlers by default,
 * we need to do this here.
*/
class ApplicationNoExcept : public QApplication
{
public:
    using QApplication::QApplication;

    // QCoreApplication interface
public:
    bool notify(QObject *receiver, QEvent *event) override
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch(const std::exception & e)
        {
            qWarning() << "<!>-Exception: " << GetNestedExceptionsText(e);
            return false;
        }
        catch(...)
        {
            qWarning() << "<!>-Caught unknown exception type!";
            return false;
        }
    }
};



inline bool ShouldDisplayUI(const QCommandLineParser & parser)
{
    return cmdline_processing::ParseProcessingMode(parser) == cmdline_processing::eProcessingMode::None;
}

void SetupApplication()
{
    //App constants
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setApplicationName(APP_NAME);

    //RNG seed init
    GetRandomGenerator().seed(QTime(0,0,0).secsTo(QTime::currentTime()));
}

QCommandLineParser & SetupApplicationOptions(ApplicationNoExcept & /*app*/, QCommandLineParser & parser)
{
    //Options
    parser.addOptions(cmdline_processing::PGRM_Options);

    //Positional
    parser.addPositionalArgument("filepath", "File to open with the program!");

    return parser;
}

int main(int argc, char *argv[])
{
    ApplicationNoExcept app(argc, argv);
    QCommandLineParser  parser;
    SetupApplication();
    SetupApplicationOptions(app, parser).process(app);

    cmdline_processing::ArgumentsHandler *  phndlr  = nullptr;  //Interface pointer for handling arguments
    QScopedPointer<QMainWindow>             pmainw;             //Keeps the instance of the main window alive, and properly deletes
    try
    {
        //Should we display the UI?
        if(ShouldDisplayUI(parser))
        {
            qInfo() <<"Staring in GUI mode!\n";
            MainWindow * pm = new MainWindow;
            pmainw.reset(pm);
            phndlr = pm;
        }
        else
        {
            qInfo() <<"Staring in console mode!\n";
            //If not, we're always in processing mode!
            ProcessingConsole * pc = new ProcessingConsole;
            pmainw.reset(pc);
            phndlr = pc;
        }
        pmainw->show();
        pmainw->setFocus();
        phndlr->ProcessArguments(parser);
        return app.exec();
    }
    catch(const std::exception & e)
    {
        qFatal("main(): %s\n", e.what());
        return -1;
    }
}
