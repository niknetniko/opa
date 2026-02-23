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

struct PersonEntity {
    IntegerPrimaryKey id = -1;
    bool root = false;
    QString sex;

    static PersonEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id"_s).toLongLong(),
            .root = query.value(u"root"_s).toBool(),
            .sex = query.value(u"sex"_s).toString()
        };
    }
};

struct PersonDisplayEntity: PersonEntity {
    QString titles;
    QString givenNames;
    QString prefix;
    QString surname;

    static PersonDisplayEntity fromSql(const QSqlQuery& query) {
        PersonDisplayEntity p;

        static_cast<PersonEntity&>(p) = PersonEntity::fromSql(query);

        p.titles = query.value(u"titles"_s).toString();
        p.givenNames = query.value(u"given_names"_s).toString();
        p.prefix = query.value(u"prefix"_s).toString();
        p.surname = query.value(u"surname"_s).toString();

        return p;
    }
};
