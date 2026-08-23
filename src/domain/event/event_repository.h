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
    [[nodiscard]] std::optional<EventEntity> findEventById(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventDisplayEntity> findAllEvents() const;

    std::optional<IntegerPrimaryKey> insertEvent(const QString& type) const;

    bool updateEvent(
        IntegerPrimaryKey id,
        const QString& type,
        const QString& date,
        const QString& name,
        const QString& note,
        std::optional<IntegerPrimaryKey> locationId = std::nullopt
    ) const;

    bool deleteEvent(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForEvent(IntegerPrimaryKey eventId) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForPerson(IntegerPrimaryKey personId) const;

    std::optional<IntegerPrimaryKey>
    insertEventRelation(IntegerPrimaryKey eventId, IntegerPrimaryKey personId, const QString& role) const;

    bool deleteEventRelation(IntegerPrimaryKey relationId) const;

    bool updateEventRelationRole(IntegerPrimaryKey relationId, const QString& role) const;

    [[nodiscard]] QList<PersonEventEntity> findEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findBirthEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findDeathEventsForPerson(IntegerPrimaryKey personId) const;

    std::optional<IntegerPrimaryKey>
    insertEventWithRelation(const QString& type, IntegerPrimaryKey personId, const QString& role) const;

    std::optional<IntegerPrimaryKey> insertFullEvent(
         const QString& type,
        const QString& date,
        const QString& name,
        const QString& note,
        IntegerPrimaryKey personId,
        const QString& role,
        std::optional<IntegerPrimaryKey> locationId = std::nullopt
    ) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEvent(IntegerPrimaryKey eventId) const;
    bool addEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;
    bool removeEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEventRelation(IntegerPrimaryKey relationId) const;
    bool addEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;
    bool removeEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;
};
