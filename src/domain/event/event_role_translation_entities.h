/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

struct EventRoleTranslationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey roleId = -1;
    QString locale;
    QString name;

    static EventRoleTranslationEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .roleId = query.value(u"role_id").toLongLong(),
            .locale = query.value(u"locale").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};
