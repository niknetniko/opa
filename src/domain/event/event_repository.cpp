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

QList<EventDisplayEntity> EventRepository::findAllEvents() const {
    // language=sqlite
    const auto sql = u"SELECT id, type, date, name FROM events ORDER BY date_sort NULLS LAST"_s;
    return fetchAll<EventDisplayEntity>(sql);
}

std::optional<EventEntity> EventRepository::findEventById(IntegerPrimaryKey id) const {
    // language=sqlite
    const auto sql = u"SELECT id, type, date, name, note, location_id FROM events WHERE id = :id"_s;
    return fetchOne<EventEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEvent(const QString& type) const {
    // language=sqlite
    const auto sql = u"INSERT INTO events (type) VALUES (:type)"_s;
    auto newId = QueryHelper::insert(sql, {{u":type"_s, type}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(newId);
    }
    return newId;
}

bool EventRepository::updateEvent(
    IntegerPrimaryKey id,
    const QString& type,
    const QString& date,
    const QString& name,
    const QString& note,
    const std::optional<IntegerPrimaryKey> locationId
) const {
    // language=sqlite
    const auto sql =
        u"UPDATE events SET type = :type, date = :date, date_sort = :date_sort, name = :name, note = :note, location_id = :location_id WHERE id = :id"_s;
    const auto parsedDate = GenealogicalDate::fromDatabaseRepresentation(date);
    const QVariantMap bindings = {
        {u":type"_s, type},
        {u":date"_s, date},
        {u":date_sort"_s, parsedDate.isNull() ? QVariant {} : QVariant {parsedDate.sortKey()}},
        {u":name"_s, name},
        {u":note"_s, note},
        {u":location_id"_s, locationId.has_value() ? QVariant(*locationId) : QVariant {}},
        {u":id"_s, id},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::Events>(id);
    }
    return ok;
}

bool EventRepository::deleteEvent(IntegerPrimaryKey id) const {
    // language=sqlite
    const auto sql = u"DELETE FROM events WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::Events>(id, sql, {{u":id"_s, id}});
}

QList<EventRelationEntity> EventRepository::findRelationsForEvent(IntegerPrimaryKey eventId) const {
    // language=sqlite
    const auto sql = u"SELECT id, event_id, person_id, role FROM event_relations WHERE event_id = :event_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":event_id"_s, eventId}});
}

QList<EventRelationEntity> EventRepository::findRelationsForPerson(IntegerPrimaryKey personId) const {
    // language=sqlite
    const auto sql = u"SELECT id, event_id, person_id, role FROM event_relations WHERE person_id = :person_id"_s;
    return fetchAll<EventRelationEntity>(sql, {{u":person_id"_s, personId}});
}

std::optional<IntegerPrimaryKey> EventRepository::insertEventRelation(
    IntegerPrimaryKey eventId,
    IntegerPrimaryKey personId,
    const QString& role
) const {
    // language=sqlite
    const auto sql = u"INSERT INTO event_relations (event_id, person_id, role) VALUES (:event_id, :person_id, :role)"_s;
    const QVariantMap bindings = {
        {u":event_id"_s, eventId},
        {u":person_id"_s, personId},
        {u":role"_s, role},
    };
    const auto newId = QueryHelper::insert(sql, bindings);
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventRelations>(newId);
    }
    return newId;
}

bool EventRepository::deleteEventRelation(IntegerPrimaryKey relationId) const {
    // language=sqlite
    const auto sql = u"DELETE FROM event_relations WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::EventRelations>(relationId, sql, {{u":id"_s, relationId}});
}

bool EventRepository::updateEventRelationRole(IntegerPrimaryKey relationId, const QString& role) const {
    // language=sqlite
    const auto sql = u"UPDATE event_relations SET role = :role WHERE id = :id"_s;
    const QVariantMap bindings = {
        {u":role"_s, role},
        {u":id"_s, relationId},
    };
    return QueryHelper::executeAndNotify<Schema::EventRelations>(relationId, sql, bindings);
}

QList<PersonEventEntity> EventRepository::findEventsForPerson(IntegerPrimaryKey personId) const {
    // language=sqlite
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, erel.role, events.type, events.date, events.name FROM events "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"WHERE erel.person_id = :person_id "
        u"ORDER BY events.date_sort NULLS LAST"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findBirthEventsForPerson(IntegerPrimaryKey personId) const {
    // TODO: do not hard code birth events here, have a central place somewhere
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, erel.role, events.type, events.date, events.name "
        u"FROM events "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"WHERE erel.person_id = :person_id "
        u"AND erel.role = 'Primary' "
        u"AND events.type IN ('Birth', 'Baptism') "
        u"AND LENGTH(events.date) != 0 "
        u"ORDER BY events.type = 'Birth' DESC, events.type = 'Baptism' DESC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
}

QList<PersonEventEntity> EventRepository::findDeathEventsForPerson(IntegerPrimaryKey personId) const {
    // TODO: central place for event types
    const auto sql =
        u"SELECT events.id, erel.id AS relation_id, erel.role, events.type, events.date, events.name "
        u"FROM events "
        u"LEFT JOIN event_relations AS erel ON events.id = erel.event_id "
        u"WHERE erel.person_id = :person_id "
        u"AND erel.role = 'Primary' "
        u"AND events.type IN ('Death', 'Funeral') "
        u"ORDER BY events.type = 'Death' DESC, events.type = 'Funeral' DESC"_s;
    return fetchAll<PersonEventEntity>(sql, {{u":person_id"_s, personId}});
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
    const QString& type,
    IntegerPrimaryKey personId,
    const QString& role
) const {
    return executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
        auto eventId = insertEvent(type);
        if (!eventId) {
            return std::nullopt;
        }

        if (!insertEventRelation(*eventId, personId, role).has_value()) {
            return std::nullopt;
        }

        return eventId;
    });
}

std::optional<IntegerPrimaryKey> EventRepository::insertFullEvent(
    const QString& type,
    const QString& date,
    const QString& name,
    const QString& note,
    IntegerPrimaryKey personId,
    const QString& role,
    std::optional<IntegerPrimaryKey> locationId
) const {
    return executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
        auto eventId = insertEvent(type);
        if (!eventId) {
            return std::nullopt;
        }

        if (!updateEvent(*eventId, type, date, name, note, locationId)) {
            return std::nullopt;
        }

        if (!insertEventRelation(*eventId, personId, role).has_value()) {
            return std::nullopt;
        }

        return eventId;
    });
}
