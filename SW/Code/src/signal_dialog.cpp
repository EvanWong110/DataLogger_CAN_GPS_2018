#include "signal_dialog.h"
#include "ui_signal_dialog.h"

#include <iostream>
#include <QMessageBox>

SignalDialog::SignalDialog(ConfigFrame& frame, ConfigSignal* pPreviousSignal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignalDialog),
    frame(frame),
    pPreviousSignal(pPreviousSignal)
{
    ui->setupUi(this);
    ui->bigEndian_checkBox->setEnabled(false);
    on_startBit_spinBox_valueChanged(ui->startBit_spinBox->value());
    on_lengthBits_spinBox_valueChanged(ui->lengthBits_spinBox->value());

    if (pPreviousSignal != nullptr){

        ui->signalID_spinBox->setValue(static_cast<int>(pPreviousSignal->getSignalID()));

        if (pPreviousSignal->getValueType().isSignedType()){
            ui->signed_checkBox->setChecked(true);
        }
        if (pPreviousSignal->getValueType().isBigEndianType()){
            ui->bigEndian_checkBox->setChecked(true);
        }

        ui->startBit_spinBox->setValue(static_cast<int>(pPreviousSignal->getStartBit()));
        ui->lengthBits_spinBox->setValue(static_cast<int>(pPreviousSignal->getLengthBits()));
        ui->multiplier_spinBox->setValue(pPreviousSignal->getMultiplier());
        ui->divider_spinBox->setValue(static_cast<int>(pPreviousSignal->getDivider()));
        ui->offset_spinBox->setValue(pPreviousSignal->getOffset());
        ui->signalName_lineEdit->setText(QString::fromStdString(pPreviousSignal->getSignalName()));
        ui->unit_lineEdit->setText(QString::fromStdString(pPreviousSignal->getUnitName()));
        ui->comment_lineEdit->setText(QString::fromStdString(pPreviousSignal->getComment()));
    }
}

unsigned int SignalDialog::getSignalID(){
    return static_cast<unsigned int>(ui->signalID_spinBox->value());
}

bool SignalDialog::getAutoSignalID(){
    return ui->signalIDAuto_checkBox->isChecked();
}

bool SignalDialog::getIsSigned(){
    return ui->signed_checkBox->isChecked();
}

bool SignalDialog::getIsBigEndian(){
    return ((ui->lengthBits_spinBox->value() > 8) && (ui->bigEndian_checkBox->isChecked()));
}

int SignalDialog::getMultiplier(){
    return ui->multiplier_spinBox->value();
}

unsigned int SignalDialog::getDivider(){
    if (ui->divider_spinBox->value() < 0){
        throw std::invalid_argument("Divider must be greater or equal to 0.");
    }
    return static_cast<unsigned int>(ui->divider_spinBox->value());
}

int SignalDialog::getOffset(){
    return ui->offset_spinBox->value();
}

QString SignalDialog::getSignalName(){
    return ui->signalName_lineEdit->text();
}

QString SignalDialog::getUnit(){
    return ui->unit_lineEdit->text();
}

QString SignalDialog::getComment(){
    return ui->comment_lineEdit->text();
}

unsigned int SignalDialog::getStartBit(){
    return static_cast<unsigned int>(ui->startBit_spinBox->value());
}

unsigned int SignalDialog::getLengthBits(){
    return static_cast<unsigned int>(ui->lengthBits_spinBox->value());
}

SignalDialog::~SignalDialog()
{
    delete ui;
}

void SignalDialog::on_signalIDAuto_checkBox_stateChanged(int state)
{
    if (state == 0){
        ui->signalID_spinBox->setEnabled(true);
    } else {
        ui->signalID_spinBox->setEnabled(false);
    }
}

void SignalDialog::on_lengthBits_spinBox_valueChanged(int val)
{
    if (val <= 8){
        ui->bigEndian_checkBox->setEnabled(false);
    } else {
        ui->bigEndian_checkBox->setEnabled(true);
    }
    QString text;
    text.append(QString::number(val / 8));
    text.append(".");
    text.append(QString::number(val % 8));
    ui->lengthByteBit_lineEdit->setText(text);
}

void SignalDialog::on_startBit_spinBox_valueChanged(int val)
{
    QString text;
    text.append(QString::number(val / 8));
    text.append(".");
    text.append(QString::number(val % 8));
    ui->startByteBit_lineEdit->setText(text);
}
