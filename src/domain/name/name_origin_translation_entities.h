/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

struct NameOriginTranslationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey originId = -1;
    QString locale;
    QString name;

    static NameOriginTranslationEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .originId = query.value(u"origin_id").toLongLong(),
            .locale = query.value(u"locale").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};
