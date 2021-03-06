#include "trigger_dialog.h"
#include "ui_trigger_dialog.h"
#include <QPushButton>
#include <QMessageBox>

TriggerDialog::TriggerDialog(Config& _config, ConfigTrigger* pTriggerPrototype, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TriggerDialog),
    config(_config),
    constValueChangeInProgress(false)
{
    ui->setupUi(this);

    if (config.framesEmpty()){
        throw std::logic_error("No frames defined in configuration. Please define any frame to be able to define trigger.");
    }

    for (auto oper : ConfigTrigger::getAllCompareOperators()){
        ui->compareOperator_comboBox->addItem(
                    QString::fromStdString(ConfigTrigger::getTriggerCompareOperatorSymbol(oper) + " :: " + ConfigTrigger::getTriggerCompareOperatorName(oper)),
                    QVariant::fromValue(oper)
        );
        if ((pTriggerPrototype != nullptr) && (pTriggerPrototype->getCompareOperator() == oper)){
            ui->compareOperator_comboBox->setCurrentIndex(static_cast<int>(ui->compareOperator_comboBox->count() - 1));
        }
    }

    const ConfigFrame* pPrototypeFrame = nullptr;
    const ConfigSignal* pPrototypeSignal = nullptr;

    if (pTriggerPrototype != nullptr){
        if (ConfigTrigger::isSignalUsedForOperator(pTriggerPrototype->getCompareOperator())){
            pPrototypeSignal = std::get<const ConfigSignal*>(pTriggerPrototype->getFrameSignal());
            pPrototypeFrame = pPrototypeSignal->getParentFrame();
        } else {
            pPrototypeFrame = std::get<const ConfigFrame*>(pTriggerPrototype->getFrameSignal());
        }
    }

    for (auto frameIt = config.cbeginFrames(); frameIt != config.cendFrames(); frameIt++){
        QString text;
        text.append(QString::fromStdString((*frameIt)->getFrameName()));
        text.append(" [");
        text.append("0x");
        text.append(QString::number((*frameIt)->getFrameID(), 16));
        text.append("]");
        ui->frame_comboBox->addItem(text, QVariant::fromValue(*frameIt));

        if (pPrototypeFrame == (*frameIt)){
            ui->frame_comboBox->setCurrentIndex(static_cast<int>(std::distance(config.cbeginFrames(), frameIt)));
            //<----- here is automaticly generated on_frame_comboBox_currentIndexChanged ----->//
            for (int index = 0; index < ui->signal_comboBox->count(); index++){
                if (ui->signal_comboBox->itemData(index).value<ConfigSignal*>() == pPrototypeSignal){
                    ui->signal_comboBox->setCurrentIndex(index);
                }
            }
        }
    }
    //<----- here is automaticly generated on_frame_comboBox_currentIndexChanged ----->//

    if (pTriggerPrototype != nullptr){
        ui->triggerName_lineEdit->setText(QString::fromStdString(pTriggerPrototype->getTriggerName()));
        ui->rawConstValue_spinBox->setValue(static_cast<int>(pTriggerPrototype->getCompareConstValue()));
    }

    reloadFormulaRender();
}

ConfigTrigger::FrameSignalVariant TriggerDialog::getSelectedFrameSignal() const
{
    if (ConfigTrigger::isSignalUsedForOperator(getCompareOperator())){
        return ConfigTrigger::FrameSignalVariant(getSelectedSignal());
    } else {
        return ConfigTrigger::FrameSignalVariant(getSelectedFrame());
    }
}

ConfigTrigger::TriggerCompareOperator TriggerDialog::getCompareOperator() const
{
    if (ui->compareOperator_comboBox->currentIndex() == -1){
        throw std::logic_error("Compare operator not selected.");
    }
    return ui->compareOperator_comboBox->itemData(ui->compareOperator_comboBox->currentIndex()).value<ConfigTrigger::TriggerCompareOperator>();
}

unsigned int TriggerDialog::getConstCompareValue() const
{
    return static_cast<unsigned int>(ui->rawConstValue_spinBox->value());
}

QString TriggerDialog::getTriggerName() const
{
    return ui->triggerName_lineEdit->text();
}

QString TriggerDialog::getFormulaRenderValue() const
{
    return ui->formulaRender_lineEdit->text();
}

TriggerDialog::~TriggerDialog()
{
    delete ui;
}

