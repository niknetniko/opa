/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_repository.h"

#include "../../core/data_event_broker.h"
#include "../../core/query_helper.h"

using namespace Qt::StringLiterals;

QList<SourceEntity> SourceRepository::findAll() const {
    // language=sqlite
    const auto sql = u"SELECT id, title, type, author, publication, confidence, note, parent_id FROM sources ORDER BY id"_s;
    return fetchAll<SourceEntity>(sql, {});
}

std::optional<SourceEntity> SourceRepository::findById(IntegerPrimaryKey id) const {
    // language=sqlite
    const auto sql = u"SELECT id, title, type, author, publication, confidence, note, parent_id FROM sources WHERE id = :id"_s;
    return fetchOne<SourceEntity>(sql, {{u":id"_s, id}});
}

QList<SourceEntity> SourceRepository::findByTitleContaining(const QString& searchTerm) const {
    const auto sql = u"SELECT id, title, type, author, publication, confidence, note, parent_id "
                     u"FROM sources WHERE title LIKE :term COLLATE NOCASE ORDER BY title ASC"_s;
    return fetchAll<SourceEntity>(sql, {{u":term"_s, u"%%1%"_s.arg(searchTerm)}});
}

std::optional<IntegerPrimaryKey> SourceRepository::insert(
    const QString& title,
    const QString& type,
    const QString& author,
    const QString& publication,
    int confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = u"INSERT INTO sources (title, type, author, publication, confidence, note, parent_id) "
                     u"VALUES (:title, :type, :author, :publication, :confidence, :note, :parent_id)"_s;
    auto bindings = QVariantMap {
        {u":title"_s, title},
        {u":type"_s, type},
        {u":author"_s, author},
        {u":publication"_s, publication},
        {u":confidence"_s, confidence},
        {u":note"_s, note},
    };
    if (parentId.has_value()) {
        bindings[u":parent_id"_s] = parentId.value();
    } else {
        bindings[u":parent_id"_s] = QVariant(QMetaType::fromType<IntegerPrimaryKey>());
    }
    const auto newId = QueryHelper::insert(sql, bindings);
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Sources>(newId);
    }
    return newId;
}

bool SourceRepository::update(
    IntegerPrimaryKey id,
    const QString& title,
    const QString& type,
    const QString& author,
    const QString& publication,
    const int confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = u"UPDATE sources SET title = :title, type = :type, author = :author, "
                     u"publication = :publication, confidence = :confidence, note = :note, "
                     u"parent_id = :parent_id WHERE id = :id"_s;
    auto bindings = QVariantMap {
        {u":title"_s, title},
        {u":type"_s, type},
        {u":author"_s, author},
        {u":publication"_s, publication},
        {u":confidence"_s, confidence},
        {u":note"_s, note},
        {u":id"_s, id},
    };
    if (parentId.has_value()) {
        bindings[u":parent_id"_s] = parentId.value();
    } else {
        bindings[u":parent_id"_s] = QVariant(QMetaType::fromType<IntegerPrimaryKey>());
    }
    return QueryHelper::executeAndNotify<Schema::Sources>(id, sql, bindings);
}

bool SourceRepository::remove(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM sources WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::Sources>(id, sql, {{u":id"_s, id}});
}
