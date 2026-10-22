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

    [[nodiscard]] std::optional<IntegerPrimaryKey> insertEvent(const QString& type) const;

    [[nodiscard]] bool updateEvent(
        IntegerPrimaryKey id,
        const QString& type,
        const QString& date,
        const QString& name,
        const QString& note,
        std::optional<IntegerPrimaryKey> locationId = std::nullopt
    ) const;

    [[nodiscard]] bool deleteEvent(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForEvent(IntegerPrimaryKey eventId) const;

    [[nodiscard]] QList<EventRelationEntity> findRelationsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    insertEventRelation(IntegerPrimaryKey eventId, IntegerPrimaryKey personId, const QString& role) const;

    [[nodiscard]] bool deleteEventRelation(IntegerPrimaryKey relationId) const;

    [[nodiscard]] bool updateEventRelationRole(IntegerPrimaryKey relationId, const QString& role) const;

    [[nodiscard]] QList<PersonEventEntity> findEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findBirthEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<PersonEventEntity> findDeathEventsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey>
    insertEventWithRelation(const QString& type, IntegerPrimaryKey personId, const QString& role) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey> insertFullEvent(
        const QString& type,
        const QString& date,
        const QString& name,
        const QString& note,
        IntegerPrimaryKey personId,
        const QString& role,
        std::optional<IntegerPrimaryKey> locationId = std::nullopt
    ) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEvent(IntegerPrimaryKey eventId) const;
    [[nodiscard]] bool addEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;
    [[nodiscard]] bool removeEventCitation(IntegerPrimaryKey eventId, IntegerPrimaryKey sourceId) const;

    [[nodiscard]] QList<SourceEntity> findCitationsForEventRelation(IntegerPrimaryKey relationId) const;
    [[nodiscard]] bool addEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;
    [[nodiscard]] bool removeEventRelationCitation(IntegerPrimaryKey relationId, IntegerPrimaryKey sourceId) const;
};
