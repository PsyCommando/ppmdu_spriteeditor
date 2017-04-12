#include "mainwindow.h"
#include "processingconsole.h"
#include <QApplication>
#include <QList>
#include <QString>
#include <QCommandLineOption>
#include <QCommandLineParser>

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
    QCommandLineParser  parser;
    parser.addOptions(PGRM_Options);
    parser.process(a);

//    try
//    {
        //Should we display the UI?
        if( ShouldDisplayUI(parser) )
        {
            MainWindow w;
            w.show();
            w.setFocus();
            return a.exec();
        }
        else
        {
            //If not, we're always in processing mode!
            ProcessingConsole w;
            w.show();

            //##TODO##
            //Execute the tasks here!
            w.WriteLine( "AYY LMAO!" );


            return a.exec();
        }
//    }
//    catch(const std::exception & e)
//    {
//        //std::cerr << e.what() <<"\n";
//        return -1;
//    }
}
