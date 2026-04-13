/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_events_model.h"

#include "../../core/data_event_broker.h"
#include "dates/genealogical_date.h"
#include "event_repository.h"

#include <KLocalizedString>

PersonEventListModel::PersonEventListModel(IntegerPrimaryKey personId, QObject* parent) :
    ObjectTableModel(parent),
    personId(personId) {

    this->setColumn(ROLE, i18n("Role"), &PersonEventEntity::role);
    this->setColumn(TYPE, i18n("Type"), &PersonEventEntity::type);
    this->setColumn(DATE, i18n("Date"), [](const PersonEventEntity& e) -> QVariant {
        if (e.date.isEmpty()) {
            return {};
        }
        return GenealogicalDate::fromDatabaseRepresentation(e.date).toDisplayText();
    });
    this->setColumn(NAME, i18n("Name"), &PersonEventEntity::name);
    this->setColumn(ID, i18n("ID"), &PersonEventEntity::id);
    this->setColumn(ROLE_ID, i18n("Role ID"), &PersonEventEntity::roleId);
    this->setColumn(RELATION_ID, i18n("Relation ID"), &PersonEventEntity::relationId);

    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventTypes>(this);
    connectToTable<Schema::EventRoles>(this);
    connectToTable<Schema::EventRelations>(this);

    reload();
}

void PersonEventListModel::reload() {
    EventRepository repo;
    this->setItems(repo.findEventsForPerson(personId));
}
