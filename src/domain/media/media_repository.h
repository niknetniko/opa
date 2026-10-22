/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
#include "media_entities.h"

#include <QList>
#include <optional>

class MediaRepository : public BaseRepository {
public:
    // --- Core CRUD ---

    [[nodiscard]] QList<MediaEntity> findAll() const;
    [[nodiscard]] std::optional<MediaEntity> findById(IntegerPrimaryKey id) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey> insert(
        const QString& path,
        const std::optional<QString>& title,
        const std::optional<QString>& note,
        const QString& mimeType
    ) const;

    [[nodiscard]] bool
    update(IntegerPrimaryKey id, const std::optional<QString>& title, const std::optional<QString>& note) const;

    [[nodiscard]] bool remove(IntegerPrimaryKey id) const;

    // --- Queries per entity type ---

    [[nodiscard]] QList<MediaEntity> findForPerson(IntegerPrimaryKey personId) const;
    [[nodiscard]] QList<MediaEntity> findForName(IntegerPrimaryKey nameId) const;
    [[nodiscard]] QList<MediaEntity> findForEvent(IntegerPrimaryKey eventId) const;
    [[nodiscard]] QList<MediaEntity> findForEventRelation(IntegerPrimaryKey relationId) const;
    [[nodiscard]] QList<MediaEntity> findForSource(IntegerPrimaryKey sourceId) const;
    [[nodiscard]] QList<MediaEntity> findForLocation(IntegerPrimaryKey locationId) const;

    // --- Attach / detach per entity type ---

    [[nodiscard]] bool attachToPerson(IntegerPrimaryKey personId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromPerson(IntegerPrimaryKey personId, IntegerPrimaryKey mediaId) const;

    [[nodiscard]] bool attachToName(IntegerPrimaryKey nameId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromName(IntegerPrimaryKey nameId, IntegerPrimaryKey mediaId) const;

    [[nodiscard]] bool attachToEvent(IntegerPrimaryKey eventId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromEvent(IntegerPrimaryKey eventId, IntegerPrimaryKey mediaId) const;

    [[nodiscard]] bool attachToEventRelation(IntegerPrimaryKey relationId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromEventRelation(IntegerPrimaryKey relationId, IntegerPrimaryKey mediaId) const;

    [[nodiscard]] bool attachToSource(IntegerPrimaryKey sourceId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromSource(IntegerPrimaryKey sourceId, IntegerPrimaryKey mediaId) const;

    [[nodiscard]] bool attachToLocation(IntegerPrimaryKey locationId, IntegerPrimaryKey mediaId) const;
    [[nodiscard]] bool detachFromLocation(IntegerPrimaryKey locationId, IntegerPrimaryKey mediaId) const;
};
