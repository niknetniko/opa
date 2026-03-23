/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "parent_event_roles_list_model.h"

#include "event_repository.h"
#include "event_roles.h"
#include "event_types.h"

#include <KLocalizedString>

ParentEventRolesListModel::ParentEventRolesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &EventRoleEntity::id);
    this->setColumn(ROLE, i18n("Role"), &EventRoleEntity::role);
    this->setColumn(BUILTIN, i18n("Built-in"), &EventRoleEntity::builtin);

    reload();
}

void ParentEventRolesListModel::reload() {
    // Build the set of parent role untranslated names.
    QSet<QString> parentRoleNames;
    for (const auto& role: EventRoles::parentRoles()) {
        const auto& name = EventRoles::nameOriginToString.value(role);
        if (name.untranslatedText()) {
            parentRoleNames.insert(QString::fromUtf8(name.untranslatedText()));
        }
    }

    EventRepository repo;
    const auto allRoles = repo.findAllEventRoles();
    QList<EventRoleEntity> filtered;
    for (const auto& entity: allRoles) {
        if (parentRoleNames.contains(entity.role)) {
            filtered.append(entity);
        }
    }
    this->setItems(filtered);
}

RelationshipEventTypesListModel::RelationshipEventTypesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &EventTypeEntity::id);
    this->setColumn(TYPE, i18n("Type"), &EventTypeEntity::type);
    this->setColumn(BUILTIN, i18n("Built-in"), &EventTypeEntity::builtin);

    reload();
}

void RelationshipEventTypesListModel::reload() {
    // Build the set of relationship event type untranslated names.
    QSet<QString> relationshipTypeNames;
    for (const auto& eventType: EventTypes::relationshipStartingEvents()) {
        const auto& name = EventTypes::typeToString.value(eventType);
        if (name.untranslatedText()) {
            relationshipTypeNames.insert(QString::fromUtf8(name.untranslatedText()));
        }
    }

    EventRepository repo;
    const auto allTypes = repo.findAllEventTypes();
    QList<EventTypeEntity> filtered;
    for (const auto& entity: allTypes) {
        if (relationshipTypeNames.contains(entity.type)) {
            filtered.append(entity);
        }
    }
    this->setItems(filtered);
}
