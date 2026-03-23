/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_editor_dialog.h"

#include "database/database.h"
#include "dates/genealogical_date.h"
#include "dates/genealogical_date_editor_dialog.h"
#include "domain/event/event_repository.h"
#include "domain/event/event_roles_model.h"
#include "domain/event/event_types_model.h"
#include "domain/source/source_repository.h"
#include "note_editor_dialog.h"
#include "ui/source/citation_list_widget.h"
#include "ui_event_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"

#include <KCollapsibleGroupBox>
#include <KLocalizedString>
#include <QComboBox>

using namespace Qt::StringLiterals;

EventEditorDialog::EventEditorDialog(IntegerPrimaryKey personId, QWidget* parent) :
    QDialog(parent),
    form(new Ui::EventEditorForm),
    personId(personId),
    eventCitationsPending(
        {},
        [](const SourceEntity& e) { return e.id; },
        [](IntegerPrimaryKey id) { return SourceRepository().findById(id); }
    ),
    relationCitationsPending(
        {},
        [](const SourceEntity& e) { return e.id; },
        [](IntegerPrimaryKey id) { return SourceRepository().findById(id); }
    ) {
    setupUi();
    setWindowTitle(i18n("Nieuwe gebeurtenis toevoegen"));
}

EventEditorDialog::EventEditorDialog(
    IntegerPrimaryKey eventId,
    IntegerPrimaryKey roleId,
    IntegerPrimaryKey relationId,
    IntegerPrimaryKey personId,
    QWidget* parent
) :
    QDialog(parent),
    form(new Ui::EventEditorForm),
    eventId(eventId),
    personId(personId),
    roleId(roleId),
    relationId(relationId),
    eventCitationsPending(
        EventRepository().findCitationsForEvent(eventId),
        [](const SourceEntity& e) { return e.id; },
        [](IntegerPrimaryKey id) { return SourceRepository().findById(id); }
    ),
    relationCitationsPending(
        EventRepository().findCitationsForEventRelation(relationId),
        [](const SourceEntity& e) { return e.id; },
        [](IntegerPrimaryKey id) { return SourceRepository().findById(id); }
    ) {
    setupUi();

    // Load the event data and pre-fill the form.
    EventRepository repo;
    if (const auto event = repo.findEventById(eventId)) {
        auto typeIndex = typesModel->match(
            typesModel->index(0, EventTypesListModel::ID), Qt::DisplayRole, event->typeId
        );
        if (!typeIndex.isEmpty()) {
            form->eventTypeComboBox->setCurrentIndex(typeIndex.constFirst().row());
        }

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

    auto nameId = format_id(FormattedIdentifierDelegate::EVENT, eventId);
    setWindowTitle(i18n("%1 bewerken", nameId));
}

void EventEditorDialog::setupUi() {
    form->setupUi(this);

    connect(form->eventDateEditButton, &QPushButton::clicked, this, &EventEditorDialog::editDateWithEditor);
    connect(form->noteEditButton, &QPushButton::clicked, this, &EventEditorDialog::editNoteWithEditor);

    typesModel = new EventTypesListModel(this);
    form->eventTypeComboBox->setModel(typesModel);
    form->eventTypeComboBox->setModelColumn(EventTypesListModel::TYPE);

    rolesModel = new EventRolesListModel(this);
    form->eventRoleComboBox->setModel(rolesModel);
    form->eventRoleComboBox->setModelColumn(EventRolesListModel::ROLE);

    form->noteEdit->enableRichTextMode();

    // Event relation citations (inserted after the first group box).
    auto* relationCitationsGroup = new KCollapsibleGroupBox(this);
    relationCitationsGroup->setTitle(i18n("Event relation sources"));
    auto* relationCitationsLayout = new QVBoxLayout(relationCitationsGroup);
    eventRelationCitationsWidget = new CitationListWidget(
        [this] { return relationCitationsPending.items(); },
        [this](IntegerPrimaryKey sourceId) { return relationCitationsPending.add(sourceId); },
        [this](IntegerPrimaryKey sourceId) { return relationCitationsPending.remove(sourceId); },
        relationCitationsGroup
    );
    relationCitationsLayout->addWidget(eventRelationCitationsWidget);
    form->verticalLayout->insertWidget(1, relationCitationsGroup);

    // Event citations (inserted after the event information group box).
    auto* eventCitationsGroup = new KCollapsibleGroupBox(this);
    eventCitationsGroup->setTitle(i18n("Event sources"));
    auto* eventCitationsLayout = new QVBoxLayout(eventCitationsGroup);
    eventCitationsWidget = new CitationListWidget(
        [this] { return eventCitationsPending.items(); },
        [this](IntegerPrimaryKey sourceId) { return eventCitationsPending.add(sourceId); },
        [this](IntegerPrimaryKey sourceId) { return eventCitationsPending.remove(sourceId); },
        eventCitationsGroup
    );
    eventCitationsLayout->addWidget(eventCitationsWidget);
    form->verticalLayout->insertWidget(3, eventCitationsGroup);
}

void EventEditorDialog::accept() {
    // Collect values from widgets before the transaction.
    auto typeRow = form->eventTypeComboBox->currentIndex();
    auto typeId = typesModel->index(typeRow, EventTypesListModel::ID).data().toLongLong();

    auto displayDate = form->eventDatePicker->text();
    auto date = displayDate.isEmpty()
        ? QString{}
        : GenealogicalDate::fromDisplayText(displayDate).toDatabaseRepresentation();
    auto name = form->eventNameEdit->text();
    auto note = form->noteEdit->textOrHtml();

    auto roleRow = form->eventRoleComboBox->currentIndex();
    auto newRoleId = rolesModel->index(roleRow, EventRolesListModel::ID).data().toLongLong();

    auto result = executeInTransaction([&]() -> std::optional<bool> {
        EventRepository repo;

        if (!eventId.has_value()) {
            // New event: create everything.
            auto newEventId = repo.insertFullEvent(typeId, date, name, note, personId, newRoleId);
            if (!newEventId) {
                return std::nullopt;
            }

            auto createdEventId = *newEventId;
            eventId = createdEventId;
            roleId = newRoleId;

            // Look up the relation id for the just-created relation.
            auto relations = repo.findRelationsForEvent(createdEventId);
            if (relations.isEmpty()) {
                return std::nullopt;
            }
            relationId = relations.first().id;

            // Commit pending citations using the newly created relationId.
            eventCitationsPending.commit(
                [&](IntegerPrimaryKey sourceId) { return repo.addEventCitation(createdEventId, sourceId); },
                [&](IntegerPrimaryKey sourceId) { return repo.removeEventCitation(createdEventId, sourceId); }
            );
            relationCitationsPending.commit(
                [&](IntegerPrimaryKey sourceId) {
                    return repo.addEventRelationCitation(*relationId, sourceId);
                },
                [&](IntegerPrimaryKey sourceId) {
                    return repo.removeEventRelationCitation(*relationId, sourceId);
                }
            );
        } else {
            // Existing event: update fields.
            if (!repo.updateEvent(*eventId, typeId, date, name, note)) {
                return std::nullopt;
            }

            // Update the role if it changed — simple UPDATE on the relation row.
            if (newRoleId != roleId) {
                if (!repo.updateEventRelationRole(*relationId, newRoleId)) {
                    return std::nullopt;
                }
                roleId = newRoleId;
            }

            // Commit pending citation changes.
            eventCitationsPending.commit(
                [&](IntegerPrimaryKey sourceId) { return repo.addEventCitation(*eventId, sourceId); },
                [&](IntegerPrimaryKey sourceId) { return repo.removeEventCitation(*eventId, sourceId); }
            );
            relationCitationsPending.commit(
                [&](IntegerPrimaryKey sourceId) {
                    return repo.addEventRelationCitation(*relationId, sourceId);
                },
                [&](IntegerPrimaryKey sourceId) {
                    return repo.removeEventRelationCitation(*relationId, sourceId);
                }
            );
        }

        return true;
    });

    if (result) {
        QDialog::accept();
    }
}

void EventEditorDialog::reject() {
    QDialog::reject();
}

void EventEditorDialog::showDialogForNewEvent(IntegerPrimaryKey personId, QWidget* parent) {
    auto* dialog = new EventEditorDialog(personId, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void EventEditorDialog::showDialogForExistingEvent(
    IntegerPrimaryKey eventId, IntegerPrimaryKey roleId, IntegerPrimaryKey relationId, IntegerPrimaryKey personId, QWidget* parent
) {
    auto* dialog = new EventEditorDialog(eventId, roleId, relationId, personId, parent);
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
