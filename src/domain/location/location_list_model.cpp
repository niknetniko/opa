/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_list_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationListModel::LocationListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationEntity::id);
    this->setColumn(NAME, i18n("Name"), &LocationEntity::name);
    this->setColumn(TYPE_ID, i18n("Type"), [](const LocationEntity& e) -> QVariant {
        return e.typeId.has_value() ? QVariant(*e.typeId) : QVariant{};
    });
    this->setColumn(PARENT_ID, i18n("Parent"), [](const LocationEntity& e) -> QVariant {
        return e.parentId.has_value() ? QVariant(*e.parentId) : QVariant{};
    });

    connectToTable<Schema::Locations>(this);

    reload();
}

void LocationListModel::reload() {
    LocationRepository repo;
    this->setItems(repo.findAll());
}
