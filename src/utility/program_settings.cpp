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
    const QString SETTING_LAST_EXPORT_PATH  {"last_export_path"};
    const QString SETTING_LAST_IMPORT_PATH  {"last_import_path"};

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
    if(m_settings.group() != groupname)
        m_settings.beginGroup(groupname);
    writefun(m_settings);
    m_settings.endGroup();
    Sync();
}

void ProgramSettings::ReadSettings(const QString & groupname, std::function<void (QSettings&)> readfun)
{
    CheckSettings();
    if(m_settings.group() != groupname)
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

QVariant ProgramSettings::getGeneralSetting(const QString &key, const QVariant &defval)
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(GeneralSettingsGroupName);
    QVariant var = m_settings.value(key, defval);
    m_settings.endGroup();
    return var;
}

QVariant ProgramSettings::getUserSetting(const QString &key, const QVariant &defval)
{
    using namespace settings_consts;
    CheckSettings();
    m_settings.beginGroup(UserSettingsGroupName);
    QVariant var = m_settings.value(key, defval);
    m_settings.endGroup();
    return var;
}

bool ProgramSettings::isAdvancedMode()
{
    using namespace settings_consts;
    bool advanced = getGeneralSetting(SETTING_ADVANCED_MODE, DV_ADVANCED_MODE).toBool();
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
    bool autop = getGeneralSetting(SETTING_PREVIEW_AUTO, DV_PREVIEW_AUTO).toBool();
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
    int fpt = getGeneralSetting(SETTING_PREVIEW_FRMRATE, DV_PREVIEW_FRAMERATE).toInt();
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
    int zoom = getGeneralSetting(SETTING_EDITOR_ZOOM_DEF, DV_EDITOR_ZOOM_DEF).toInt();
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
    int zoomincr = getGeneralSetting(SETTING_EDITOR_ZOOM_INCR, DV_EDITOR_ZOOM_INCR).toInt();
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
    QString lastpath = getUserSetting(SETTING_LAST_PROJECT_PATH, "").toString();
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

QString ProgramSettings::lastExportPath()
{
    using namespace settings_consts;
    QString lastpath = getUserSetting(SETTING_LAST_EXPORT_PATH, "").toString();
    return lastpath;
}

void ProgramSettings::setLastExportPath(QString path)
{
    using namespace settings_consts;
    m_settings.beginGroup(UserSettingsGroupName);
    m_settings.setValue(SETTING_LAST_EXPORT_PATH, path);
    m_settings.endGroup();
    Sync();
}

QString ProgramSettings::lastImportPath()
{
    using namespace settings_consts;
    QString lastpath = getUserSetting(SETTING_LAST_IMPORT_PATH, "").toString();
    return lastpath;
}

void ProgramSettings::setLastImportPath(QString path)
{
    using namespace settings_consts;
    m_settings.beginGroup(UserSettingsGroupName);
    m_settings.setValue(SETTING_LAST_IMPORT_PATH, path);
    m_settings.endGroup();
    Sync();
}
