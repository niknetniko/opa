/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./event_repository.h"

#include "../../core/data_event_broker.h"

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
        DataEventBroker::instance().notifyChanged<Schema::EventTypes>(newId.value());
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
        DataEventBroker::instance().notifyChanged<Schema::EventRoles>(newId.value());
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

std::optional<EventEntity> EventRepository::findEventById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, type_id, date, name, note FROM events WHERE id = :id"_s;
    return fetchOne<EventEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEvent(IntegerPrimaryKey typeId) const {
    const auto sql = u"INSERT INTO events (type_id) VALUES (:type_id)"_s;
    const auto newId = QueryHelper::insert(sql, {{u":type_id"_s, typeId}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(newId.value());
    }
    return newId;
}

bool EventRepository::updateEvent(
    IntegerPrimaryKey id,
    IntegerPrimaryKey typeId,
    const QString& date,
    const QString& name,
    const QString& note
) const {
    const auto sql =
        u"UPDATE events SET type_id = :type_id, date = :date, name = :name, note = :note WHERE id = :id"_s;
    const QVariantMap bindings = {
        {u":type_id"_s, typeId},
        {u":date"_s, date},
        {u":name"_s, name},
        {u":note"_s, note},
        {u":id"_s, id},
    };
    return QueryHelper::executeAndNotify<Schema::Events>(id, sql, bindings);
}

bool EventRepository::deleteEvent(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM events WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::Events>(id, sql, {{u":id"_s, id}});
}

QList<EventRelationEntity> EventRepository::findRelationsForEvent(IntegerPrimaryKey eventId) const {
    const auto sql = u"SELECT event_id, person_id, role_id FROM event_relations WHERE event_id = :event_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":event_id"_s, eventId}});
}

QList<EventRelationEntity> EventRepository::findRelationsForPerson(IntegerPrimaryKey personId) const {
    const auto sql = u"SELECT event_id, person_id, role_id FROM event_relations WHERE person_id = :person_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":person_id"_s, personId}});
}

bool EventRepository::insertEventRelation(
    IntegerPrimaryKey eventId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId
) const {
    const auto sql =
        u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (:event_id, :person_id, :role_id)"_s;
    const QVariantMap bindings = {
        {u":event_id"_s, eventId},
        {u":person_id"_s, personId},
        {u":role_id"_s, roleId},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelations>(eventId);
    }
    return ok;
}

bool EventRepository::deleteEventRelation(
    IntegerPrimaryKey eventId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId
) const {
    const auto sql =
        u"DELETE FROM event_relations WHERE event_id = :event_id AND person_id = :person_id AND role_id = :role_id"_s;
    const QVariantMap bindings = {
        {u":event_id"_s, eventId},
        {u":person_id"_s, personId},
        {u":role_id"_s, roleId},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelations>(eventId);
    }
    return ok;
}

QList<PersonEventEntity> EventRepository::findEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql = u"SELECT events.id, er.id AS role_id, er.role, et.type, events.date, events.name "
                     "FROM events "
                     "LEFT JOIN event_types AS et ON events.type_id = et.id "
                     "LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
                     "LEFT JOIN event_roles AS er ON er.id = erel.role_id "
                     "WHERE erel.person_id = :person_id "
                     "ORDER BY events.date ASC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findBirthEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql = u"SELECT events.id, er.id AS role_id, er.role, et.type, events.date, events.name "
                     "FROM events "
                     "LEFT JOIN event_types AS et ON events.type_id = et.id "
                     "LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
                     "LEFT JOIN event_roles AS er ON er.id = erel.role_id "
                     "WHERE erel.person_id = :person_id "
                     "AND er.role = 'Primary' "
                     "AND et.type IN ('Birth', 'Baptism') "
                     "AND LENGTH(events.date) != 0 "
                     "ORDER BY et.type = 'Birth' DESC, et.type = 'Baptism' DESC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findDeathEventsForPerson(IntegerPrimaryKey personId) const {
    const auto sql = u"SELECT events.id, er.id AS role_id, er.role, et.type, events.date, events.name "
                     "FROM events "
                     "LEFT JOIN event_types AS et ON events.type_id = et.id "
                     "LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
                     "LEFT JOIN event_roles AS er ON er.id = erel.role_id "
                     "WHERE erel.person_id = :person_id "
                     "AND er.role = 'Primary' "
                     "AND et.type IN ('Death', 'Funeral') "
                     "ORDER BY et.type = 'Death' DESC, et.type = 'Funeral' DESC"_s;
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

std::optional<IntegerPrimaryKey> EventRepository::insertEventWithRelation(
    IntegerPrimaryKey typeId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId
) const {
    const auto eventId = insertEvent(typeId);
    if (!eventId) {
        return std::nullopt;
    }
    if (!insertEventRelation(*eventId, personId, roleId)) {
        deleteEvent(*eventId);
        return std::nullopt;
    }
    return eventId;
}
