/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_repository.h"

#include "../../core/data_event_broker.h"

using namespace Qt::StringLiterals;

QList<SourceEntity> SourceRepository::findAll() const {
    const auto sql = QStringLiteral(
        "SELECT id, title, type, author, publication, confidence, note, parent_id "
        "FROM sources ORDER BY id ASC"
    );
    return fetchAll<SourceEntity>(sql, {});
}

std::optional<SourceEntity> SourceRepository::findById(IntegerPrimaryKey id) const {
    const auto sql = QStringLiteral(
        "SELECT id, title, type, author, publication, confidence, note, parent_id "
        "FROM sources WHERE id = :id"
    );
    return fetchOne<SourceEntity>(sql, {{u":id"_s, id}});
}

QStringList SourceRepository::findAllTypes() const {
    const auto sql = QStringLiteral(
        "SELECT DISTINCT type FROM sources WHERE type IS NOT NULL AND type != '' ORDER BY type ASC"
    );
    QStringList result;
    const auto entities = fetchAll<SourceEntity>(sql, {});
    for (const auto& entity : entities) {
        result.append(entity.type);
    }
    return result;
}

std::optional<IntegerPrimaryKey> SourceRepository::insert(
    const QString& title,
    const QString& type,
    const QString& author,
    const QString& publication,
    const QString& confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = QStringLiteral(
        "INSERT INTO sources (title, type, author, publication, confidence, note, parent_id) "
        "VALUES (:title, :type, :author, :publication, :confidence, :note, :parent_id)"
    );
    auto bindings = QVariantMap{
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
    const QString& confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = QStringLiteral(
        "UPDATE sources SET title = :title, type = :type, author = :author, "
        "publication = :publication, confidence = :confidence, note = :note, "
        "parent_id = :parent_id WHERE id = :id"
    );
    auto bindings = QVariantMap{
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
