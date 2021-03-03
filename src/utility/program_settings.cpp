#include "program_settings.hpp"
#include <QApplication>

namespace settings_consts
{
    //
    // Settings Default Values
    //
    const int   DV_PREVIEW_FRAMERATE    {24};
    const bool  DV_PREVIEW_AUTO         {true};

    //
    const int   DV_EDITOR_ZOOM_DEF      {100};
    const int   DV_EDITOR_ZOOM_INCR     {50};

    const bool  DV_ADVANCED_MODE        {false};


    //
    // Settings Name
    //
    const QString SETTING_ADVANCED_MODE     {"advanced_mode"};
    const QString SETTING_LAST_PROJECT_PATH {"last_project_path"};

    const QString SETTING_EDITOR_ZOOM_DEF   {"editor_zoom_default"};
    const QString SETTING_EDITOR_ZOOM_INCR  {"editor_zoom_increment"};

    const QString SETTING_PREVIEW_FRMRATE   {"preview_framerate"};
    const QString SETTING_PREVIEW_AUTO      {"preview_autoplay"};
};

const QString ProgramSettings::UserSettingsGroupName    {"User"};
const QString ProgramSettings::GeneralSettingsGroupName {"General"};

ProgramSettings::ProgramSettings()
    :QObject(nullptr),
     m_settings(QSettings::Format::IniFormat,
                QSettings::Scope::UserScope,
                QApplication::organizationName(),
                QApplication::applicationName())
{
}

QSettings &ProgramSettings::Settings()
{
    return Instance().m_settings;
}

ProgramSettings &ProgramSettings::Instance()
{
    static ProgramSettings s_instance;
    return s_instance;
}

void ProgramSettings::WriteSettings(const QString & groupname, std::function<void (QSettings&)> writefun)
{
    m_settings.beginGroup(groupname);
    writefun(m_settings);
    m_settings.endGroup();
    Sync();
}

void ProgramSettings::ReadSettings(const QString & groupname, std::function<void (QSettings&)> readfun)
{
    CheckSettings();
    m_settings.beginGroup(groupname);
    readfun(m_settings);
    m_settings.endGroup();
}

void ProgramSettings::Sync()
{
    m_settings.sync();
}

bool ProgramSettings::CheckSettings() const
{
    if(m_settings.status() != QSettings::Status::NoError)
    {
        qWarning()  << "ProgramSettings: Error reading settings ini file! "
                    << ((m_settings.status() == QSettings::Status::FormatError)?
                            ("Formatting error!") :
                            ("File access error!"));
        return false;
    }
    return true;
}

bool ProgramSettings::CheckKeyPresent(const QString &key) const
{
    bool bkeypresent = m_settings.contains(key);
    if(!bkeypresent)
        qWarning() << "ProgramSettings::CheckKeyPresent(): Missing key \"" << key <<"\"!";
    return bkeypresent;
}

bool ProgramSettings::isAdvancedMode()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    if(!CheckKeyPresent(SETTING_ADVANCED_MODE))
        m_settings.setValue(SETTING_ADVANCED_MODE, DV_ADVANCED_MODE);
    bool advanced = m_settings.value(SETTING_ADVANCED_MODE, DV_ADVANCED_MODE).toBool();
    m_settings.endGroup();
    return advanced;
}

void ProgramSettings::setAdvancedMode(bool bon)
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    m_settings.setValue(SETTING_ADVANCED_MODE, bon);
    m_settings.endGroup();
    Sync();
}

bool ProgramSettings::isAutoplayEnabled()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    CheckKeyPresent(SETTING_PREVIEW_AUTO);
    bool autop = m_settings.value(SETTING_PREVIEW_AUTO, DV_PREVIEW_AUTO).toBool();
    m_settings.endGroup();
    return autop;
}

void ProgramSettings::setAutoplayEnabled(bool bon)
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    m_settings.setValue(SETTING_PREVIEW_AUTO, bon);
    m_settings.endGroup();
    Sync();
}

int ProgramSettings::previewFramerate()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    CheckKeyPresent(SETTING_PREVIEW_FRMRATE);
    int fpt = m_settings.value(SETTING_PREVIEW_FRMRATE, DV_PREVIEW_FRAMERATE).toInt();
    m_settings.endGroup();
    return fpt;
}

void ProgramSettings::setPreviewFramerate(int frmpt)
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    m_settings.setValue(SETTING_PREVIEW_FRMRATE, frmpt);
    m_settings.endGroup();
    Sync();
}

int ProgramSettings::editorDefaultZoom()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    CheckKeyPresent(SETTING_EDITOR_ZOOM_DEF);
    int zoom = m_settings.value(SETTING_EDITOR_ZOOM_DEF, DV_EDITOR_ZOOM_DEF).toInt();
    m_settings.endGroup();
    return zoom;
}

void ProgramSettings::setEditorDefaultZoom(int percent)
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    m_settings.setValue(SETTING_EDITOR_ZOOM_DEF, percent);
    m_settings.endGroup();
    Sync();
}

int ProgramSettings::editorZoomIncrements()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    CheckKeyPresent(SETTING_EDITOR_ZOOM_INCR);
    int zoomincr = m_settings.value(SETTING_EDITOR_ZOOM_INCR, DV_EDITOR_ZOOM_INCR).toInt();
    m_settings.endGroup();
    return zoomincr;
}

void ProgramSettings::setEditorZoomIncrements(int percent)
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    m_settings.setValue(SETTING_EDITOR_ZOOM_INCR, percent);
    m_settings.endGroup();
    Sync();
}

QString ProgramSettings::lastProjectPath()
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(UserSettingsGroupName);
    CheckKeyPresent(SETTING_LAST_PROJECT_PATH);
    QString lastpath = m_settings.value(SETTING_LAST_PROJECT_PATH, "").toString();
    m_settings.endGroup();
    return lastpath;
}

void ProgramSettings::setLastProjectPath(QString path)
{
    using namespace settings_consts;
    m_settings.beginGroup(UserSettingsGroupName);
    m_settings.setValue(SETTING_LAST_PROJECT_PATH, path);
    m_settings.endGroup();
    Sync();
}