QString TriggerDialog::prepareFormulaRender(ConfigTrigger::FrameSignalVariant _frameSignalVariant, ConfigTrigger::TriggerCompareOperator oper, unsigned long rawConstCompareVal)
{
    QString strVal;

    if (ConfigTrigger::isSignalUsedForOperator(oper)){
        if (!std::holds_alternative<const ConfigSignal*>(_frameSignalVariant)){
            throw std::logic_error("Signal must be defined for given compare operator.");
        }
        strVal += QString::fromStdString(std::get<const ConfigSignal*>(_frameSignalVariant)->getParentFrame()->getFrameName());
        strVal += " [0x";
        strVal += QString::number(std::get<const ConfigSignal*>(_frameSignalVariant)->getParentFrame()->getFrameID(), 16);
        strVal += "]::";
        strVal += QString::fromStdString(std::get<const ConfigSignal*>(_frameSignalVariant)->getSignalName());
    } else {
        if (!std::holds_alternative<const ConfigFrame*>(_frameSignalVariant)){
            throw std::logic_error("Frame must be defined for given compare operator.");
        }
        strVal += QString::fromStdString(std::get<const ConfigFrame*>(_frameSignalVariant)->getFrameName());
        strVal += " [0x";
        strVal += QString::number(std::get<const ConfigFrame*>(_frameSignalVariant)->getFrameID(), 16);
        strVal += "]";
    }

    strVal += " ";
    strVal += QString::fromStdString(ConfigTrigger::getTriggerCompareOperatorSymbol(oper));
    strVal += " ";

    if (ConfigTrigger::isConstCompareValueUsedForOperator(oper)){
        if (ConfigTrigger::isSignalUsedForOperator(oper)){
            strVal += QString::number(std::get<const ConfigSignal*>(_frameSignalVariant)->convertRawValueToSymbolic(rawConstCompareVal), 'f', 2);
        } else {
            strVal += QString::number(rawConstCompareVal);
        }
    }
    return strVal;
}

void TriggerDialog::on_signal_comboBox_currentIndexChanged(int index)
{
    if (index == -1){
        disableSignalRelativeWidgets(false);
        return;
    }
    try {
        ConfigSignal* pSignal = ui->signal_comboBox->itemData(index).value<ConfigSignal*>();
        enableSignalRelativeWidgets(pSignal);
        on_rawConstValue_spinBox_valueChanged(ui->rawConstValue_spinBox->value());
        ui->symbolicConstValue_doubleSpinBox->setSingleStep(pSignal->convertRawValueToSymbolic(1));
        reloadFormulaRender();
    } catch (const std::logic_error& e){
        QMessageBox::warning(this, "Error", e.what());
    } catch (const std::exception& e){
        QMessageBox::warning(this, "Error", QString("Unkonwn error occured: ") + QString(e.what()));
    }
}

void TriggerDialog::on_frame_comboBox_currentIndexChanged(int index)
{
    if (index == -1){
        return;
    }
    try {
        ConfigFrame* pFrame = getSelectedFrame();
        ui->signal_comboBox->clear();

        if (pFrame->signalsEmpty()){
            on_compareOperator_comboBox_currentIndexChanged(ui->compareOperator_comboBox->currentIndex()); //< update signal combo box according to chosen compare operator
        } else {
            for (auto signalIt = pFrame->cbeginSignals(); signalIt != pFrame->cendSignals(); signalIt++){
                ui->signal_comboBox->addItem(QString::fromStdString((*signalIt)->getSignalName()), QVariant::fromValue(*signalIt));
            }
        }
        reloadFormulaRender();
    } catch (const std::logic_error& e){
        QMessageBox::warning(this, "Error", e.what());
    } catch (const std::exception& e){
        QMessageBox::warning(this, "Error", QString("Unkonwn error occured: ") + QString(e.what()));
    }
}

void TriggerDialog::on_symbolicConstValue_doubleSpinBox_valueChanged(double val)
{
    if (constValueChangeInProgress){
        return;
    }
    try {
        constValueChangeInProgress = true;
        ConfigSignal* pSignal = nullptr;
        try {
            pSignal = getSelectedSignal();
        } catch (const std::logic_error&) {
            return;
        }
        if (pSignal != nullptr){
            unsigned long rawVal = pSignal->convertSymbolicValueToRaw(val);
            ui->rawConstValue_spinBox->setValue(static_cast<int>(rawVal));
        }
        constValueChangeInProgress = false;
        reloadFormulaRender();
    } catch (const std::logic_error& e){
        QMessageBox::warning(this, "Error", e.what());
    } catch (const std::exception& e){
        QMessageBox::warning(this, "Error", QString("Unkonwn error occured: ") + QString(e.what()));
    }
}

