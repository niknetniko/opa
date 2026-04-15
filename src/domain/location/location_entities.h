/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/query_utils.h"
#include "database/schema.h"

#include <QSqlQuery>
#include <QString>
#include <optional>

struct LocationTypeEntity {
    IntegerPrimaryKey id = -1;
    QString type;
    bool builtin = false;

    static LocationTypeEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .type = query.value(u"type").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};

struct Coordinates {
    double latitude = 0.0;
    double longitude = 0.0;
};

struct LocationEntity {
    IntegerPrimaryKey id = -1;
    QString name;
    std::optional<IntegerPrimaryKey> typeId;
    std::optional<IntegerPrimaryKey> parentId;
    QString note;
    std::optional<Coordinates> coordinates;
    QString dateStart;
    QString dateEnd;

    static LocationEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        const auto lat = validOrOptional<double>(query.value(u"latitude"));
        const auto lon = validOrOptional<double>(query.value(u"longitude"));
        std::optional<Coordinates> coords;
        if (lat.has_value() && lon.has_value()) {
            coords = Coordinates{*lat, *lon};
        }
        return {
            .id = query.value(u"id").toLongLong(),
            .name = query.value(u"name").toString(),
            .typeId = validOrOptional<IntegerPrimaryKey>(query.value(u"type_id")),
            .parentId = validOrOptional<IntegerPrimaryKey>(query.value(u"parent_id")),
            .note = query.value(u"note").toString(),
            .coordinates = coords,
            .dateStart = query.value(u"date_start").toString(),
            .dateEnd = query.value(u"date_end").toString(),
        };
    }
};

// Used in comboboxes — full ancestral path resolved via recursive CTE.
struct LocationDisplayEntity {
    IntegerPrimaryKey id = -1;
    QString name;
    QString fullPath; // e.g. "Netherlands > Groningen"

    static LocationDisplayEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .name = query.value(u"name").toString(),
            .fullPath = query.value(u"full_path").toString(),
        };
    }
};
