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

struct NameEntity {
    IntegerPrimaryKey id = -1;
    IntegerPrimaryKey personId = -1;
    int sort = 0;
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;
    QString note;
    QString origin;

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
            .origin = query.value(u"origin").toString(),
        };
    }
};
