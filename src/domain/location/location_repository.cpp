/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"
#include "dates/genealogical_date.h"

using namespace Qt::StringLiterals;

// ── Location types ────────────────────────────────────────────────────────────

QList<LocationTypeEntity> LocationRepository::findAllLocationTypes() const {
    return fetchAll<LocationTypeEntity>(u"SELECT id, type, builtin FROM location_types ORDER BY type"_s);
}

std::optional<LocationTypeEntity> LocationRepository::findLocationTypeById(IntegerPrimaryKey id) const {
    return fetchOne<LocationTypeEntity>(
        u"SELECT id, type, builtin FROM location_types WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<IntegerPrimaryKey> LocationRepository::insertLocationType(const QString& type) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO location_types (type, builtin) VALUES (:type, false)"_s,
        {{u":type"_s, type}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::LocationTypes>(newId);
    }
    return newId;
}

bool LocationRepository::updateLocationType(IntegerPrimaryKey id, const QString& type) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypes>(
        id,
        u"UPDATE location_types SET type = :type WHERE id = :id"_s,
        {{u":type"_s, type}, {u":id"_s, id}}
    );
}

bool LocationRepository::deleteLocationType(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypes>(
        id,
        u"DELETE FROM location_types WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

bool LocationRepository::isLocationTypeUsed(IntegerPrimaryKey id) const {
    auto [query, ok] =
        QueryHelper::executeWithResult(u"SELECT 1 FROM locations WHERE type_id = :id LIMIT 1"_s, {{u":id"_s, id}});
    return ok && query.next();
}

// ── Locations ─────────────────────────────────────────────────────────────────

QList<LocationEntity> LocationRepository::findAll() const {
    return fetchAll<LocationEntity>(
        u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
        u"FROM locations ORDER BY name"_s
    );
}

QList<LocationDisplayEntity> LocationRepository::findAllWithPaths() const {
    const auto sql = u"WITH RECURSIVE path(id, name, type_id, parent_id, full_path) AS ("
                     u"  SELECT id, name, type_id, parent_id, name FROM locations WHERE parent_id IS NULL"
                     u"  UNION ALL"
                     u"  SELECT l.id, l.name, l.type_id, l.parent_id, path.full_path || ' > ' || l.name"
                     u"  FROM locations l JOIN path ON l.parent_id = path.id"
                     u") SELECT id, name, full_path FROM path ORDER BY full_path"_s;
    return fetchAll<LocationDisplayEntity>(sql);
}

std::optional<LocationEntity> LocationRepository::findById(IntegerPrimaryKey id) const {
    return fetchOne<LocationEntity>(
        u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
        u"FROM locations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<IntegerPrimaryKey> LocationRepository::insert(
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO locations (name, type_id, parent_id) VALUES (:name, :type_id, :parent_id)"_s,
        {
            {u":name"_s, name},
            {u":type_id"_s, typeId.has_value() ? QVariant(*typeId) : QVariant{}},
            {u":parent_id"_s, parentId.has_value() ? QVariant(*parentId) : QVariant{}},
        }
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Locations>(newId);
    }
    return newId;
}

bool LocationRepository::update(
    IntegerPrimaryKey id,
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId,
    const QString& note,
    std::optional<Coordinates> coordinates,
    const QString& dateStart,
    const QString& dateEnd
) const {
    const auto parsedStart = GenealogicalDate::fromDatabaseRepresentation(dateStart);
    const auto parsedEnd = GenealogicalDate::fromDatabaseRepresentation(dateEnd);
    return QueryHelper::executeAndNotify<Schema::Locations>(
        id,
        u"UPDATE locations SET name = :name, type_id = :type_id, parent_id = :parent_id, "
        u"note = :note, latitude = :latitude, longitude = :longitude, "
        u"date_start = :date_start, date_start_sort = :date_start_sort, "
        u"date_end = :date_end, date_end_sort = :date_end_sort WHERE id = :id"_s,
        {
            {u":name"_s, name},
            {u":type_id"_s, typeId.has_value() ? QVariant(*typeId) : QVariant{}},
            {u":parent_id"_s, parentId.has_value() ? QVariant(*parentId) : QVariant{}},
            {u":note"_s, note},
            {u":latitude"_s, coordinates.has_value() ? QVariant(coordinates->latitude) : QVariant{}},
            {u":longitude"_s, coordinates.has_value() ? QVariant(coordinates->longitude) : QVariant{}},
            {u":date_start"_s, dateStart},
            {u":date_start_sort"_s, parsedStart.isNull() ? QVariant{} : QVariant{parsedStart.sortKey()}},
            {u":date_end"_s, dateEnd},
            {u":date_end_sort"_s, parsedEnd.isNull() ? QVariant{} : QVariant{parsedEnd.sortKey()}},
            {u":id"_s, id},
        }
    );
}

bool LocationRepository::deleteLocation(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::Locations>(
        id,
        u"DELETE FROM locations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

bool LocationRepository::isUsed(IntegerPrimaryKey id) const {
    auto [query, ok] =
        QueryHelper::executeWithResult(u"SELECT 1 FROM events WHERE location_id = :id LIMIT 1"_s, {{u":id"_s, id}});
    return ok && query.next();
}

bool LocationRepository::hasChildren(IntegerPrimaryKey id) const {
    auto [query, ok] =
        QueryHelper::executeWithResult(u"SELECT 1 FROM locations WHERE parent_id = :id LIMIT 1"_s, {{u":id"_s, id}});
    return ok && query.next();
}

std::optional<IntegerPrimaryKey> LocationRepository::findOrCreate(
    const QString& name,
    std::optional<IntegerPrimaryKey> typeId,
    std::optional<IntegerPrimaryKey> parentId
) const {
    // Matches on (name, parent_id) only; typeId is intentionally ignored for deduplication.
    // Two locations with the same name and parent are considered the same regardless of type.
    std::optional<LocationEntity> existing;
    if (parentId.has_value()) {
        existing = fetchOne<LocationEntity>(
            u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
            u"FROM locations WHERE name = :name AND parent_id = :parent_id"_s,
            {{u":name"_s, name}, {u":parent_id"_s, *parentId}}
        );
    } else {
        existing = fetchOne<LocationEntity>(
            u"SELECT id, name, type_id, parent_id, note, latitude, longitude, date_start, date_end "
            u"FROM locations WHERE name = :name AND parent_id IS NULL"_s,
            {{u":name"_s, name}}
        );
    }
    if (existing.has_value()) {
        return existing->id;
    }
    return insert(name, typeId, parentId);
}
