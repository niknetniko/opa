/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event.h"

#include "data_manager.h"
#include "database/schema.h"

#include <KLocalizedString>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

EventRolesModel::EventRolesModel(QObject* parent) : QSqlTableModel(parent) {
    QSqlTableModel::setTable(Schema::EventRolesTable);

    QSqlTableModel::setHeaderData(ID, Qt::Horizontal, i18n("Id"));
    QSqlTableModel::setHeaderData(ROLE, Qt::Horizontal, i18n("Rol"));
}

QList<EventTypes::Values> EventTypes::relationshipStartingEvents() {
    return {Marriage};
}

QList<EventTypes::Values> EventTypes::birthEventsInOrder() {
    return {Birth, Baptism};
}

QList<EventTypes::Values> EventTypes::deathEventsInOrder() {
    return {Death, Funeral};
}

QList<EventRoles::Values> EventRoles::parentRoles() {
    return {Mother, Father, AdoptiveParent, Stepparent, FosterParent, SurrogateMother, GeneticDonor, RecognizedParent};
}

IntegerPrimaryKey EventRolesModel::getDefaultRole() {
    return getRoleId(EventRoles::Primary);
}

IntegerPrimaryKey EventRolesModel::getRoleId(EventRoles::Values role) {
    auto* roleModel = DataManager::get().eventRolesModel();
    return getTypeId(roleModel, role, EventRoles::nameOriginToString, ROLE, ID);
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

ParentEventRolesModel::ParentEventRolesModel(QObject* parent) : QSortFilterProxyModel(parent) {
    auto list = EventRoles::parentRoles();
    this->parentRoles = QSet(list.constBegin(), list.constEnd());
    QSortFilterProxyModel::setSourceModel(DataManager::get().eventRolesModel());
}

bool ParentEventRolesModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    Q_UNUSED(sourceParent);
    auto rawValue = sourceModel()->index(sourceRow, EventRolesModel::ROLE).data();
    auto asEnumValue = enumFromString<EventRoles::Values>(rawValue.toString());
    return parentRoles.contains(asEnumValue);
}

RelationshipEventTypesModel::RelationshipEventTypesModel(QObject* parent) : QSortFilterProxyModel(parent) {
    auto list = EventTypes::relationshipStartingEvents();
    this->relationshipTypes = QSet(list.constBegin(), list.constEnd());
    QSortFilterProxyModel::setSourceModel(DataManager::get().eventTypesModel());
}

bool RelationshipEventTypesModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    Q_UNUSED(sourceParent);
    auto rawValue = sourceModel()->index(sourceRow, EventTypesModel::TYPE).data();
    auto asEnumValue = enumFromString<EventTypes::Values>(rawValue.toString());
    return relationshipTypes.contains(asEnumValue);
}

NewEventInformation addEventToPerson(EventTypes::Values eventType, IntegerPrimaryKey person) {
    auto* typeModel = DataManager::get().eventTypesModel();
    auto typeId = getTypeId(typeModel, eventType, EventTypes::typeToString, EventTypesModel::TYPE, EventTypesModel::ID);
    auto roleId = EventRolesModel::getDefaultRole();

    if (!QSqlDatabase::database().transaction()) {
        qFatal() << "Could not get transaction on database:";
        qFatal() << QSqlDatabase::database().lastError();
        return {};
    }

    auto* eventModel = DataManager::get().eventsModel();
    auto newEventRecord = eventModel->record();
    newEventRecord.setGenerated(EventsModel::ID, false);
    newEventRecord.setValue(EventsModel::TYPE_ID, typeId);

    if (!eventModel->insertRecord(-1, newEventRecord)) {
        qWarning() << "Could not insert new event:";
        qWarning() << eventModel->lastError();
        if (!QSqlDatabase::database().rollback()) {
            qFatal() << "Additionally, could not revert transaction:";
            qFatal() << eventModel->lastError();
        }
        return {};
    }

    auto newEventId = eventModel->query().lastInsertId();
    if (!newEventId.isValid()) {
        qWarning() << "Could not get last inserted ID:";
        qWarning() << eventModel->lastError();
        if (!QSqlDatabase::database().rollback()) {
            qFatal() << "Additionally, could not revert transaction:";
            qFatal() << eventModel->lastError();
        }
        return {};
    }

    auto* eventRelationModel = DataManager::get().eventRelationsModel();
    auto eventRelationRecord = eventRelationModel->record();
    eventRelationRecord.setValue(EventRelationsModel::EVENT_ID, newEventId);
    eventRelationRecord.setValue(EventRelationsModel::PERSON_ID, person);
    eventRelationRecord.setValue(EventRelationsModel::ROLE_ID, roleId);

    if (!eventRelationModel->insertRecord(-1, eventRelationRecord)) {
        qWarning() << "Could not insert event relation:";
        qWarning() << eventRelationModel->lastError();
        if (!QSqlDatabase::database().rollback()) {
            qFatal() << "Additionally, could not revert transaction:";
            qFatal() << eventModel->lastError();
        }
        return {};
    }

    if (!QSqlDatabase::database().commit()) {
        qFatal() << "Could not commit transaction:";
        qFatal() << QSqlDatabase::database().lastError();
        if (!QSqlDatabase::database().rollback()) {
            qFatal() << "Additionally, could not revert transaction:";
            qFatal() << eventModel->lastError();
        }
        return {};
    }

    return {.eventId = newEventId, .roleId = roleId, .typeId = typeId};
}

