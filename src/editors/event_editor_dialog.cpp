/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_editor_dialog.h"

#include "dates/genealogical_date.h"
#include "domain/event/event_roles_model.h"
#include "domain/event/event_types_model.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "domain/event/event_repository.h"
#include "note_editor_dialog.h"
#include "ui_event_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KLocalizedString>
#include <QComboBox>

EventEditorDialog::EventEditorDialog(
    IntegerPrimaryKey eventId,
    IntegerPrimaryKey roleId,
    IntegerPrimaryKey personId,
    bool newEvent,
    QWidget* parent
) :
    QDialog(parent),
    form(new Ui::EventEditorForm),
    newEvent(newEvent),
    eventId(eventId),
    roleId(roleId),
    personId(personId),
    originalRoleId(roleId) {
    form->setupUi(this);

    // Connect the buttons.
    connect(form->eventDateEditButton, &QPushButton::clicked, this, &EventEditorDialog::editDateWithEditor);
    connect(form->noteEditButton, &QPushButton::clicked, this, &EventEditorDialog::editNoteWithEditor);

    // Populate the event type combo box from the domain types model.
    typesModel = new EventTypesListModel(this);
    form->eventTypeComboBox->setModel(typesModel);
    form->eventTypeComboBox->setModelColumn(EventTypesListModel::TYPE);

    // Populate the role combo box from the domain roles model.
    rolesModel = new EventRolesListModel(this);
    form->eventRoleComboBox->setModel(rolesModel);
    form->eventRoleComboBox->setModelColumn(EventRolesListModel::ROLE);

    form->noteEdit->enableRichTextMode();

    // Load the event data and pre-fill the form.
    EventRepository repo;
    if (const auto event = repo.findEventById(eventId)) {
        // Pre-select the event type.
        auto typeIndex = typesModel->match(
            typesModel->index(0, EventTypesListModel::ID), Qt::DisplayRole, event->typeId
        );
        if (!typeIndex.isEmpty()) {
            form->eventTypeComboBox->setCurrentIndex(typeIndex.constFirst().row());
        }

        // Pre-fill date, name, note.
        if (!event->date.isEmpty()) {
            form->eventDatePicker->setText(GenealogicalDate::fromDatabaseRepresentation(event->date).toDisplayText());
        }
        form->eventNameEdit->setText(event->name);
        form->noteEdit->setTextOrHtml(event->note);
    }

    // Pre-select the role.
    auto roleIndex = rolesModel->match(rolesModel->index(0, EventRolesListModel::ID), Qt::DisplayRole, roleId);
    if (!roleIndex.isEmpty()) {
        form->eventRoleComboBox->setCurrentIndex(roleIndex.constFirst().row());
    }

    if (newEvent) {
        this->setWindowTitle(i18n("Nieuwe gebeurtenis toevoegen"));
    } else {
        auto nameId = format_id(FormattedIdentifierDelegate::EVENT, eventId);
        this->setWindowTitle(i18n("%1 bewerken", nameId));
    }
}

void EventEditorDialog::accept() {
    EventRepository repo;

    // Collect values from widgets.
    auto typeRow = form->eventTypeComboBox->currentIndex();
    auto typeId = typesModel->index(typeRow, EventTypesListModel::ID).data().toLongLong();

    auto displayDate = form->eventDatePicker->text();
    auto date = displayDate.isEmpty()
        ? QString{}
        : GenealogicalDate::fromDisplayText(displayDate).toDatabaseRepresentation();
    auto name = form->eventNameEdit->text();
    auto note = form->noteEdit->textOrHtml();

    if (!repo.updateEvent(eventId, typeId, date, name, note)) {
        qWarning() << "Could not save event" << eventId;
        return;
    }

    // Update the role if it changed (delete old relation, insert new one).
    auto roleRow = form->eventRoleComboBox->currentIndex();
    auto newRoleId = rolesModel->index(roleRow, EventRolesListModel::ID).data().toLongLong();

    if (newRoleId != originalRoleId) {
        repo.deleteEventRelation(eventId, personId, originalRoleId);
        repo.insertEventRelation(eventId, personId, newRoleId);
        roleId = newRoleId;
        originalRoleId = newRoleId;
    }

    QDialog::accept();
}

void EventEditorDialog::reject() {
    if (newEvent) {
        // Delete the newly-created event (cascade removes the relation too if FK is set,
        // but we delete the relation explicitly to be safe).
        EventRepository repo;
        repo.deleteEventRelation(eventId, personId, roleId);
        repo.deleteEvent(eventId);
    }

    QDialog::reject();
}

void EventEditorDialog::showDialogForNewEvent(
    IntegerPrimaryKey eventId, IntegerPrimaryKey roleId, IntegerPrimaryKey personId, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventId, roleId, personId, true, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EventEditorDialog::showDialogForExistingEvent(
    IntegerPrimaryKey eventId, IntegerPrimaryKey roleId, IntegerPrimaryKey personId, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventId, roleId, personId, false, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
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
