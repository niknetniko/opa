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
    const auto sql = u"SELECT id, title, type_id, author, publication, confidence, note, parent_id "
                     u"FROM sources ORDER BY id ASC"_s;
    return fetchAll<SourceEntity>(sql, {});
}

std::optional<SourceEntity> SourceRepository::findById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, title, type_id, author, publication, confidence, note, parent_id "
                     u"FROM sources WHERE id = :id"_s;
    return fetchOne<SourceEntity>(sql, {{u":id"_s, id}});
}

QList<SourceEntity> SourceRepository::findByTitleContaining(const QString& searchTerm) const {
    const auto sql = u"SELECT id, title, type_id, author, publication, confidence, note, parent_id "
                     u"FROM sources WHERE title LIKE :term COLLATE NOCASE ORDER BY title ASC"_s;
    return fetchAll<SourceEntity>(sql, {{u":term"_s, u"%%1%"_s.arg(searchTerm)}});
}

std::optional<IntegerPrimaryKey> SourceRepository::insert(
    const QString& title,
    std::optional<IntegerPrimaryKey> typeId,
    const QString& author,
    const QString& publication,
    const QString& confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = u"INSERT INTO sources (title, type_id, author, publication, confidence, note, parent_id) "
                     u"VALUES (:title, :type_id, :author, :publication, :confidence, :note, :parent_id)"_s;
    auto bindings = QVariantMap{
        {u":title"_s, title},
        {u":author"_s, author},
        {u":publication"_s, publication},
        {u":confidence"_s, confidence},
        {u":note"_s, note},
    };
    if (typeId.has_value()) {
        bindings[u":type_id"_s] = typeId.value();
    } else {
        bindings[u":type_id"_s] = QVariant(QMetaType::fromType<IntegerPrimaryKey>());
    }
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
    std::optional<IntegerPrimaryKey> typeId,
    const QString& author,
    const QString& publication,
    const QString& confidence,
    const QString& note,
    std::optional<IntegerPrimaryKey> parentId
) const {
    const auto sql = u"UPDATE sources SET title = :title, type_id = :type_id, author = :author, "
                     u"publication = :publication, confidence = :confidence, note = :note, "
                     u"parent_id = :parent_id WHERE id = :id"_s;
    auto bindings = QVariantMap{
        {u":title"_s, title},
        {u":author"_s, author},
        {u":publication"_s, publication},
        {u":confidence"_s, confidence},
        {u":note"_s, note},
        {u":id"_s, id},
    };
    if (typeId.has_value()) {
        bindings[u":type_id"_s] = typeId.value();
    } else {
        bindings[u":type_id"_s] = QVariant(QMetaType::fromType<IntegerPrimaryKey>());
    }
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

QList<SourceTypeEntity> SourceRepository::findAllSourceTypes() const {
    return fetchAll<SourceTypeEntity>(
        u"SELECT id, type, builtin FROM source_types ORDER BY builtin DESC, type ASC"_s,
        {}
    );
}

std::optional<SourceTypeEntity> SourceRepository::findSourceTypeById(IntegerPrimaryKey id) const {
    return fetchOne<SourceTypeEntity>(u"SELECT id, type, builtin FROM source_types WHERE id = :id"_s, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> SourceRepository::insertSourceType(const QString& type) const {
    auto newId =
        QueryHelper::insert(u"INSERT INTO source_types (type, builtin) VALUES (:type, FALSE)"_s, {{u":type"_s, type}});
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::SourceTypes>(*newId);
    }
    return newId;
}

bool SourceRepository::updateSourceType(IntegerPrimaryKey id, const QString& type) const {
    return QueryHelper::executeAndNotify<Schema::SourceTypes>(
        id,
        u"UPDATE source_types SET type = :type WHERE id = :id AND builtin = FALSE"_s,
        {{u":type"_s, type}, {u":id"_s, id}}
    );
}

bool SourceRepository::deleteSourceType(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::SourceTypes>(
        id,
        u"DELETE FROM source_types WHERE id = :id AND builtin = FALSE"_s,
        {{u":id"_s, id}}
    );
}

bool SourceRepository::isSourceTypeUsed(IntegerPrimaryKey typeId) const {
    struct CountResult {
        int count = 0;
        static CountResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.count = q.value(0).toInt()};
        }
    };
    auto result =
        fetchOne<CountResult>(u"SELECT COUNT(*) FROM sources WHERE type_id = :type_id"_s, {{u":type_id"_s, typeId}});
    return result && result->count > 0;
}
