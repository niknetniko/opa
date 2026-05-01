/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>

struct SourceTypeEntity {
    IntegerPrimaryKey id = -1;
    QString type;
    bool builtin = false;

    static SourceTypeEntity fromSql(const QSqlQuery& query) {
        using namespace Qt::StringLiterals;
        return {
            .id = query.value(u"id").toLongLong(),
            .type = query.value(u"type").toString(),
            .builtin = query.value(u"builtin").toBool(),
        };
    }
};
