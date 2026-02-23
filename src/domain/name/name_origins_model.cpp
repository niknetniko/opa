/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "name_origins_model.h"

#include "../../core/data_event_broker.h"
#include "database/schema.h"
#include "name_repository.h"

#include <KLocalizedString>

NameOriginsModel::NameOriginsModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &NameOriginEntity::id);
    this->setColumn(ORIGIN, i18n("Origin"), &NameOriginEntity::origin);
    this->setColumn(BUILTIN, i18n("Built-in"), &NameOriginEntity::builtin);

    connectToTable<Schema::NameOrigins>(this);

    reload();
}

void NameOriginsModel::reload() {
    NameRepository repo;
    this->setItems(repo.findAllOrigins());
}
