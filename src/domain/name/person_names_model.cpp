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


    this->setColumn(ID, i18n("ID"), &NameWithOriginEntity::id);
    this->setColumn(SORT, i18n("Volgorde"), &NameWithOriginEntity::sort);
    this->setColumn(TITLES, i18n("Titels"), &NameWithOriginEntity::titles);
    this->setColumn(GIVEN_NAMES, i18n("Voornamen"), &NameWithOriginEntity::givenNames);
    this->setColumn(PREFIX, i18n("Voorvoegsels"), &NameWithOriginEntity::prefix);
    this->setColumn(SURNAME, i18n("Achternaam"), &NameWithOriginEntity::surname);
    this->setColumn(ORIGIN, i18n("Oorsprong"), &NameWithOriginEntity::origin);

    connectToTable<Schema::People>(this, this->personId);
    connectToTable<Schema::Names>(this);

    reload();
}

void PersonNamesModel::reload() {
    NameRepository repo;
    this->setItems(repo.findNamesWithOriginForPerson(personId));
}
