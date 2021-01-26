#ifndef PROGRAMSETTINGS_HPP
#define PROGRAMSETTINGS_HPP

#include <QObject>
#include <QSettings>
#include <src/ui/mainwindow.hpp>
#include <functional>

namespace settings_consts
{
    //--------------------------
    // Settings default values
    //--------------------------
    extern const int    DV_PREVIEW_FRAMERATE;
    extern const bool   DV_PREVIEW_AUTO;

    extern const int    DV_EDITOR_ZOOM_DEF;
    extern const int    DV_EDITOR_ZOOM_INCR;

    extern const bool   DV_ADVANCED_MODE;

    //--------------------------
    // Setting names
    //--------------------------
    extern const QString SETTING_PREVIEW_FRMRATE;
    extern const QString SETTING_PREVIEW_AUTO;

    extern const QString SETTING_EDITOR_ZOOM_DEF;
    extern const QString SETTING_EDITOR_ZOOM_INCR;

    extern const QString SETTING_ADVANCED_MODE;
    extern const QString SETTING_LAST_PROJECT_PATH;

};

/*
 * Singleton manager for program-wide settings
*/
class ProgramSettings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProgramSettings);
    ProgramSettings();

public:
    static const QString GeneralSettingsGroupName; //Name for general settings group
    static  QSettings        & Settings();
    static  ProgramSettings  & Instance();

    //Settings quick access
    bool isAdvancedMode();
    bool isAutoplayEnabled();

    int previewFramerate();
    int editorDefaultZoom();
    int editorZoomIncrements();

    QString lastProjectPath();

public slots:
    void WriteSettings(const QString & groupname, std::function<void(QSettings&)> writefun);
    void ReadSettings (const QString & groupname, std::function<void(QSettings&)> readfun);

    //Save/load settings to file
    void Sync();

private:
    QSettings m_settings;
};

#endif // PROGRAMSETTINGS_HPP
