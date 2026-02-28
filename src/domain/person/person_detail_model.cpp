/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "person_detail_model.h"

#include "../name/names.h"
#include "core/data_event_broker.h"
#include "database/schema.h"
#include "person_repository.h"

#include <KLocalizedString>

PersonDetailModel::PersonDetailModel(IntegerPrimaryKey personId, QObject* parent) :
    ObjectTableModel(parent),
    personId(personId) {
    this->setColumn(ID, i18n("ID"), &PersonDisplayEntity::id);
    this->setColumn(TITLES, i18n("Titles"), &PersonDisplayEntity::titles);
    this->setColumn(GIVEN_NAMES, i18n("Given names"), &PersonDisplayEntity::givenNames);
    this->setColumn(PREFIXES, i18n("Prefix"), &PersonDisplayEntity::prefix);
    this->setColumn(SURNAME, i18n("Surname"), &PersonDisplayEntity::surname);
    this->setColumn(ROOT, i18n("Root"), &PersonDisplayEntity::root);
    this->setColumn(SEX, i18n("Sex"), &PersonDisplayEntity::sex);
    this->setColumn(DISPLAY_NAME, i18n("Name"), [](const PersonDisplayEntity& p) {
        return construct_display_name(p.titles, p.givenNames, p.prefix, p.surname);
    });

    connectToTable<Schema::People>(this, this->personId);
    connectToTable<Schema::Names>(this, this->personId);

    reload();
}

void PersonDetailModel::reload() {
    PersonRepository repo;
    if (const auto result = repo.findDisplayById(personId)) {
        this->setItems({*result});
    } else {
        this->setItems({});
    }
}
