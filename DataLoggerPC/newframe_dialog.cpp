#include "newframe_dialog.h"
#include "ui_newframe_dialog.h"

#include <QMessageBox>

NewFrameDialog::NewFrameDialog(Config& config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFrameDialog),
    config(config)
{
    ui->setupUi(this);
}

NewFrameDialog::NewFrameDialog(int id, QString moduleName, Config& config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFrameDialog),
    config(config)
{
    ui->setupUi(this);
    ui->id_SpinBox->setValue(id);
    ui->moduleName_lineEdit->setText(moduleName);
}

int NewFrameDialog::getID(){
    return ui->id_SpinBox->value();
}

QString NewFrameDialog::getModuleName(){
    return ui->moduleName_lineEdit->text();
}

NewFrameDialog::~NewFrameDialog()
{
    delete ui;
}

void NewFrameDialog::on_buttonBox_accepted()
{
    if (!config.has_frame_with_id(ui->id_SpinBox->value())){
        accept();
    } else {
        QMessageBox::warning(this, "Wrong ID Error", "Frame with given ID Exists in configuration. Choose other ID.");
    }
}
