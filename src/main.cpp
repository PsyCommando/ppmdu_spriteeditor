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

const QString OPT_Export = "e";
const QString OPT_Import = "i";


const QList<QCommandLineOption> PGRM_Options
{
    QCommandLineOption(OPT_Import, "Application will run in import mode!"),
    QCommandLineOption(OPT_Export, "Application will run in export mode!"),
};

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
            qWarning() << "<!>-Exception: " <<e.what();
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
    return !(parser.isSet(OPT_Import) || parser.isSet(OPT_Export));
}

void SetupApplication()
{
    //App constants
    QCoreApplication::setOrganizationName("PPMDU");
    QCoreApplication::setApplicationName("Sprite Crunsher");

    //RNG seed init
    GetRandomGenerator().seed(QTime(0,0,0).secsTo(QTime::currentTime()));
}

QCommandLineParser & SetupApplicationOptions(ApplicationNoExcept & /*app*/, QCommandLineParser & parser)
{
    //Options
    parser.addOptions(PGRM_Options);

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

    QString openFilePath;
    const QStringList positionalArguments = parser.positionalArguments();
    if(!positionalArguments.empty())
        openFilePath = positionalArguments.first();

    try
    {
        //Should we display the UI?
        if( ShouldDisplayUI(parser) )
        {
            qInfo() <<"Staring in GUI mode!\n";
            MainWindow w;
            w.show();
            w.setFocus();
            if(!openFilePath.isEmpty())
                w.LoadContainer(openFilePath);
            return app.exec();
        }
        else
        {
            qInfo() <<"Staring in console mode!\n";
            //If not, we're always in processing mode!
            ProcessingConsole w;
            w.show();

            //##TODO##
            //Execute the tasks here!
            w.WriteLine( "AYY LMAO!" );
            return app.exec();
        }
    }
    catch(const std::exception & e)
    {
        qFatal("main(): %s\n", e.what());
        return -1;
    }
}
