/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_editor_dialog.h"

#include "data/data_manager.h"
#include "data/event.h"
#include "dates/genealogical_date.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "note_editor_dialog.h"
#include "ui_event_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/model_utils_find_source_model_of_type.h"
#include "utils/proxy_enabled_relational_delegate.h"

#include <QDataWidgetMapper>
#include <QSqlError>

EventEditorDialog::EventEditorDialog(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, bool newEvent, QWidget* parent
) :
    AbstractEditorDialog(parent),
    form(new Ui::EventEditorForm),
    newEvent(newEvent) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->eventDateEditButton, &QPushButton::clicked, this, &EventEditorDialog::editDateWithEditor);
    connect(form->noteEditButton, &QPushButton::clicked, this, &EventEditorDialog::editNoteWithEditor);
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connectComboBox(eventRelationModel, EventRelationsModel::ROLE, form->eventRoleComboBox);
    connectComboBox(eventModel, EventsModel::TYPE, form->eventTypeComboBox);

    if (newEvent) {
        this->setWindowTitle(i18n("Nieuwe gebeurtenis toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::EVENT, eventModel->index(0, 0).data());
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }


    auto* eventRelationMapper = new QDataWidgetMapper(this);
    eventRelationMapper->setModel(eventRelationModel);
    eventRelationMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    eventRelationMapper->addMapping(form->eventRoleComboBox, EventRelationsModel::ROLE);
    eventRelationMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventRelationMapper->toFirst();
    this->mappers.append(eventRelationMapper);

    form->noteEdit->enableRichTextMode();

    auto* eventMapper = new QDataWidgetMapper(this);
    eventMapper->setModel(eventModel);
    eventMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    eventMapper->addMapping(form->eventDatePicker, EventsModel::DATE);
    eventMapper->addMapping(form->eventNameEdit, EventsModel::NAME);
    eventMapper->addMapping(form->eventTypeComboBox, EventsModel::TYPE);
    eventMapper->addMapping(form->noteEdit, EventsModel::NOTE);
    eventMapper->setItemDelegate(new SuperSqlRelationalDelegate(this));
    eventMapper->toFirst();
    this->mappers.append(eventMapper);
}

void EventEditorDialog::showDialogForNewEvent(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventRelationModel, eventModel, true, parent);
    dialog->show();
}

void EventEditorDialog::showDialogForExistingEvent(
    QAbstractItemModel* eventRelationModel, QAbstractItemModel* eventModel, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventRelationModel, eventModel, false, parent);
    dialog->show();
}

void EventEditorDialog::revert() {
    if (!newEvent) {
        return;
    }
    auto* eventRelationsModel = mappers[EVENT_RELATION_MAPPER]->model();
    if (!eventRelationsModel->removeRow(eventRelationsModel->rowCount() - 1)) {
        qWarning() << "Could not revert event relation model?";
    }
    auto* eventsModel = mappers[EVENT_MAPPER]->model();
    if (!eventsModel->removeRow(eventsModel->rowCount() - 1)) {
        qWarning() << "Could not revert event model?";
    }
}

void EventEditorDialog::editDateWithEditor() {
    auto currentText = form->eventDatePicker->text();
    auto startDate = GenealogicalDate::fromDisplayText(currentText);
    if (auto date = GenealogicalDateEditorDialog::editDate(startDate, this); date.isValid()) {
        form->eventDatePicker->setText(date.toDisplayText());
    }
}

void EventEditorDialog::editNoteWithEditor() {
    auto currentText = form->noteEdit->textOrHtml();
    if (auto note = NoteEditorDialog::editText(currentText, i18n("Edit note"), this); !note.isEmpty()) {
        form->noteEdit->setTextOrHtml(note);
    }
}
