/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

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

    static EventEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
            .note = query.value(u"note").toString(),
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
    IntegerPrimaryKey roleId = -1;
    QString role;
    QString type;
    QString date;
    QString name;

    static PersonEventEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .roleId = query.value(u"role_id").toLongLong(),
            .role = query.value(u"role").toString(),
            .type = query.value(u"type").toString(),
            .date = query.value(u"date").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};

struct EventRelationEntity {
    IntegerPrimaryKey eventId = -1;
    IntegerPrimaryKey personId = -1;
    IntegerPrimaryKey roleId = -1;

    static EventRelationEntity fromSql(const QSqlQuery& query) {
        return {
            .eventId = query.value(u"event_id").toLongLong(),
            .personId = query.value(u"person_id").toLongLong(),
            .roleId = query.value(u"role_id").toLongLong(),
        };
    }
};
