/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "genealogical_date_editor_dialog.h"

#include "ui_genealogical_date_editor_dialog.h"

#include <QMetaEnum>

GenealogicalDateEditorDialog::GenealogicalDateEditorDialog(const GenealogicalDate& existingDate, QWidget* parent) :
    QDialog(parent),
    date(existingDate),
    ui(new Ui::GenealogicalDateEditorDialog) {
    ui->setupUi(this);
    defaultLineEditStyle = ui->textualLineEdit->styleSheet();

    // Set up the combo box with their enum values.
    const auto modifierMeta = QMetaEnum::fromType<GenealogicalDate::Modifier>();
    for (int i = 0; i < modifierMeta.keyCount(); ++i) {
        auto displayData = QString::fromUtf8(modifierMeta.valueToKey(i)).toLower();
        displayData[0] = displayData[0].toUpper();
        ui->modifierInput->addItem(displayData, modifierMeta.value(i));
    }

    const auto qualityMeta = QMetaEnum::fromType<GenealogicalDate::Quality>();
    for (int i = 0; i < qualityMeta.keyCount(); ++i) {
        auto displayData = QString::fromUtf8(qualityMeta.valueToKey(i)).toLower();
        displayData[0] = displayData[0].toUpper();
        ui->qualityInput->addItem(displayData, qualityMeta.value(i));
    }

    // connect(ui.modifierInput, &QComboBox::currentIndexChanged, this,
    // &GenealogicalDateEditorDialog::updateDateFromForms); connect(ui.qualityInput, &QComboBox::currentIndexChanged,
    // this, &GenealogicalDateEditorDialog::updateDateFromForms); connect(ui.daySpinBox, &QSpinBox::valueChanged, this,
    // &GenealogicalDateEditorDialog::updateDateFromForms); connect(ui.monthSpinBox, &QSpinBox::valueChanged, this,
    // &GenealogicalDateEditorDialog::updateDateFromForms); connect(ui.yearSpinBox, &QSpinBox::valueChanged, this,
    // &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->textualLineEdit, &QLineEdit::textChanged, this, &GenealogicalDateEditorDialog::textualUpdated);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateFormsFromDate();
}

GenealogicalDateEditorDialog::~GenealogicalDateEditorDialog() {
    delete ui;
}

GenealogicalDate GenealogicalDateEditorDialog::editDate(const GenealogicalDate& existingDate, QWidget* parent) {
    GenealogicalDateEditorDialog editWindow{existingDate, parent};
    editWindow.exec();
    return editWindow.date;
}

void GenealogicalDateEditorDialog::accept() {
    updateDateFromForms();
    QDialog::accept();
}
void GenealogicalDateEditorDialog::reject() {
    this->date = {};
    QDialog::reject();
}

void GenealogicalDateEditorDialog::updateDateFromForms() {
    auto modifier = ui->modifierInput->currentData().value<GenealogicalDate::Modifier>();
    auto quality = ui->qualityInput->currentData().value<GenealogicalDate::Quality>();
    const QDate proleptic{ui->yearSpinBox->value(), ui->monthSpinBox->value(), ui->daySpinBox->value()};

    this->date = {modifier, quality, proleptic, true, true, true, QStringLiteral()};
}

void GenealogicalDateEditorDialog::updateFormsFromDate() const {
    const int modifierIndex = ui->modifierInput->findData(date.modifier());
    ui->modifierInput->setCurrentIndex(modifierIndex);
    const int qualityIndex = ui->qualityInput->findData(date.quality());
    ui->qualityInput->setCurrentIndex(qualityIndex);

    auto proleptic = date.prolepticRepresentation();
    if (auto dmy = QCalendar().partsFromDate(proleptic); dmy.isValid()) {
        ui->daySpinBox->setValue(dmy.day);
        ui->monthSpinBox->setValue(dmy.month);
        ui->yearSpinBox->setValue(dmy.year);
    } else {
        ui->daySpinBox->setValue(0);
        ui->monthSpinBox->setValue(0);
        ui->yearSpinBox->setValue(0);
    }
}

void GenealogicalDateEditorDialog::textualUpdated() {
    const auto newValue = ui->textualLineEdit->text();
    if (const auto parsedDate = GenealogicalDate::fromDisplayText(newValue); parsedDate.isValid()) {
        date = parsedDate;
        ui->textualLineEdit->setStyleSheet(defaultLineEditStyle);
    } else {
        ui->textualLineEdit->setStyleSheet(defaultLineEditStyle.append(QStringLiteral("; border: 1px solid red;")));
    }
}
