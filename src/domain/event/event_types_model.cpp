/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_types_model.h"

#include "../../core/data_event_broker.h"
#include "database/schema.h"
#include "event_repository.h"

#include <KLocalizedString>

EventTypesListModel::EventTypesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &EventTypeEntity::id);
    this->setColumn(TYPE, i18n("Type"), &EventTypeEntity::type);
    this->setColumn(BUILTIN, i18n("Built-in"), &EventTypeEntity::builtin);

    connectToTable<Schema::EventTypes>(this);

    reload();
}

void EventTypesListModel::reload() {
    EventRepository repo;
    this->setItems(repo.findAllEventTypes());
}
