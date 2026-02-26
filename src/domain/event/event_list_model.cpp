/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_list_model.h"

#include "../../core/data_event_broker.h"
#include "database/schema.h"
#include "event_repository.h"

#include <KLocalizedString>

EventListModel::EventListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &EventDisplayEntity::id);
    this->setColumn(TYPE_ID, i18n("Type ID"), &EventDisplayEntity::typeId);
    this->setColumn(TYPE, i18n("Type"), &EventDisplayEntity::type);
    this->setColumn(DATE, i18n("Date"), &EventDisplayEntity::date);
    this->setColumn(NAME, i18n("Name"), &EventDisplayEntity::name);

    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventTypes>(this);

    reload();
}

void EventListModel::reload() {
    EventRepository repo;
    this->setItems(repo.findAllEvents());
}
