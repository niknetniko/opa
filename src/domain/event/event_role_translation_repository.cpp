/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "event_role_translation_repository.h"

#include "core/data_event_broker.h"
#include "core/query_helper.h"

using namespace Qt::StringLiterals;

QList<EventRoleTranslationEntity> EventRoleTranslationRepository::findAllForType(IntegerPrimaryKey roleId) const {
    return fetchAll<EventRoleTranslationEntity>(
        u"SELECT id, role_id, locale, name FROM event_role_translations WHERE role_id = :role_id ORDER BY locale"_s,
        {{u":role_id"_s, roleId}}
    );
}

std::optional<IntegerPrimaryKey>
EventRoleTranslationRepository::insert(IntegerPrimaryKey roleId, const QString& locale, const QString& name) const {
    auto newId = QueryHelper::insert(
        u"INSERT INTO event_role_translations (role_id, locale, name) VALUES (:role_id, :locale, :name)"_s,
        {{u":role_id"_s, roleId}, {u":locale"_s, locale}, {u":name"_s, name}}
    );
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::EventRoleTranslations>(newId);
    }
    return newId;
}

bool EventRoleTranslationRepository::remove(IntegerPrimaryKey id) const {
    return QueryHelper::executeAndNotify<Schema::EventRoleTranslations>(
        id,
        u"DELETE FROM event_role_translations WHERE id = :id"_s,
        {{u":id"_s, id}}
    );
}

std::optional<QString>
EventRoleTranslationRepository::findByTypeIdAndLocale(IntegerPrimaryKey roleId, const QString& locale) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT name FROM event_role_translations WHERE role_id = :role_id AND locale = :locale"_s,
        {{u":role_id"_s, roleId}, {u":locale"_s, locale}}
    );
    if (result) {
        return result->name;
    }
    return std::nullopt;
}

std::optional<QString>
EventRoleTranslationRepository::findByTypeStringAndLocale(const QString& roleString, const QString& locale) const {
    struct StringResult {
        QString name;
        static StringResult fromSql(const QSqlQuery& q) {
            using namespace Qt::StringLiterals;
            return {.name = q.value(u"name").toString()};
        }
    };
    auto result = fetchOne<StringResult>(
        u"SELECT ert.name FROM event_role_translations ert "
        u"JOIN event_roles er ON er.id = ert.role_id "
        u"WHERE er.role = :role AND ert.locale = :locale"_s,
        {{u":role"_s, roleString}, {u":locale"_s, locale}}
    );
    if (result) {
        return result->name;
    }
    return std::nullopt;
}
