/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_paths_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"
#include "location_repository.h"

#include <KLocalizedString>

LocationPathsModel::LocationPathsModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &LocationDisplayEntity::id);
    this->setColumn(FULL_PATH, i18n("Location"), &LocationDisplayEntity::fullPath);

    connectToTable<Schema::Locations>(this);

    reload();
}

void LocationPathsModel::reload() {
    LocationRepository repo;
    this->setItems(repo.findAllWithPaths());
}
