#include <src/ui/mainwindow.hpp>
#include <src/ui/processingconsole.hpp>
#include <QApplication>
#include <QList>
#include <QString>
#include <QDebug>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTime>
#include <src/randomgenhelper.hpp>

//#include <iostream>

const QString OPT_Export = "e";
const QString OPT_Import = "i";


const QList<QCommandLineOption> PGRM_Options
{
    QCommandLineOption(OPT_Import, "Application will run in import mode!"),
    QCommandLineOption(OPT_Export, "Application will run in export mode!"),
};


inline bool ShouldDisplayUI(const QCommandLineParser & parser)
{
    return !(parser.isSet(OPT_Import) || parser.isSet(OPT_Export));
}

int main(int argc, char *argv[])
{
    QApplication        a(argc, argv);
    QCoreApplication::setOrganizationName("PPMDU");
    QCoreApplication::setApplicationName("Sprite Muncher");
    QCommandLineParser  parser;
    //random generator
    GetRandomGenerator().seed(QTime(0,0,0).secsTo(QTime::currentTime()));

    parser.addOptions(PGRM_Options);
    parser.addPositionalArgument("filepath", "File to open with the program!");
    parser.process(a);

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
            return a.exec();
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
            return a.exec();
        }
    }
    catch(const std::exception & e)
    {
        qFatal("main(): %s\n", e.what());
        return -1;
    }
}
