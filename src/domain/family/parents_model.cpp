/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "parents_model.h"

#include "../name/names.h"
#include "core/data_event_broker.h"
#include "family_repository.h"

#include <KLocalizedString>

ParentsModel::ParentsModel(IntegerPrimaryKey personId, QObject* parent) : ObjectTableModel(parent), personId(personId) {

    this->setColumn(ROLE, i18n("Role"), &ParentEntity::role);
    this->setColumn(PERSON_ID, i18n("Person ID"), &ParentEntity::personId);
    this->setColumn(DISPLAY_NAME, i18n("Name"), [](const ParentEntity& e) {
        return construct_display_name(e.titles, e.givenNames, e.prefix, e.surname);
    });

    connectToTable<Schema::People>(this);
    connectToTable<Schema::Names>(this);
    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventRoles>(this);
    connectToTable<Schema::EventRelations>(this);

    reload();
}

void ParentsModel::reload() {
    FamilyRepository repo;
    this->setItems(repo.findParentsForPerson(personId));
}
