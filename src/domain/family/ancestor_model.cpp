/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ancestor_model.h"

#include "core/data_event_broker.h"
#include "data/names.h"
#include "family_repository.h"

#include <KLocalizedString>

AncestorModel::AncestorModel(IntegerPrimaryKey personId, QObject* parent) :
    ObjectTableModel(parent),
    personId(personId) {

    this->setColumn(CHILD_ID, i18n("Child ID"), &AncestorEntity::childId);
    this->setColumn(FATHER_ID, i18n("Father ID"), [](const AncestorEntity& e) -> QVariant {
        return e.fatherId.has_value() ? QVariant(*e.fatherId) : QVariant();
    });
    this->setColumn(MOTHER_ID, i18n("Mother ID"), [](const AncestorEntity& e) -> QVariant {
        return e.motherId.has_value() ? QVariant(*e.motherId) : QVariant();
    });
    this->setColumn(VISITED, i18n("Visited"), &AncestorEntity::visited);
    this->setColumn(LEVEL, i18n("Level"), &AncestorEntity::level);
    this->setColumn(DISPLAY_NAME, i18n("Name"), [](const AncestorEntity& e) {
        return construct_display_name(e.titles, e.givenNames, e.prefix, e.surname);
    });

    connectToTable<Schema::People>(this);
    connectToTable<Schema::Names>(this);
    connectToTable<Schema::Events>(this);
    connectToTable<Schema::EventRoles>(this);
    connectToTable<Schema::EventRelations>(this);

    reload();
}

void AncestorModel::reload() {
    FamilyRepository repo;
    this->setItems(repo.findAncestorsForPerson(personId));
}
