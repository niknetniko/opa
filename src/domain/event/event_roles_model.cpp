/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_roles_model.h"

#include "../../core/data_event_broker.h"
#include "database/schema.h"
#include "event_repository.h"

#include <KLocalizedString>

EventRolesListModel::EventRolesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &EventRoleEntity::id);
    this->setColumn(ROLE, i18n("Role"), &EventRoleEntity::role, [](EventRoleEntity& entity, const QVariant& value) {
        QString newRole = value.toString();
        if (newRole.isEmpty()) {
            return false;
        }
        EventRepository repo;
        return repo.updateEventRole(entity.id, newRole);
    });
    this->setColumn(BUILTIN, i18n("Built-in"), &EventRoleEntity::builtin);

    connectToTable<Schema::EventRoles>(this);

    reload();
}

void EventRolesListModel::reload() {
    EventRepository repo;
    this->setItems(repo.findAllEventRoles());
}
