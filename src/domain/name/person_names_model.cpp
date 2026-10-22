/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./person_names_model.h"

#include "../../core/data_event_broker.h"
#include "name_repository.h"
#include "names.h"

#include <KLocalizedString>

PersonNamesModel::PersonNamesModel(IntegerPrimaryKey personId, QObject* parent) :
    ObjectTableModel(parent),
    personId(personId) {


    this->setColumn(ID, i18n("ID"), &NameEntity::id);
    this->setColumn(SORT, i18n("Volgorde"), &NameEntity::sort);
    this->setColumn(TITLES, i18n("Titels"), &NameEntity::titles);
    this->setColumn(GIVEN_NAMES, i18n("Voornamen"), &NameEntity::givenNames);
    this->setColumn(PREFIX, i18n("Voorvoegsels"), &NameEntity::prefix);
    this->setColumn(SURNAME, i18n("Achternaam"), &NameEntity::surname);
    this->setColumn(ORIGIN, i18n("Oorsprong"), &NameEntity::origin);

    connectToTable<Schema::People>(this, this->personId);
    connectToTable<Schema::Names>(this);

    reload();
}

void PersonNamesModel::reload() {
    const NameRepository repo;
    this->setItems(repo.findNamesForPerson(personId));
}
