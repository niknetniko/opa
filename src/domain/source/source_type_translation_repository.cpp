/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_type_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<SourceTypeTranslationEntity> SourceTypeTranslationRepository::findAllForType(IntegerPrimaryKey typeId) const {
    return fetchAll<SourceTypeTranslationEntity>(
        u"SELECT id, type_id, locale, name FROM source_type_translations WHERE type_id = :type_id ORDER BY locale"_s,
        {{u":type_id"_s, typeId}}
    );
}

std::optional<IntegerPrimaryKey> SourceTypeTranslationRepository::insert(
    IntegerPrimaryKey typeId, const QString& locale, const QString& name
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO source_type_translations (type_id, locale, name) VALUES (:type_id, :locale, :name)"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::SourceTypeTranslations>(*newId);
    }
    return newId;
}

bool SourceTypeTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::SourceTypeTranslations>(
        id,
        u"DELETE FROM source_type_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString> SourceTypeTranslationRepository::findByTypeIdAndLocale(
    IntegerPrimaryKey typeId, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT name FROM source_type_translations WHERE type_id = :type_id AND locale = :locale"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}

std::optional<QString> SourceTypeTranslationRepository::findByTypeStringAndLocale(
    const QString& typeString, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT stt.name FROM source_type_translations stt "
        u"JOIN source_types st ON st.id = stt.type_id "
        u"WHERE st.type = :type AND stt.locale = :locale"_s,
        {{u":type"_s, typeString}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}
