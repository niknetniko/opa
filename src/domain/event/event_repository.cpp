/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./event_repository.h"

#include "../../core/data_event_broker.h"
#include "database/database.h"
#include "dates/genealogical_date.h"

using namespace Qt::StringLiterals;

QList<EventTypeEntity> EventRepository::findAllEventTypes() const {
    const auto sql = u"SELECT id, type, builtin FROM event_types ORDER BY id ASC"_s;
    return fetchAll<EventTypeEntity>(sql);
}

std::optional<EventTypeEntity> EventRepository::findEventTypeById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, type, builtin FROM event_types WHERE id = :id"_s;
    return fetchOne<EventTypeEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEventType(const QString& type) const {
    const auto sql = u"INSERT INTO event_types (type, builtin) VALUES (:type, false)"_s;
    const auto newId = QueryHelper::insert(sql, {{u":type"_s, type}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventTypes>(newId);
    }
    return newId;
}

bool EventRepository::updateEventType(IntegerPrimaryKey id, const QString& type) const {
    const auto sql = u"UPDATE event_types SET type = :type WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventTypes>(id, sql, {{u":type"_s, type}, {u":id"_s, id}});
}

bool EventRepository::deleteEventType(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM event_types WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventTypes>(id, sql, {{u":id"_s, id}});
}

QList<EventRoleEntity> EventRepository::findAllEventRoles() const {
    const auto sql = u"SELECT id, role, builtin FROM event_roles ORDER BY id ASC"_s;
    return fetchAll<EventRoleEntity>(sql);
}

std::optional<EventRoleEntity> EventRepository::findEventRoleById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, role, builtin FROM event_roles WHERE id = :id"_s;
    return fetchOne<EventRoleEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEventRole(const QString& role) const {
    const auto sql = u"INSERT INTO event_roles (role, builtin) VALUES (:role, false)"_s;
    const auto newId = QueryHelper::insert(sql, {{u":role"_s, role}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventRoles>(newId);
    }
    return newId;
}

bool EventRepository::updateEventRole(IntegerPrimaryKey id, const QString& role) const {
    const auto sql = u"UPDATE event_roles SET role = :role WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventRoles>(id, sql, {{u":role"_s, role}, {u":id"_s, id}});
}

bool EventRepository::deleteEventRole(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM event_roles WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventRoles>(id, sql, {{u":id"_s, id}});
}

QList<EventDisplayEntity> EventRepository::findAllEvents() const {
    const auto sql = u"SELECT e.id, e.type_id, et.type, e.date, e.name "
                     u"FROM events e LEFT JOIN event_types et ON e.type_id = et.id "
                     u"ORDER BY e.date_sort ASC NULLS LAST"_s;
    return fetchAll<EventDisplayEntity>(sql);
}

std::optional<EventEntity> EventRepository::findEventById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, type_id, date, name, note, location_id FROM events WHERE id = :id"_s;
    return fetchOne<EventEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEvent(IntegerPrimaryKey typeId) const {
    const auto sql = u"INSERT INTO events (type_id) VALUES (:type_id)"_s;
    auto newId = QueryHelper::insert(sql, {{u":type_id"_s, typeId}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(newId);
    }
    return newId;
}

bool EventRepository::updateEvent(
    IntegerPrimaryKey id,
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    std::optional<IntegerPrimaryKey> locationId
) const {
    const auto sql =
        u"UPDATE events SET type_id = :type_id, date = :date, date_sort = :date_sort, name = :name, note = :note, location_id = :location_id WHERE id = :id"_s;
    const auto parsedDate = GenealogicalDate::fromDatabaseRepresentation(date);
    const QVariantMap bindings = {
        {u":type_id"_s, typeId},
        {u":date"_s, date},
        {u":date_sort"_s, parsedDate.isNull() ? QVariant{} : QVariant{parsedDate.sortKey()}},
        {u":name"_s, name},
        {u":note"_s, note},
        {u":location_id"_s, locationId.has_value() ? QVariant(*locationId) : QVariant{}},
        {u":id"_s, id},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(id);
    }
    return ok;
}

bool EventRepository::deleteEvent(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM events WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::Events>(id, sql, {{u":id"_s, id}});
}

QList<EventRelationEntity> EventRepository::findRelationsForEvent(IntegerPrimaryKey eventId) const {
    const auto sql = u"SELECT id, event_id, person_id, role_id FROM event_relations WHERE event_id = :event_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":event_id"_s, eventId}});
}

QList<EventRelationEntity> EventRepository::findRelationsForPerson(IntegerPrimaryKey personId) const {
    const auto sql = u"SELECT id, event_id, person_id, role_id FROM event_relations WHERE person_id = :person_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":person_id"_s, personId}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEventRelation(
    IntegerPrimaryKey eventId,
    IntegerPrimaryKey personId,
    IntegerPrimaryKey roleId
) const {
    const auto sql = u"INSERT INTO event_relations (event_id, person_id, role_id) "
                     u"VALUES (:event_id, :person_id, :role_id)"_s;
    const QVariantMap bindings = {
        {u":event_id"_s, eventId},
        {u":person_id"_s, personId},
        {u":role_id"_s, roleId},
    };
    const auto newId = QueryHelper::insert(sql, bindings);
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelations>(newId);
    }
    return newId;
}

bool EventRepository::deleteEventRelation(IntegerPrimaryKey relationId) const {
    const auto sql = u"DELETE FROM event_relations WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventRelations>(relationId, sql, {{u":id"_s, relationId}});
}

bool EventRepository::updateEventRelationRole(IntegerPrimaryKey relationId, IntegerPrimaryKey newRoleId) const {
    const auto sql = u"UPDATE event_relations SET role_id = :role_id WHERE id = :id"_s;
    const QVariantMap bindings = {
        {u":role_id"_s, newRoleId},
        {u":id"_s, relationId},
    };
    return QueryHelper::executeAndNotify<Schema::EventRelations>(relationId, sql, bindings);
}

QList<PersonEventEntity> EventRepository::findEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, er.id AS role_id, er.role, et.type, events.date, events.name "
        u"FROM events "
        u"LEFT JOIN event_types AS et ON events.type_id = et.id "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"LEFT JOIN event_roles AS er ON er.id = erel.role_id "
        u"WHERE erel.person_id = :person_id "
        u"ORDER BY events.date_sort ASC NULLS LAST"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findBirthEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, er.id AS role_id, er.role, et.type, events.date, events.name "
        u"FROM events "
        u"LEFT JOIN event_types AS et ON events.type_id = et.id "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"LEFT JOIN event_roles AS er ON er.id = erel.role_id "
        u"WHERE erel.person_id = :person_id "
        u"AND er.role = 'Primary' "
        u"AND et.type IN ('Birth', 'Baptism') "
        u"AND LENGTH(events.date) != 0 "
        u"ORDER BY et.type = 'Birth' DESC, et.type = 'Baptism' DESC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findDeathEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, er.id AS role_id, er.role, et.type, events.date, events.name "
        u"FROM events "
        u"LEFT JOIN event_types AS et ON events.type_id = et.id "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"LEFT JOIN event_roles AS er ON er.id = erel.role_id "
        u"WHERE erel.person_id = :person_id "
        u"AND er.role = 'Primary' "
        u"AND et.type IN ('Death', 'Funeral') "
        u"ORDER BY et.type = 'Death' DESC, et.type = 'Funeral' DESC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

std::optional<IntegerPrimaryKey> EventRepository::findEventTypeIdByName(const QString& typeName) const {
    const auto sql = u"SELECT id FROM event_types WHERE type = :type"_s;
    const auto entity = fetchOne<EventTypeEntity>(sql, {{u":type"_s, typeName}});
    if (entity) {
        return entity->id;
    }
    return std::nullopt;
}

std::optional<IntegerPrimaryKey> EventRepository::findEventRoleIdByName(const QString& roleName) const {
    const auto sql = u"SELECT id, role, builtin FROM event_roles WHERE role = :role"_s;
    const auto entity = fetchOne<EventRoleEntity>(sql, {{u":role"_s, roleName}});
    if (entity) {
        return entity->id;
    }
    return std::nullopt;
}

bool EventRepository::isEventTypeUsed(IntegerPrimaryKey typeId) const {
    const auto sql = u"SELECT COUNT(*) FROM events WHERE type_id = :id"_s;
    auto [query, ok] = QueryHelper::executeWithResult(sql, {{u":id"_s, typeId}});
    if (ok && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool EventRepository::reassignEventTypeId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const {
    const auto sql = u"UPDATE events SET type_id = :to_id WHERE type_id = :from_id"_s;
    const QVariantMap bindings = {{u":to_id"_s, toId}, {u":from_id"_s, fromId}};
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(std::nullopt);
    }
    return ok;
}

bool EventRepository::isEventRoleUsed(IntegerPrimaryKey roleId) const {
    const auto sql = u"SELECT COUNT(*) FROM event_relations WHERE role_id = :id"_s;
    auto [query, ok] = QueryHelper::executeWithResult(sql, {{u":id"_s, roleId}});
    if (ok && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool EventRepository::reassignEventRoleId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const {
    const auto sql = u"UPDATE event_relations SET role_id = :to_id WHERE role_id = :from_id"_s;
    const QVariantMap bindings = {{u":to_id"_s, toId}, {u":from_id"_s, fromId}};
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelations>(std::nullopt);
    }
    return ok;
}

QList<SourceEntity> EventRepository::findCitationsForEvent(IntegerPrimaryKey eventId) const {
    const auto sql = u"SELECT s.* FROM event_citations ec "
                     u"JOIN sources s ON ec.source_id = s.id "
                     u"WHERE ec.event_id = :event_id "
                     u"ORDER BY s.title ASC"_s;
    return fetchAll<SourceEntity>(sql, {{u":event_id"_s, eventId}});
}

bool EventRepository::addEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const {
    const auto sql = u"INSERT INTO event_citations (event_id, source_id) VALUES (:event_id, :source_id)"_s;
    const QVariantMap bindings = {{u":event_id"_s, eventId}, {u":source_id"_s, sourceId}};
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventCitations>(eventId);
    }
    return ok;
}

bool EventRepository::removeEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const {
    const auto sql = u"DELETE FROM event_citations WHERE event_id = :event_id AND source_id = :source_id"_s;
    const QVariantMap bindings = {{u":event_id"_s, eventId}, {u":source_id"_s, sourceId}};
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventCitations>(eventId);
    }
    return ok;
}

QList<SourceEntity> EventRepository::findCitationsForEventRelation(IntegerPrimaryKey relationId) const {
    const auto sql = u"SELECT s.* FROM event_relation_citations erc "
                     u"JOIN sources s ON erc.source_id = s.id "
                     u"WHERE erc.event_relation_id = :relation_id "
                     u"ORDER BY s.title ASC"_s;
    return fetchAll<SourceEntity>(sql, {{u":relation_id"_s, relationId}});
}

bool EventRepository::addEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const {
    const auto sql = u"INSERT INTO event_relation_citations (event_relation_id, source_id) "
                     u"VALUES (:relation_id, :source_id)"_s;
    const QVariantMap bindings = {
        {u":relation_id"_s, relationId},
        {u":source_id"_s, sourceId},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelationCitations>(relationId);
    }
    return ok;
}

bool EventRepository::removeEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const {
    const auto sql = u"DELETE FROM event_relation_citations "
                     u"WHERE event_relation_id = :relation_id AND source_id = :source_id"_s;
    const QVariantMap bindings = {
        {u":relation_id"_s, relationId},
        {u":source_id"_s, sourceId},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelationCitations>(relationId);
    }
    return ok;
}

std::optional<IntegerPrimaryKey> EventRepository::insertEventWithRelation(
    IntegerPrimaryKey typeId,
    IntegerPrimaryKey personId,
    IntegerPrimaryKey roleId
) const {
    return executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
        auto eventId = insertEvent(typeId);
        if (!eventId) {
            return std::nullopt;
        }

        if (!insertEventRelation(*eventId, personId, roleId).has_value()) {
            return std::nullopt;
        }

        return eventId;
    });
}

std::optional<IntegerPrimaryKey> EventRepository::insertFullEvent(
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note,
    IntegerPrimaryKey personId,
    IntegerPrimaryKey roleId,
    std::optional<IntegerPrimaryKey> locationId
) const {
    return executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
        auto eventId = insertEvent(typeId);
        if (!eventId) {
            return std::nullopt;
        }

        if (!updateEvent(*eventId, typeId, date, name, note, locationId)) {
            return std::nullopt;
        }

        if (!insertEventRelation(*eventId, personId, roleId).has_value()) {
            return std::nullopt;
        }

        return eventId;
    });
}
