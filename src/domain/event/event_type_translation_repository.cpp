/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_type_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<EventTypeTranslationEntity> EventTypeTranslationRepository::findAllForType(IntegerPrimaryKey typeId) const {
    return fetchAll<EventTypeTranslationEntity>(
        u"SELECT id, type_id, locale, name FROM event_type_translations WHERE type_id = :type_id ORDER BY locale"_s,
        {{u":type_id"_s, typeId}}
    );
}

std::optional<IntegerPrimaryKey>
EventTypeTranslationRepository::insert(IntegerPrimaryKey typeId, const QString& locale, const QString& name) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO event_type_translations (type_id, locale, name) VALUES (:type_id, :locale, :name)"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventTypeTranslations>(newId);
    }
    return newId;
}

bool EventTypeTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::EventTypeTranslations>(
        id,
        u"DELETE FROM event_type_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString>
EventTypeTranslationRepository::findByTypeIdAndLocale(IntegerPrimaryKey typeId, const QString& locale) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT name FROM event_type_translations WHERE type_id = :type_id AND locale = :locale"_s,
        {{u":type_id"_s, typeId}, {u":locale"_s, locale}}
    );
    if (result) {
        return result->name;
    }
    return std::nullopt;
}

std::optional<QString>
EventTypeTranslationRepository::findByTypeStringAndLocale(const QString& typeString, const QString& locale) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT ett.name FROM event_type_translations ett "
        u"JOIN event_types et ON et.id = ett.type_id "
        u"WHERE et.type = :type AND ett.locale = :locale"_s,
        {{u":type"_s, typeString}, {u":locale"_s, locale}}
    );
    if (result) {
        return result->name;
    }
    return std::nullopt;
}
