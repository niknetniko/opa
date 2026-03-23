/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
#include "domain/source/source_entities.h"
#include "event_entities.h"

#include <QList>
#include <optional>

class EventRepository : public BaseRepository {
public:
    [[nodiscard]] QList<EventTypeEntity> findAllEventTypes() const;

    [[nodiscard]] std::optional<EventTypeEntity> findEventTypeById(IntegerPrimaryKey id) const;

    std::optional<IntegerPrimaryKey> insertEventType(const QString& type) const;

    bool updateEventType(IntegerPrimaryKey id, const QString& type) const;

    bool deleteEventType(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventRoleEntity> findAllEventRoles() const;

    [[nodiscard]] std::optional<EventRoleEntity> findEventRoleById(IntegerPrimaryKey id) const;

    std::optional<IntegerPrimaryKey> insertEventRole(const QString& role) const;

    bool updateEventRole(IntegerPrimaryKey id, const QString& role) const;

    bool deleteEventRole(IntegerPrimaryKey id) const;

    [[nodiscard]] std::optional<EventEntity> findEventById(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventDisplayEntity> findAllEvents() const;

    std::optional<IntegerPrimaryKey> insertEvent(IntegerPrimaryKey typeId) const;

    bool updateEvent(
        IntegerPrimaryKey id,
        IntegerPrimaryKey typeId,
        const QString& date,
        const QString& name,
        const QString& note
    ) const;

    bool deleteEvent(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForEvent(IntegerPrimaryKey eventId) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForPerson(IntegerPrimaryKey personId) const;

    std::optional<IntegerPrimaryKey>
    insertEventRelation(IntegerPrimaryKey eventId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId) const;

    bool deleteEventRelation(IntegerPrimaryKey relationId) const;

    bool updateEventRelationRole(IntegerPrimaryKey relationId, IntegerPrimaryKey newRoleId) const;

    [[nodiscard]] QList<PersonEventEntity> findEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findBirthEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findDeathEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    findEventTypeIdByName(const QString& typeName) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    findEventRoleIdByName(const QString& roleName) const;

    std::optional<IntegerPrimaryKey>
    insertEventWithRelation(IntegerPrimaryKey typeId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId) const;

    std::optional<IntegerPrimaryKey> insertFullEvent(
        IntegerPrimaryKey typeId,
        const QString& date,
        const QString& name,
        const QString& note,
        IntegerPrimaryKey personId,
        IntegerPrimaryKey roleId
    ) const;

    [[nodiscard]] bool isEventTypeUsed(IntegerPrimaryKey typeId) const;

    bool reassignEventTypeId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const;

    [[nodiscard]] bool isEventRoleUsed(IntegerPrimaryKey roleId) const;

    bool reassignEventRoleId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEvent(IntegerPrimaryKey eventId) const;
    bool addEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;
    bool removeEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEventRelation(IntegerPrimaryKey relationId) const;
    bool addEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;
    bool removeEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;

};
