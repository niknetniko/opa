/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event.h"

#include "data_manager.h"
#include "database/schema.h"

#include <KLocalizedString>

EventRolesModel::EventRolesModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::EventRolesTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(ROLE, Qt::Horizontal, i18n("Rol"));
}

QVariant EventRolesModel::getDefaultRole() {
    auto* roleModel = DataManager::get().eventRolesModel();
    auto defaultRole = EventRoles::nameOriginToString[EventRoles::Primary].toString();
    auto defaultEventRoleIndex =
        roleModel->match(roleModel->index(0, EventRolesModel::ROLE), Qt::DisplayRole, defaultRole).constFirst();
    if (!defaultEventRoleIndex.isValid()) {
        qWarning() << "Default role not found, aborting new event.";
        return {};
    }
    return roleModel->index(defaultEventRoleIndex.row(), EventRolesModel::ID).data();
}

EventTypesModel::EventTypesModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::EventTypesTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(TYPE, Qt::Horizontal, i18n("Soort"));
}

EventRelationsModel::EventRelationsModel(QObject* parent, QSqlTableModel* rolesModel) :
    CustomSqlRelationalModel(parent) {
    CustomSqlRelationalModel::setTable(Schema::EventRelationsTable);

    this->setRelation(ROLE_ID, rolesModel, EventRolesModel::ROLE, EventRolesModel::ID);

    CustomSqlRelationalModel::setHeaderData(EVENT_ID, Qt::Horizontal, i18n("Gebeurtenis-id"));
    CustomSqlRelationalModel::setHeaderData(PERSON_ID, Qt::Horizontal, i18n("Persoon-id"));
    CustomSqlRelationalModel::setHeaderData(ROLE_ID, Qt::Horizontal, i18n("Rol-id"));
    CustomSqlRelationalModel::setHeaderData(ROLE, Qt::Horizontal, i18n("Rol"));
}

EventsModel::EventsModel(QObject* parent, QSqlTableModel* typesModel) : CustomSqlRelationalModel(parent) {
    CustomSqlRelationalModel::setTable(Schema::EventsTable);

    this->setRelation(TYPE_ID, typesModel, EventTypesModel::TYPE, EventTypesModel::ID);

    CustomSqlRelationalModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    CustomSqlRelationalModel::setHeaderData(TYPE_ID, Qt::Horizontal, i18n("Soort-id"));
    CustomSqlRelationalModel::setHeaderData(DATE, Qt::Horizontal, i18n("Datum"));
    CustomSqlRelationalModel::setHeaderData(NAME, Qt::Horizontal, i18n("Naam"));
    CustomSqlRelationalModel::setHeaderData(TYPE, Qt::Horizontal, i18n("Soort"));
    CustomSqlRelationalModel::setHeaderData(NOTE, Qt::Horizontal, i18n("Notitie"));
}
