/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_types_list_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationTypesListModel::LocationTypesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationTypeEntity::id);
    this->setColumn(
        TYPE,
        i18n("Type"),
        &LocationTypeEntity::type,
        [](LocationTypeEntity& entity, const QVariant& value) {
            QString newType = value.toString();
            if (newType.isEmpty()) {
                return false;
            }
            LocationRepository repo;
            if (!repo.updateLocationType(entity.id, newType)) {
                return false;
            }
            return true;
        }
    );
    this->setColumn(BUILTIN, i18n("Built-in"), &LocationTypeEntity::builtin);

    connectToTable<Schema::LocationTypes>(this);

    reload();
}

void LocationTypesListModel::reload() {
    LocationRepository repo;
    this->setItems(repo.findAllLocationTypes());
}
