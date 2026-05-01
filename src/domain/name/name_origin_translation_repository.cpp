/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "name_origin_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<NameOriginTranslationEntity> NameOriginTranslationRepository::findAllForType(IntegerPrimaryKey originId) const {
    return fetchAll<NameOriginTranslationEntity>(
        u"SELECT id, origin_id, locale, name FROM name_origin_translations WHERE origin_id = :origin_id ORDER BY locale"_s,
        {{u":origin_id"_s, originId}}
    );
}

std::optional<IntegerPrimaryKey> NameOriginTranslationRepository::insert(
    IntegerPrimaryKey originId, const QString& locale, const QString& name
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO name_origin_translations (origin_id, locale, name) VALUES (:origin_id, :locale, :name)"_s,
        {{u":origin_id"_s, originId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::NameOriginTranslations>(*newId);
    }
    return newId;
}

bool NameOriginTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::NameOriginTranslations>(
        id,
        u"DELETE FROM name_origin_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString> NameOriginTranslationRepository::findByTypeIdAndLocale(
    IntegerPrimaryKey originId, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT name FROM name_origin_translations WHERE origin_id = :origin_id AND locale = :locale"_s,
        {{u":origin_id"_s, originId}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}

std::optional<QString> NameOriginTranslationRepository::findByTypeStringAndLocale(
    const QString& originString, const QString& locale
) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT not_.name FROM name_origin_translations not_ "
        u"JOIN name_origins no ON no.id = not_.origin_id "
        u"WHERE no.origin = :origin AND not_.locale = :locale"_s,
        {{u":origin"_s, originString}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}
