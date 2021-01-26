#ifndef ARGUMENTS_HANDLER_HPP
#define ARGUMENTS_HANDLER_HPP
#include <QCommandLineParser>
#include <map>

namespace cmdline_processing
{
//========================================
// Constants
//========================================
    //Enum for positional args
    enum struct ePositionArgs : int
    {
        SourcePath      = 0,
        DestinationPath,
    };


    //Are we trying to export from the game, or import into the game's files
    enum struct eProcessingMode : int
    {
        Import,
        Export,
        None,  //Error value
    };

    //Options names
    extern const QString OPT_Export;
    extern const QString OPT_Import;

    //List of all options names
    extern const QList<QCommandLineOption> PGRM_Options;

//========================================
// Interface
//========================================
    /*
     * Interface for both UI mode to handle arguments
    */
    class ArgumentsHandler
    {
    public:
        virtual void ProcessArguments(const QCommandLineParser & parser) = 0;
    };

//========================================
// Functions
//========================================

    //Retrieve from the commandline arguments the source file path
    QString ParseSourceFilepath(const QCommandLineParser & parser);

    //Retrieve from the commandline arguments the destination/target file path
    QString ParseDestinationFilepath(const QCommandLineParser & parser);

    //Retrieve the processing mode from the command line
    eProcessingMode ParseProcessingMode(const QCommandLineParser & parser);

};
#endif // ARGUMENTS_HANDLER_HPP
