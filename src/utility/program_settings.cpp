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

    const QString SETTING_EDITOR_ZOOM_DEF   {"editor_zoom_default"};
    const QString SETTING_EDITOR_ZOOM_INCR  {"editor_zoom_increment"};

    const QString SETTING_PREVIEW_FRMRATE   {"preview_framerate"};
    const QString SETTING_PREVIEW_AUTO      {"preview_autoplay"};
};

const QString ProgramSettings::GeneralSettingsGroupName{"General"};

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
}

void ProgramSettings::ReadSettings(const QString & groupname, std::function<void (QSettings&)> readfun)
{
    m_settings.beginGroup(groupname);
    readfun(m_settings);
    m_settings.endGroup();
}

void ProgramSettings::Sync()
{
    m_settings.sync();
}

bool ProgramSettings::isAdvancedMode()
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    bool advanced = m_settings.value(SETTING_ADVANCED_MODE, DV_ADVANCED_MODE).toBool();
    m_settings.endGroup();
    return advanced;
}

bool ProgramSettings::isAutoplayEnabled()
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    bool autop = m_settings.value(SETTING_PREVIEW_AUTO, DV_PREVIEW_AUTO).toBool();
    m_settings.endGroup();
    return autop;
}

int ProgramSettings::previewFramerate()
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    int fpt = m_settings.value(SETTING_PREVIEW_FRMRATE, DV_PREVIEW_FRAMERATE).toInt();
    m_settings.endGroup();
    return fpt;
}

int ProgramSettings::editorDefaultZoom()
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    int zoom = m_settings.value(SETTING_EDITOR_ZOOM_DEF, DV_EDITOR_ZOOM_DEF).toInt();
    m_settings.endGroup();
    return zoom;
}

int ProgramSettings::editorZoomIncrements()
{
    using namespace settings_consts;
    m_settings.beginGroup(GeneralSettingsGroupName);
    int zoomincr = m_settings.value(SETTING_EDITOR_ZOOM_INCR, DV_EDITOR_ZOOM_INCR).toInt();
    m_settings.endGroup();
    return zoomincr;
}
