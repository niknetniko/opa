/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
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

    bool insertEventRelation(IntegerPrimaryKey eventId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId) const;

    bool deleteEventRelation(IntegerPrimaryKey eventId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId) const;

    [[nodiscard]] QList<PersonEventEntity> findEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findBirthEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findDeathEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    findEventTypeIdByName(const QString& typeName) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    findEventRoleIdByName(const QString& roleName) const;

    std::optional<IntegerPrimaryKey>
    insertEventWithRelation(IntegerPrimaryKey typeId, IntegerPrimaryKey personId, IntegerPrimaryKey roleId) const;

    [[nodiscard]] bool isEventTypeUsed(IntegerPrimaryKey typeId) const;

    bool reassignEventTypeId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const;

    [[nodiscard]] bool isEventRoleUsed(IntegerPrimaryKey roleId) const;

    bool reassignEventRoleId(IntegerPrimaryKey fromId, IntegerPrimaryKey toId) const;
};
