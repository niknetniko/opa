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

struct SourceEntity {
    IntegerPrimaryKey id = -1;
    QString title;
    QString type;
    QString author;
    QString publication;
    QString confidence;
    QString note;
    std::optional<IntegerPrimaryKey> parentId;

    static SourceEntity fromSql(const QSqlQuery& query) {
        return {
            .id = query.value(u"id").toLongLong(),
            .title = query.value(u"title").toString(),
            .type = query.value(u"type").toString(),
            .author = query.value(u"author").toString(),
            .publication = query.value(u"publication").toString(),
            .confidence = query.value(u"confidence").toString(),
            .note = query.value(u"note").toString(),
            .parentId = validOrOptional<IntegerPrimaryKey>(query.value(u"parent_id")),
        };
    }
};
