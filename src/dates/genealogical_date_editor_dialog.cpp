/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "genealogical_date_editor_dialog.h"

#include "ui_genealogical_date_editor_dialog.h"

#include <KLocalizedString>

using namespace Qt::Literals::StringLiterals;

GenealogicalDateEditorDialog::GenealogicalDateEditorDialog(const GenealogicalDate& existingDate, QWidget* parent) :
    QDialog(parent),
    date(existingDate),
    ui(new Ui::GenealogicalDateEditorDialog) {
    ui->setupUi(this);
    defaultLineEditStyle = ui->textualLineEdit->styleSheet();

    // Type combo
    const QList<QPair<GenealogicalDate::DateType, QString>> typeItems = {
        {GenealogicalDate::SINGLE, i18nc("date type", "Single")},
        {GenealogicalDate::RANGE, i18nc("date type", "Range (between … and …)")},
        {GenealogicalDate::SPAN, i18nc("date type", "Span (from … to …)")},
    };
    for (auto& [value, label]: typeItems) {
        ui->typeInput->addItem(label, QVariant::fromValue(value));
    }

    // Modifier combo
    const QList<QPair<GenealogicalDate::Modifier, QString>> modifierItems = {
        {GenealogicalDate::NONE, i18nc("date modifier", "None")},
        {GenealogicalDate::BEFORE, i18nc("date modifier", "Before")},
        {GenealogicalDate::AFTER, i18nc("date modifier", "After")},
        {GenealogicalDate::ABOUT, i18nc("date modifier", "About")},
        {GenealogicalDate::DURING, i18nc("date modifier", "During")},
    };
    for (auto& [value, label]: modifierItems) {
        ui->modifierInput->addItem(label, QVariant::fromValue(value));
    }

    // Quality combo
    const QList<QPair<GenealogicalDate::Quality, QString>> qualityItems = {
        {GenealogicalDate::EXACT, i18nc("date quality", "Exact")},
        {GenealogicalDate::ESTIMATED, i18nc("date quality", "Estimated")},
        {GenealogicalDate::CALCULATED, i18nc("date quality", "Calculated")},
    };
    for (auto& [value, label]: qualityItems) {
        ui->qualityInput->addItem(label, QVariant::fromValue(value));
    }

    connect(ui->typeInput, &QComboBox::currentIndexChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(
        ui->modifierInput,
        &QComboBox::currentIndexChanged,
        this,
        &GenealogicalDateEditorDialog::updateDateFromForms
    );
    connect(
        ui->qualityInput,
        &QComboBox::currentIndexChanged,
        this,
        &GenealogicalDateEditorDialog::updateDateFromForms
    );
    connect(ui->daySpinBox, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->monthSpinBox, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->yearSpinBox, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasDayCheck, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasMonthCheck, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasTimeCheck, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->timeEdit, &QTimeEdit::timeChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, [this](const QDate& d) {
        const QSignalBlocker b1(ui->daySpinBox);
        const QSignalBlocker b2(ui->monthSpinBox);
        const QSignalBlocker b3(ui->yearSpinBox);
        ui->daySpinBox->setValue(d.day());
        ui->monthSpinBox->setValue(d.month());
        ui->yearSpinBox->setValue(d.year());
        updateDateFromForms();
    });
    connect(ui->daySpinBox2, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->monthSpinBox2, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->yearSpinBox2, &QSpinBox::valueChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasDay2Check, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasMonth2Check, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->hasTime2Check, &QCheckBox::toggled, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->timeEdit2, &QTimeEdit::timeChanged, this, &GenealogicalDateEditorDialog::updateDateFromForms);
    connect(ui->dateEdit2, &QDateEdit::dateChanged, this, [this](const QDate& d) {
        const QSignalBlocker b1(ui->daySpinBox2);
        const QSignalBlocker b2(ui->monthSpinBox2);
        const QSignalBlocker b3(ui->yearSpinBox2);
        ui->daySpinBox2->setValue(d.day());
        ui->monthSpinBox2->setValue(d.month());
        ui->yearSpinBox2->setValue(d.year());
        updateDateFromForms();
    });
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
    const auto type = ui->typeInput->currentData().value<GenealogicalDate::DateType>();
    const auto quality = ui->qualityInput->currentData().value<GenealogicalDate::Quality>();

    const bool isRangeOrSpan = (type == GenealogicalDate::RANGE || type == GenealogicalDate::SPAN);
    ui->modifierInput->setEnabled(!isRangeOrSpan);
    ui->startDateLabel->setVisible(isRangeOrSpan);
    ui->endDateWidget->setVisible(isRangeOrSpan);

    const bool hasMonth = ui->hasMonthCheck->isChecked();
    const bool hasDay = ui->hasDayCheck->isChecked();
    const bool hasTime = hasDay && ui->hasTimeCheck->isChecked();
    ui->daySpinBox->setEnabled(hasDay && hasMonth);
    ui->monthSpinBox->setEnabled(hasMonth);
    ui->dateEdit->setVisible(hasDay && hasMonth);
    ui->timeEdit->setVisible(hasDay);
    ui->hasTimeCheck->setEnabled(hasDay);

    const QDate proleptic{
        ui->yearSpinBox->value(),
        hasMonth ? ui->monthSpinBox->value() : 1,
        (hasDay && hasMonth) ? ui->daySpinBox->value() : 1
    };

    if (type == GenealogicalDate::RANGE || type == GenealogicalDate::SPAN) {
        const bool hasMonth2 = ui->hasMonth2Check->isChecked();
        const bool hasDay2 = ui->hasDay2Check->isChecked();
        const bool hasTime2 = hasDay2 && ui->hasTime2Check->isChecked();
        ui->daySpinBox2->setEnabled(hasDay2 && hasMonth2);
        ui->monthSpinBox2->setEnabled(hasMonth2);
        ui->dateEdit2->setVisible(hasDay2 && hasMonth2);
        ui->timeEdit2->setVisible(hasDay2);
        ui->hasTime2Check->setEnabled(hasDay2);

        const QDate proleptic2{
            ui->yearSpinBox2->value(),
            hasMonth2 ? ui->monthSpinBox2->value() : 1,
            (hasDay2 && hasMonth2) ? ui->daySpinBox2->value() : 1
        };

        if (type == GenealogicalDate::RANGE) {
            this->date = GenealogicalDate::makeRange(
                quality,
                proleptic,
                true,
                hasMonth,
                hasDay && hasMonth,
                proleptic2,
                true,
                hasMonth2,
                hasDay2 && hasMonth2
            );
        } else {
            this->date = GenealogicalDate::makeSpan(
                quality,
                proleptic,
                true,
                hasMonth,
                hasDay && hasMonth,
                proleptic2,
                true,
                hasMonth2,
                hasDay2 && hasMonth2
            );
        }
        this->date.setStartTime(hasTime ? ui->timeEdit->time() : QTime{});
        this->date.setEndTime(hasTime2 ? ui->timeEdit2->time() : QTime{});
    } else {
        const auto modifier = ui->modifierInput->currentData().value<GenealogicalDate::Modifier>();
        this->date = {modifier, quality, proleptic, true, hasMonth, hasDay && hasMonth, QStringLiteral()};
        this->date.setStartTime(hasTime ? ui->timeEdit->time() : QTime{});
    }
}

void GenealogicalDateEditorDialog::updateFormsFromDate() const {
    // Block signals while loading to prevent updateDateFromForms from firing multiple times
    const QSignalBlocker b1(ui->typeInput);
    const QSignalBlocker b2(ui->modifierInput);
    const QSignalBlocker b3(ui->qualityInput);
    const QSignalBlocker b4(ui->daySpinBox);
    const QSignalBlocker b5(ui->monthSpinBox);
    const QSignalBlocker b6(ui->yearSpinBox);
    const QSignalBlocker b7(ui->hasDayCheck);
    const QSignalBlocker b8(ui->hasMonthCheck);
    const QSignalBlocker b9(ui->daySpinBox2);
    const QSignalBlocker b10(ui->monthSpinBox2);
    const QSignalBlocker b11(ui->yearSpinBox2);
    const QSignalBlocker b12(ui->hasDay2Check);
    const QSignalBlocker b13(ui->hasMonth2Check);
    const QSignalBlocker b14(ui->dateEdit);
    const QSignalBlocker b15(ui->dateEdit2);
    const QSignalBlocker b16(ui->hasTimeCheck);
    const QSignalBlocker b17(ui->timeEdit);
    const QSignalBlocker b18(ui->hasTime2Check);
    const QSignalBlocker b19(ui->timeEdit2);

    ui->typeInput->setCurrentIndex(ui->typeInput->findData(QVariant::fromValue(date.type())));
    ui->modifierInput->setCurrentIndex(ui->modifierInput->findData(QVariant::fromValue(date.modifier())));
    ui->qualityInput->setCurrentIndex(ui->qualityInput->findData(QVariant::fromValue(date.quality())));

    const bool isRangeOrSpan = (date.type() == GenealogicalDate::RANGE || date.type() == GenealogicalDate::SPAN);
    ui->modifierInput->setEnabled(!isRangeOrSpan);
    ui->startDateLabel->setVisible(isRangeOrSpan);
    ui->endDateWidget->setVisible(isRangeOrSpan);

    const auto sp = date.startPoint();
    ui->hasMonthCheck->setChecked(date.hasMonth());
    ui->hasDayCheck->setChecked(date.hasDay());
    ui->monthSpinBox->setEnabled(date.hasMonth());
    ui->daySpinBox->setEnabled(date.hasDay() && date.hasMonth());
    ui->dateEdit->setVisible(date.hasDay() && date.hasMonth());
    ui->hasTimeCheck->setEnabled(date.hasDay());
    ui->hasTimeCheck->setChecked(sp.hasTime);
    ui->timeEdit->setVisible(date.hasDay());
    if (sp.hasTime && sp.wallTime.isValid()) {
        ui->timeEdit->setTime(sp.wallTime);
    }

    auto proleptic = date.prolepticRepresentation();
    if (auto dmy = QCalendar().partsFromDate(proleptic); dmy.isValid()) {
        ui->daySpinBox->setValue(dmy.day);
        ui->monthSpinBox->setValue(dmy.month);
        ui->yearSpinBox->setValue(dmy.year);
        if (proleptic >= ui->dateEdit->minimumDate() && proleptic <= ui->dateEdit->maximumDate()) {
            ui->dateEdit->setDate(proleptic);
        }
    } else {
        ui->daySpinBox->setValue(1);
        ui->monthSpinBox->setValue(1);
        ui->yearSpinBox->setValue(1);
    }

    const auto ep = date.endPoint();
    ui->hasMonth2Check->setChecked(ep.month);
    ui->hasDay2Check->setChecked(ep.day);
    ui->monthSpinBox2->setEnabled(ep.month);
    ui->daySpinBox2->setEnabled(ep.day && ep.month);
    ui->dateEdit2->setVisible(ep.day && ep.month);
    ui->hasTime2Check->setEnabled(ep.day);
    ui->hasTime2Check->setChecked(ep.hasTime);
    ui->timeEdit2->setVisible(ep.day);
    if (ep.hasTime && ep.wallTime.isValid()) {
        ui->timeEdit2->setTime(ep.wallTime);
    }

    auto proleptic2 = ep.proleptic;
    if (auto dmy = QCalendar().partsFromDate(proleptic2); dmy.isValid()) {
        ui->daySpinBox2->setValue(dmy.day);
        ui->monthSpinBox2->setValue(dmy.month);
        ui->yearSpinBox2->setValue(dmy.year);
        if (proleptic2 >= ui->dateEdit2->minimumDate() && proleptic2 <= ui->dateEdit2->maximumDate()) {
            ui->dateEdit2->setDate(proleptic2);
        }
    } else {
        ui->daySpinBox2->setValue(1);
        ui->monthSpinBox2->setValue(1);
        ui->yearSpinBox2->setValue(1);
    }
}

void GenealogicalDateEditorDialog::textualUpdated() {
    const auto newValue = ui->textualLineEdit->text();
    if (const auto parsedDate = GenealogicalDate::fromDisplayText(newValue); parsedDate.isValid()) {
        date = parsedDate;
        // Sync forms without re-triggering textualUpdated
        updateFormsFromDate();
        ui->textualLineEdit->setStyleSheet(defaultLineEditStyle);
    } else {
        ui->textualLineEdit->setStyleSheet(defaultLineEditStyle + u"; border: 1px solid red;"_s);
    }
}
