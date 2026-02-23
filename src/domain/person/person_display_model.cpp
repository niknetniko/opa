/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_display_model.h"

#include "core/data_event_broker.h"
#include "data/names.h"
#include "database/schema.h"
#include "person_repository.h"

#include <KLocalizedString>

PersonDisplayModel::PersonDisplayModel(QObject* parent) : ObjectTableModel(parent) {

    this->setColumn(ID, i18n("ID"), &PersonDisplayEntity::id);
    this->setColumn(NAME, i18n("Name"), [](const PersonDisplayEntity& p) {
        return construct_display_name(p.titles, p.givenNames, p.prefix, p.surname);
    });
    this->setColumn(ROOT, i18n("Root"), &PersonDisplayEntity::root);

    connectToTable<Schema::People>(this);
    connectToTable<Schema::Names>(this);

    reload();
}

void PersonDisplayModel::reload() {
    PersonRepository repo;
    this->setItems(repo.findPeopleWithPrimaryName());
}
