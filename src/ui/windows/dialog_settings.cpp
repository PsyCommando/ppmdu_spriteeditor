#include "dialog_settings.hpp"
#include "ui_dialog_settings.h"
#include <src/utility/program_settings.hpp>

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    ProgramSettings::Instance().ReadSettings(ProgramSettings::GeneralSettingsGroupName,
                                              std::bind(&DialogSettings::readSettings, this, std::placeholders::_1));
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::writeSettings(QSettings &settings)
{
    using namespace settings_consts;
    settings.setValue(SETTING_ADVANCED_MODE,    ui->chkAdvancedSettings->isChecked());
    settings.setValue(SETTING_PREVIEW_FRMRATE,  ui->spbFramerate->value());
    settings.setValue(SETTING_PREVIEW_AUTO,     ui->chkAutoplay->isChecked());

    //Editor Settings
    settings.setValue(SETTING_EDITOR_ZOOM_DEF,  ui->spbFrmEditZoom->value());
    settings.setValue(SETTING_EDITOR_ZOOM_INCR, ui->spbFrmEditZoomIncr->value());
}

void DialogSettings::readSettings(QSettings &/*settings*/)
{
    ProgramSettings & ps = ProgramSettings::Instance();
    ui->chkAdvancedSettings->   setChecked  (ps.isAdvancedMode());
    ui->spbFramerate->          setValue    (ps.previewFramerate());
    ui->chkAutoplay->           setChecked  (ps.isAutoplayEnabled());

    //Editor Settings
    ui->spbFrmEditZoom->        setValue    (ps.editorDefaultZoom());
    ui->spbFrmEditZoomIncr->    setValue    (ps.editorZoomIncrements());
}

void DialogSettings::commitSettings()
{
    ProgramSettings::Instance().WriteSettings(ProgramSettings::GeneralSettingsGroupName,
                                              std::bind(&DialogSettings::writeSettings, this, std::placeholders::_1));
}

void DialogSettings::on_DialogSettings_accepted()
{
    commitSettings();
}
