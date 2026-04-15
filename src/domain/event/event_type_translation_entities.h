/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

struct EventTypeTranslationEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey typeId = -1;
    QString locale;
    QString name;

    static EventTypeTranslationEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .typeId = query.value(u"type_id").toLongLong(),
            .locale = query.value(u"locale").toString(),
            .name = query.value(u"name").toString(),
        };
    }
};
