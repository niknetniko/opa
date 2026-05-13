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

struct EventTypeEntity {
    IntegerPrimaryKey id = -1;
    QString type;
    bool builtin = false;

    static EventTypeEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .type = query.value(u"type").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};

struct EventRoleEntity {
    IntegerPrimaryKey id = -1;
    QString role;
    bool builtin = false;

    static EventRoleEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .role = query.value(u"role").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};

struct EventEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString date;
    QString name;
    QString note;
    std::optional<IntegerPrimaryKey> locationId;
    std::optional<IntegerPrimaryKey> familyId;

    static EventEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
            .note = query.value(u"note").toString(),
            .locationId = validOrOptional<IntegerPrimaryKey>(query.value(u"location_id")),
            .familyId = validOrOptional<IntegerPrimaryKey>(query.value(u"family_id")),
        };
    }
};

struct EventDisplayEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString type;
    QString date;
    QString name;

    static EventDisplayEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .type = query.value(u"type").toString(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};

struct PersonEventEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey relationId = -1;
    IntegerPrimaryKey roleId = -1;
    QString role;
    QString type;
    QString date;
    QString name;

    static PersonEventEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .relationId = query.value(u"relation_id").toLongLong(),
            .roleId = query.value(u"role_id").toLongLong(),
            .role = query.value(u"role").toString(),
            .type = query.value(u"type").toString(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};

struct EventRelationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey eventId = -1;
    IntegerPrimaryKey personId = -1;
    IntegerPrimaryKey roleId = -1;

    static EventRelationEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .eventId = query.value(u"event_id").toLongLong(),
            .personId = query.value(u"person_id").toLongLong(),
            .roleId = query.value(u"role_id").toLongLong(),
        };
    }
};