void TriggerDialog::on_rawConstValue_spinBox_valueChanged(int rawVal)
{
    if (constValueChangeInProgress){
        return;
    }
    try {
        constValueChangeInProgress = true;
        const ConfigSignal* pSignal = nullptr;
        try {
            pSignal = getSelectedSignal();
        } catch (const std::logic_error&) {
            return;
        }
        if (pSignal != nullptr){
            double computedValue = pSignal->convertRawValueToSymbolic(static_cast<unsigned int>(rawVal));
            ui->symbolicConstValue_doubleSpinBox->setValue(computedValue);
        }
        constValueChangeInProgress = false;
        reloadFormulaRender();
    } catch (const std::logic_error& e){
        QMessageBox::warning(this, "Error", e.what());
    } catch (const std::exception& e){
        QMessageBox::warning(this, "Error", QString("Unkonwn error occured: ") + QString(e.what()));
    }
}

void TriggerDialog::on_compareOperator_comboBox_currentIndexChanged(int)
{
    try {
        if (ConfigTrigger::isSignalUsedForOperator(getCompareOperator())){
            try {
                ConfigSignal* pSignal = getSelectedSignal();
                if (pSignal != nullptr){
                    enableSignalRelativeWidgets(pSignal);
                } else {
                    disableSignalRelativeWidgets(true);
                }
            } catch (const std::logic_error&){
                disableSignalRelativeWidgets(false);
            }
        } else {
            disableSignalRelativeWidgets(true);
        }
        if (ConfigTrigger::isConstCompareValueUsedForOperator(getCompareOperator())){
            ui->rawConstValue_spinBox->setEnabled(true);
        } else {
            ui->rawConstValue_spinBox->setEnabled(false);
        }
        reloadFormulaRender();
    } catch (const std::logic_error& e){
        QMessageBox::warning(this, "Error", e.what());
    } catch (const std::exception& e){
        QMessageBox::warning(this, "Error", QString("Unkonwn error occured: ") + QString(e.what()));
    }
}

ConfigFrame *TriggerDialog::getSelectedFrame() const
{
    if (ui->frame_comboBox->currentIndex() == -1){
        throw std::logic_error("No frame selected.");
    }
    return ui->frame_comboBox->itemData(ui->frame_comboBox->currentIndex()).value<ConfigFrame*>();
}

/**
 * @brief TriggerDialog::getSelectedSignal
 * @return nullptr if signal is not used for compare operator or signal pointer used with given compare oparetion
 * @throws std::logic_error when operator requires signal pointer, but signal for some reason is not possible to obtain.
 */
ConfigSignal *TriggerDialog::getSelectedSignal() const
{
    if (!ConfigTrigger::isSignalUsedForOperator(getCompareOperator())){
        return nullptr;
    }
    if (ui->signal_comboBox->currentIndex() == -1){
        throw std::logic_error("No signal selected.");
    }
    return ui->signal_comboBox->itemData(ui->signal_comboBox->currentIndex()).value<ConfigSignal*>();
}

void TriggerDialog::disableSignalRelativeWidgets(bool disableSignalNameWidget)
{
    ui->symbolicConstValue_doubleSpinBox->setEnabled(false);
    ui->symbolicConstValue_doubleSpinBox->clear();
    ui->symbolicConstValue_doubleSpinBox->setSuffix("");
    if (disableSignalNameWidget){
        ui->signal_comboBox->setEnabled(false);
    } else {
        ui->signal_comboBox->setEnabled(true);
    }
}

void TriggerDialog::enableSignalRelativeWidgets(const ConfigSignal* pSignal)
{
    ui->symbolicConstValue_doubleSpinBox->setEnabled(true);
    ui->symbolicConstValue_doubleSpinBox->setValue(
                pSignal->convertRawValueToSymbolic(
                    static_cast<unsigned long>(ui->rawConstValue_spinBox->value())
                ));
    ui->signal_comboBox->setEnabled(true);
    ui->symbolicConstValue_doubleSpinBox->setSuffix(" " + QString::fromStdString(pSignal->getUnitName()));
}

void TriggerDialog::reloadFormulaRender()
{
    QString strVal;
    try {
        ui->formulaRender_lineEdit->setText(prepareFormulaRender(getSelectedFrameSignal(), getCompareOperator(), getConstCompareValue()));
    } catch (const logic_error&) {
        ui->formulaRender_lineEdit->clear();
        return;
    }
}
