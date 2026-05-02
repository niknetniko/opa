/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_types_list_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"
#include "source_repository.h"

#include <KLocalizedString>

SourceTypesListModel::SourceTypesListModel(QObject* parent) : ObjectTableModel(parent) {
    this->setColumn(ID, i18n("ID"), &SourceTypeEntity::id);
    this->setColumn(TYPE, i18n("Type"), &SourceTypeEntity::type, [](SourceTypeEntity& entity, const QVariant& value) {
        const QString newType = value.toString();
        if (newType.isEmpty()) {
            return false;
        }
        SourceRepository repo;
        return repo.updateSourceType(entity.id, newType);
    });
    this->setColumn(BUILTIN, i18n("Built-in"), &SourceTypeEntity::builtin);

    connectToTable<Schema::SourceTypes>(this);

    reload();
}

void SourceTypesListModel::reload() {
    SourceRepository repo;
    this->setItems(repo.findAllSourceTypes());
}
