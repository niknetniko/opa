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

using namespace Qt::StringLiterals;

struct FamilyEntity {
    IntegerPrimaryKey id = -1;
    std::optional<QString> note;

    static FamilyEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id"_s).toLongLong(),
            .note = validOrOptional<QString>(query.value(u"note"_s)),
        };
    }
};

struct FamilyMemberEntity {
    QString eventType;
    IntegerPrimaryKey eventTypeId = -1;
    IntegerPrimaryKey personId = -1;
    std::optional<IntegerPrimaryKey> partnerId;
    std::optional<IntegerPrimaryKey> familyId;
    IntegerPrimaryKey eventId = -1;
    QString date;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;

    static FamilyMemberEntity fromSql(const QSqlQuery& query) {
        FamilyMemberEntity e;
        e.eventType = query.value(u"event_type"_s).toString();
        e.eventTypeId = query.value(u"event_type_id"_s).toLongLong();
        e.personId = query.value(u"person_id"_s).toLongLong();
        e.partnerId = validOrOptional<IntegerPrimaryKey>(query.value(u"partner_id"_s));
        e.familyId = validOrOptional<IntegerPrimaryKey>(query.value(u"family_id"_s));
        e.eventId = query.value(u"event_id"_s).toLongLong();
        e.date = query.value(u"event_date"_s).toString();
        e.titles = query.value(u"titles"_s).toString();
        e.givenNames = query.value(u"given_names"_s).toString();
        e.prefix = query.value(u"prefix"_s).toString();
        e.surname = query.value(u"surname"_s).toString();
        return e;
    }
};

struct FamilyOverviewRow {
    IntegerPrimaryKey familyId = -1;
    QString familyDisplayName;
    IntegerPrimaryKey eventId = -1;
    QString eventType;
    QString eventDate;
    IntegerPrimaryKey personId = -1;
    QString role;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;

    static FamilyOverviewRow fromSql(const QSqlQuery& query) {
        FamilyOverviewRow e;
        e.familyId = query.value(u"family_id"_s).toLongLong();
        e.familyDisplayName = query.value(u"family_display_name"_s).toString();
        e.eventId = query.value(u"event_id"_s).toLongLong();
        e.eventType = query.value(u"event_type"_s).toString();
        e.eventDate = query.value(u"event_date"_s).toString();
        e.personId = query.value(u"person_id"_s).toLongLong();
        e.role = query.value(u"role"_s).toString();
        e.titles = query.value(u"titles"_s).toString();
        e.givenNames = query.value(u"given_names"_s).toString();
        e.prefix = query.value(u"prefix"_s).toString();
        e.surname = query.value(u"surname"_s).toString();
        return e;
    }
};

struct AncestorEntity {
    IntegerPrimaryKey childId = -1;
    std::optional<IntegerPrimaryKey> fatherId;
    std::optional<IntegerPrimaryKey> motherId;
    QString visited;
    int level = 0;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;

    static AncestorEntity fromSql(const QSqlQuery& query) {
        AncestorEntity e;
        e.childId = query.value(u"child_id"_s).toLongLong();
        const auto fatherValue = query.value(u"father_id"_s);
        e.fatherId = fatherValue.isNull() ? std::nullopt : std::optional<IntegerPrimaryKey>{fatherValue.toLongLong()};
        const auto motherValue = query.value(u"mother_id"_s);
        e.motherId = motherValue.isNull() ? std::nullopt : std::optional<IntegerPrimaryKey>{motherValue.toLongLong()};
        e.visited = query.value(u"visited"_s).toString();
        e.level = query.value(u"level"_s).toInt();
        e.titles = query.value(u"titles"_s).toString();
        e.givenNames = query.value(u"given_names"_s).toString();
        e.prefix = query.value(u"prefix"_s).toString();
        e.surname = query.value(u"surname"_s).toString();
        return e;
    }
};

struct ParentEntity {
    IntegerPrimaryKey personId = -1;
    IntegerPrimaryKey roleId = -1;
    QString role;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;

    static ParentEntity fromSql(const QSqlQuery& query) {
        return {
            .personId = query.value(u"person_id"_s).toLongLong(),
            .roleId = query.value(u"role_id"_s).toLongLong(),
            .role = query.value(u"role"_s).toString(),
            .titles = query.value(u"titles"_s).toString(),
            .givenNames = query.value(u"given_names"_s).toString(),
            .prefix = query.value(u"prefix"_s).toString(),
            .surname = query.value(u"surname"_s).toString(),
        };
    }
};
