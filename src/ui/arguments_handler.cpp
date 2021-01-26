#include "arguments_handler.hpp"

namespace cmdline_processing
{
    const QString OPT_Export = "e";
    const QString OPT_Import = "i";

    const QList<QCommandLineOption> PGRM_Options
    {
        QCommandLineOption(OPT_Import, "Application will run in import mode!"),
        QCommandLineOption(OPT_Export, "Application will run in export mode!"),
    };

//====================================
// Functions
//====================================
    QString ParsePositional(const QCommandLineParser &parser, ePositionArgs pos)
    {
        QString parsed;
        const QStringList positionalArguments = parser.positionalArguments();
        if(!positionalArguments.empty())
            parsed = positionalArguments.at(static_cast<int>(pos));
        return parsed;
    }

    QString ParseSourceFilepath(const QCommandLineParser &parser)
    {
        return ParsePositional(parser, ePositionArgs::SourcePath);
    }

    QString ParseDestinationFilepath(const QCommandLineParser &parser)
    {
        return ParsePositional(parser, ePositionArgs::DestinationPath);
    }

    eProcessingMode ParseProcessingMode(const QCommandLineParser &parser)
    {
        if(parser.isSet(OPT_Import))
            return eProcessingMode::Import;
        else if(parser.isSet(OPT_Export))
            return eProcessingMode::Export;
        else
            return eProcessingMode::None;
    }

};

