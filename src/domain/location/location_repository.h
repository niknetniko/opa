/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "database/schema.h"
#include "location_entities.h"

#include <QList>
#include <optional>

class LocationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<LocationEntity> findAll() const;
    [[nodiscard]] QList<LocationDisplayEntity> findAllWithPaths() const;
    [[nodiscard]] std::optional<LocationEntity> findById(IntegerPrimaryKey id) const;
    std::optional<IntegerPrimaryKey>
    insert(const QString& name, const QString& type, std::optional<IntegerPrimaryKey> parentId) const;
    bool update(
        IntegerPrimaryKey id,
        const QString& name,
        const QString& type,
        std::optional<IntegerPrimaryKey> parentId,
        const QString& note,
        const std::optional<Coordinates>& coordinates,
        const QString& dateStart,
        const QString& dateEnd
    ) const;
    bool deleteLocation(IntegerPrimaryKey id) const;
    [[nodiscard]] bool isUsed(IntegerPrimaryKey id) const;
    [[nodiscard]] bool hasChildren(IntegerPrimaryKey id) const;
    /// Finds an existing location by (name, parent_id) or creates one. type is ignored for matching.
    std::optional<IntegerPrimaryKey>
    findOrCreate(const QString& name, const QString& type, std::optional<IntegerPrimaryKey> parentId) const;
};
