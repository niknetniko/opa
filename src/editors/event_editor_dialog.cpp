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
#include "domain/event/event_role_translation_repository.h"
#include "domain/event/event_roles.h"
#include "domain/event/event_roles_model.h"
#include "domain/event/event_type_translation_repository.h"
#include "domain/event/event_types.h"
#include "domain/event/event_types_model.h"
#include "domain/location/location_paths_model.h"
#include "domain/location/location_repository.h"
#include "domain/media/media_repository.h"
#include "domain/source/source_repository.h"
#include "editors/location_editor_dialog.h"
#include "note_editor_dialog.h"
#include "ui/media/media_list_widget.h"
#include "ui/source/citation_list_widget.h"
#include "ui_event_editor_dialog.h"
#include "utils/formatted_identifier_delegate.h"
#include "utils/translating_proxy_model.h"

#include <KLocalizedString>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

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
    if (const auto event = repo.findEventById(eventId); event) {
        auto typeIndex =
            typesModel->match(typesModel->index(0, EventTypesListModel::ID), Qt::DisplayRole, event->typeId);
        if (!typeIndex.isEmpty()) {
            form->eventTypeComboBox->setCurrentIndex(typeIndex.constFirst().row());
        }

        if (!event->date.isEmpty()) {
            form->eventDatePicker->setText(GenealogicalDate::fromDatabaseRepresentation(event->date).toDisplayText());
        }
        form->eventNameEdit->setText(event->name);
        form->noteEdit->setTextOrHtml(event->note);

        if (event->locationId.has_value()) {
            for (int i = 0; i < locationsModel->rowCount(); ++i) {
                const auto idIdx = locationsModel->index(i, LocationPathsModel::ID);
                if (locationsModel->data(idIdx).toLongLong() == *event->locationId) {
                    form->eventLocationComboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    } else {
        qWarning() << "Event" << eventId << "not found in database — form will open with empty fields";
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
    auto* typesProxy = new TranslatingProxyModel(
        TypeTranslationResolver(
            [](IntegerPrimaryKey typeId, const QString& locale) {
                return EventTypeTranslationRepository().findByTypeIdAndLocale(typeId, locale);
            },
            EventTypes::toDisplayString
        ),
        this
    );
    typesProxy->setSourceModel(typesModel);
    form->eventTypeComboBox->setModel(typesProxy);
    form->eventTypeComboBox->setModelColumn(EventTypesListModel::TYPE);

    rolesModel = new EventRolesListModel(this);
    auto* rolesProxy = new TranslatingProxyModel(
        TypeTranslationResolver(
            [](IntegerPrimaryKey roleId, const QString& locale) {
                return EventRoleTranslationRepository().findByTypeIdAndLocale(roleId, locale);
            },
            EventRoles::toDisplayString
        ),
        this
    );
    rolesProxy->setSourceModel(rolesModel);
    form->eventRoleComboBox->setModel(rolesProxy);
    form->eventRoleComboBox->setModelColumn(EventRolesListModel::ROLE);

    locationsModel = new LocationPathsModel(this);
    form->eventLocationComboBox->setModel(locationsModel);
    form->eventLocationComboBox->setModelColumn(LocationPathsModel::FULL_PATH);
    form->eventLocationComboBox->setPlaceholderText(i18n("No location"));
    form->eventLocationComboBox->setCurrentIndex(-1);

    connect(form->eventLocationNewButton, &QPushButton::clicked, this, [this]() {
        auto newId = LocationEditorDialog::showDialogForNewLocation(std::nullopt, this);
        if (newId.isValid()) {
            // Find the new location in the model and select it
            for (int i = 0; i < locationsModel->rowCount(); ++i) {
                const auto idIdx = locationsModel->index(i, LocationPathsModel::ID);
                if (locationsModel->data(idIdx).toLongLong() == newId.toLongLong()) {
                    form->eventLocationComboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    });

    form->noteEdit->enableRichTextMode();

    // Sources tab — always present.
    auto* sourcesTab = new QWidget;
    auto* sourcesLayout = new QVBoxLayout(sourcesTab);

    auto* relationLabel = new QLabel(i18n("Participation sources"), sourcesTab);
    sourcesLayout->addWidget(relationLabel);
    eventRelationCitationsWidget = new CitationListWidget(
        [this] { return relationCitationsPending.items(); },
        [this](IntegerPrimaryKey sourceId) { return relationCitationsPending.add(sourceId); },
        [this](IntegerPrimaryKey sourceId) { return relationCitationsPending.remove(sourceId); },
        sourcesTab
    );
    sourcesLayout->addWidget(eventRelationCitationsWidget);

    auto* eventCitationsLabel = new QLabel(i18n("Event sources"), sourcesTab);
    sourcesLayout->addWidget(eventCitationsLabel);
    eventCitationsWidget = new CitationListWidget(
        [this] { return eventCitationsPending.items(); },
        [this](IntegerPrimaryKey sourceId) { return eventCitationsPending.add(sourceId); },
        [this](IntegerPrimaryKey sourceId) { return eventCitationsPending.remove(sourceId); },
        sourcesTab
    );
    sourcesLayout->addWidget(eventCitationsWidget);
    sourcesLayout->addStretch();
    form->tabWidget->addTab(sourcesTab, i18n("Sources"));

    // Media tab — only when editing an existing event.
    if (eventId.has_value() || relationId.has_value()) {
        auto* mediaTab = new QWidget;
        auto* mediaLayout = new QVBoxLayout(mediaTab);

        if (eventId.has_value()) {
            const auto capturedEventId = *eventId;
            auto* eventMediaLabel = new QLabel(i18n("Event media"), mediaTab);
            mediaLayout->addWidget(eventMediaLabel);
            auto* eventMediaWidget = new MediaListWidget(
                [capturedEventId] { return MediaRepository().findForEvent(capturedEventId); },
                [capturedEventId](IntegerPrimaryKey mediaId) {
                    return MediaRepository().attachToEvent(capturedEventId, mediaId);
                },
                [capturedEventId](IntegerPrimaryKey mediaId) {
                    return MediaRepository().detachFromEvent(capturedEventId, mediaId);
                },
                mediaTab
            );
            mediaLayout->addWidget(eventMediaWidget);
        }

        if (relationId.has_value()) {
            const auto capturedRelationId = *relationId;
            auto* relationMediaLabel = new QLabel(i18n("Participation media"), mediaTab);
            mediaLayout->addWidget(relationMediaLabel);
            auto* relationMediaWidget = new MediaListWidget(
                [capturedRelationId] { return MediaRepository().findForEventRelation(capturedRelationId); },
                [capturedRelationId](IntegerPrimaryKey mediaId) {
                    return MediaRepository().attachToEventRelation(capturedRelationId, mediaId);
                },
                [capturedRelationId](IntegerPrimaryKey mediaId) {
                    return MediaRepository().detachFromEventRelation(capturedRelationId, mediaId);
                },
                mediaTab
            );
            mediaLayout->addWidget(relationMediaWidget);
        }

        mediaLayout->addStretch();
        form->tabWidget->addTab(mediaTab, i18n("Media"));
    }
}

void EventEditorDialog::accept() {
    // Collect values from widgets before the transaction.
    auto typeRow = form->eventTypeComboBox->currentIndex();
    auto typeId = typesModel->index(typeRow, EventTypesListModel::ID).data().toLongLong();

    auto displayDate = form->eventDatePicker->text();
    auto date = displayDate.isEmpty() ? QString{}
                                      : GenealogicalDate::fromDisplayText(displayDate).toDatabaseRepresentation();
    auto name = form->eventNameEdit->text();
    auto note = form->noteEdit->textOrHtml();

    auto roleRow = form->eventRoleComboBox->currentIndex();
    auto newRoleId = rolesModel->index(roleRow, EventRolesListModel::ID).data().toLongLong();

    auto locationRow = form->eventLocationComboBox->currentIndex();
    auto selectedLocationId = locationRow >= 0
                                  ? std::optional<IntegerPrimaryKey>(
                                        locationsModel->index(locationRow, LocationPathsModel::ID).data().toLongLong()
                                    )
                                  : std::nullopt;

    auto result = executeInTransaction([&]() -> std::optional<bool> {
        EventRepository repo;

        if (!eventId.has_value()) {
            // New event: create everything.
            // TODO: allow assigning the new event to a family (family_id) so that marriage and
            // birth events created here participate in explicit family grouping.
            // Issue URL: https://github.com/niknetniko/opa/issues/62
            auto newEventId = repo.insertFullEvent(typeId, date, name, note, personId, newRoleId, selectedLocationId);
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
                [&](IntegerPrimaryKey sourceId) { return repo.addEventRelationCitation(*relationId, sourceId); },
                [&](IntegerPrimaryKey sourceId) { return repo.removeEventRelationCitation(*relationId, sourceId); }
            );
        } else {
            // Existing event: update fields.
            if (!repo.updateEvent(*eventId, typeId, date, name, note, selectedLocationId)) {
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
                [&](IntegerPrimaryKey sourceId) { return repo.addEventRelationCitation(*relationId, sourceId); },
                [&](IntegerPrimaryKey sourceId) { return repo.removeEventRelationCitation(*relationId, sourceId); }
            );
        }

        return true;
    });

    if (result) {
        QDialog::accept();
    } else {
        qWarning() << "Failed to save event for person" << personId;
        QMessageBox::critical(this, i18n("Save failed"), i18n("Could not save the event. Please try again."));
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
    IntegerPrimaryKey eventId,
    IntegerPrimaryKey roleId,
    IntegerPrimaryKey relationId,
    IntegerPrimaryKey personId,
    QWidget* parent
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
