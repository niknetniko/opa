/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "location_type_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<LocationTypeTranslationEntity> LocationTypeTranslationRepository::findAllForType(IntegerPrimaryKey typeId) const {
    return fetchAll<LocationTypeTranslationEntity>(
        u"SELECT id, type_id, locale, name FROM location_type_translations WHERE type_id = :type_id ORDER BY locale"_s,
        {{u":type_id"_s, typeId}}
    );
}

std::optional<IntegerPrimaryKey> LocationTypeTranslationRepository::insert(
    IntegerPrimaryKey typeId, const QString& locale, const QString& name
) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO location_type_translations (type_id, locale, name) VALUES (:type_id, :locale, :name)"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::LocationTypeTranslations>(*newId);
    }
    return newId;
}

bool LocationTypeTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::LocationTypeTranslations>(
        id,
        u"DELETE FROM location_type_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString> LocationTypeTranslationRepository::findByTypeIdAndLocale(
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
        u"SELECT name FROM location_type_translations WHERE type_id = :type_id AND locale = :locale"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}

std::optional<QString> LocationTypeTranslationRepository::findByTypeStringAndLocale(
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
        u"SELECT ltt.name FROM location_type_translations ltt "
        u"JOIN location_types lt ON lt.id = ltt.type_id "
        u"WHERE lt.type = :type AND ltt.locale = :locale"_s,
        {{u":type"_s, typeString}, {u":locale"_s, locale}}
    );
    if (result) return result->name;
    return std::nullopt;
}