BirthEventsModel::BirthEventsModel(IntegerPrimaryKey person, QObject* parent) :
    QSqlQueryModel(parent),
    personId(person) {

    auto birthEventTypes = EventTypes::birthEventsInOrder();
    QStringList stringTypes;
    for (auto birthEventType: birthEventTypes) {
        auto asString = QString::fromUtf8(EventTypes::typeToString[birthEventType].untranslatedText());
        stringTypes.append(QStringLiteral("'%1'").arg(asString));
    }

    // Generate the sort query.
    QStringList sortQueryParts;
    for (auto stringType: stringTypes) {
        sortQueryParts.append(QStringLiteral("type = %1 DESC").arg(stringType));
    }
    auto sortQuery = sortQueryParts.join(QStringLiteral(", "));
    auto filterQuery = stringTypes.join(QStringLiteral(", "));

    query_ = QStringLiteral(R"-(
SELECT events.id, type_id, type, date, name, note
FROM events
       LEFT JOIN event_types on event_types.id = events.type_id
       LEFT JOIN event_relations ON events.id = event_relations.event_id
       LEFT JOIN event_roles ON event_roles.id = event_relations.role_id
WHERE event_roles.role = 'Primary'
  AND event_types.type IN (%1)
  AND person_id = :id
  AND LENGTH(date) != 0
ORDER BY %2;
)-")
                 .arg(filterQuery, sortQuery);
}

void BirthEventsModel::resetAndLoadData() {
    QSqlQuery query;
    query.prepare(query_);
    qDebug() << "query is" << query_;
    query.bindValue(QStringLiteral(":id"), personId);

    if (!query.exec()) {
        qWarning() << "Something went wrong in person birth events query.";
        qDebug() << query.lastError();
    }

    setQuery(std::move(query));

    setHeaderData(ID, Qt::Horizontal, i18n("ID"));
    setHeaderData(TYPE_ID, Qt::Horizontal, i18n("Type ID"));
    setHeaderData(TYPE, Qt::Horizontal, i18n("Type"));
    setHeaderData(DATE, Qt::Horizontal, i18n("Date"));
    setHeaderData(NAME, Qt::Horizontal, i18n("Name"));
    setHeaderData(NOTE, Qt::Horizontal, i18n("Note"));
}

DeathEventsModel::DeathEventsModel(IntegerPrimaryKey person, QObject* parent) :
    QSqlQueryModel(parent),
    personId(person) {

    auto deathEventTypes = EventTypes::deathEventsInOrder();
    QStringList stringTypes;
    for (auto birthEventType: deathEventTypes) {
        auto asString = QString::fromUtf8(EventTypes::typeToString[birthEventType].untranslatedText());
        stringTypes.append(QStringLiteral("'%1'").arg(asString));
    }

    // Generate the sort query.
    QStringList sortQueryParts;
    for (auto stringType: stringTypes) {
        sortQueryParts.append(QStringLiteral("type = %1 DESC").arg(stringType));
    }
    auto sortQuery = sortQueryParts.join(QStringLiteral(", "));
    auto filterQuery = stringTypes.join(QStringLiteral(", "));

    query_ = QStringLiteral(R"-(
SELECT events.id, type_id, type, date, name, note
FROM events
       LEFT JOIN event_types on event_types.id = events.type_id
       LEFT JOIN event_relations ON events.id = event_relations.event_id
       LEFT JOIN event_roles ON event_roles.id = event_relations.role_id
WHERE event_roles.role = 'Primary'
  AND event_types.type IN (%1)
  AND person_id = :id
ORDER BY %2;
)-")
                 .arg(filterQuery, sortQuery);
}

void DeathEventsModel::resetAndLoadData() {
    QSqlQuery query;
    query.prepare(query_);
    qDebug() << "query is" << query_;
    query.bindValue(QStringLiteral(":id"), personId);

    if (!query.exec()) {
        qWarning() << "Something went wrong in person death events query.";
        qDebug() << query.lastError();
    }

    setQuery(std::move(query));

    setHeaderData(ID, Qt::Horizontal, i18n("ID"));
    setHeaderData(TYPE_ID, Qt::Horizontal, i18n("Type ID"));
    setHeaderData(TYPE, Qt::Horizontal, i18n("Type"));
    setHeaderData(DATE, Qt::Horizontal, i18n("Date"));
    setHeaderData(NAME, Qt::Horizontal, i18n("Name"));
    setHeaderData(NOTE, Qt::Horizontal, i18n("Note"));
}
