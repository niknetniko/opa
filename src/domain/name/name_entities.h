/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/query_utils.h"
#include "database/schema.h"

#include <QSqlQuery>
#include <QString>
#include <optional>

using namespace Qt::StringLiterals;

struct NameEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey personId = -1;
    int sort = 0;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;
    QString note;
    std::optional<IntegerPrimaryKey> originId;

    static NameEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .personId = query.value(u"person_id").toLongLong(),
            .sort = query.value(u"sort").toInt(),
            .titles = query.value(u"titles").toString(),
            .givenNames = query.value(u"given_names").toString(),
            .prefix = query.value(u"prefix").toString(),
            .surname = query.value(u"surname").toString(),
            .note = query.value(u"note").toString(),
            .originId = validOrOptional<IntegerPrimaryKey>(query.value(u"origin_id")),
        };
    }
};

struct NameWithOriginEntity : NameEntity {
    QString origin;

    static NameWithOriginEntity fromSql(const QSqlQuery& query) {
        NameWithOriginEntity n;

        // C++ here we go
        static_cast<NameEntity&>(n) = NameEntity::fromSql(query);

        n.origin = query.value(u"origin").toString();

        return n;
    }
};

struct NameOriginEntity {
    IntegerPrimaryKey id = -1;
    QString origin;
    bool builtin = false;

    static NameOriginEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .origin = query.value(u"origin").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};
