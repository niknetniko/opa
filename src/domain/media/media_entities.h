/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QSqlQuery>
#include <QString>
#include <optional>

using namespace Qt::StringLiterals;

struct MediaEntity {
    IntegerPrimaryKey id = -1;
    QString path;
    std::optional<QString> title;
    std::optional<QString> note;
    QString mimeType;

    static MediaEntity fromSql(const QSqlQuery& query) {
        auto title = query.value(u"title");
        auto note = query.value(u"note");
        return {
            .id = query.value(u"id").toLongLong(),
            .path = query.value(u"path").toString(),
            .title = title.isNull() ? std::nullopt : std::make_optional(title.toString()),
            .note = note.isNull() ? std::nullopt : std::make_optional(note.toString()),
            .mimeType = query.value(u"mime_type").toString(),
        };
    }
};
