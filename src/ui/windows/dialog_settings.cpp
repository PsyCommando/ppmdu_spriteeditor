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

void DialogSettings::writeSettings(QSettings &/*settings*/)
{
    ProgramSettings & ps = ProgramSettings::Instance(); //Just use the built-in parser for those
    //General
    ps.setAdvancedMode          (ui->chkAdvancedSettings->isChecked());

    //Previews
    ps.setPreviewFramerate      (ui->spbFramerate->value());
    ps.setAutoplayEnabled       (ui->chkAutoplay->isChecked());

    //Editor Settings
    ps.setEditorDefaultZoom     (ui->spbFrmEditZoom->value());
    ps.setEditorZoomIncrements  (ui->spbFrmEditZoomIncr->value());
}

void DialogSettings::readSettings(QSettings &/*settings*/)
{
    ProgramSettings & ps = ProgramSettings::Instance(); //Just use the built-in parser for those
    //General
    ui->chkAdvancedSettings->   setChecked  (ps.isAdvancedMode());

    //Previews
    ui->spbFramerate->          setValue    (ps.previewFramerate());
    ui->chkAutoplay->           setChecked  (ps.isAutoplayEnabled());

    //Editor Settings
    ui->spbFrmEditZoom->        setValue    (ps.editorDefaultZoom());
    ui->spbFrmEditZoomIncr->    setValue    (ps.editorZoomIncrements());
}

void DialogSettings::commitSettings()
{
    ProgramSettings & ps = ProgramSettings::Instance();
    ps.WriteSettings(ProgramSettings::GeneralSettingsGroupName,
                     std::bind(&DialogSettings::writeSettings, this, std::placeholders::_1));
}

void DialogSettings::on_DialogSettings_accepted()
{
    commitSettings();
}
