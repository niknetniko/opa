/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./person_repository.h"

#include "../../core/data_event_broker.h"

using namespace Qt::StringLiterals;

static const auto PRIMARY_NAME_JOIN = QStringLiteral(
    "SELECT p.id, p.root, p.sex, n.titles, n.given_names, n.prefix, n.surname "
    "FROM people p "
    "LEFT JOIN names n ON p.id = n.person_id "
    "AND n.sort = (SELECT MIN(n2.sort) FROM names n2 WHERE n2.person_id = p.id)"
);

QList<PersonEntity> PersonRepository::findPeople(const PersonCriteria& criteria) const {
    QueryHelper::SqlQueryBuilder builder;

    builder.from(Schema::People::table).select({u"id"_s, u"root"_s, u"sex"_s});

    if (criteria.rootOnly.has_value() && *criteria.rootOnly) {
        builder.where(u"root = 1"_s);
    }
    if (criteria.sex.has_value()) {
        builder.where(u"sex = :sex"_s);
        builder.bind(u":sex"_s, criteria.sex.value());
    }

    builder.applyCriteria(criteria);

    auto [sql, bindings] = builder.construct();
    return fetchAll<PersonEntity>(sql, bindings);
}

std::optional<PersonEntity> PersonRepository::findById(IntegerPrimaryKey id) const {
    const auto sql = u"SELECT id, root, sex FROM people WHERE id = :id"_s;
    return fetchOne<PersonEntity>(sql, {{u":id"_s, id}});
}

QList<PersonDisplayEntity> PersonRepository::findPeopleWithPrimaryName(const PersonCriteria& criteria) const {
    QueryHelper::SqlQueryBuilder builder{PRIMARY_NAME_JOIN};

    if (criteria.rootOnly.has_value() && *criteria.rootOnly) {
        builder.where(u"p.root = 1"_s);
    }
    if (criteria.sex.has_value()) {
        builder.where(u"p.sex = :sex"_s);
        builder.bind(u":sex"_s, criteria.sex.value());
    }

    builder.applyCriteria(criteria);

    auto [sql, bindings] = builder.construct();
    return fetchAll<PersonDisplayEntity>(sql, bindings);
}

std::optional<PersonDisplayEntity> PersonRepository::findDisplayById(IntegerPrimaryKey id) const {
    const QString sql = PRIMARY_NAME_JOIN + u" WHERE p.id = :id"_s;
    return fetchOne<PersonDisplayEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> PersonRepository::insertPerson(const QString& sex, bool root) const {
    const auto sql = u"INSERT INTO people (root, sex) VALUES (:root, :sex)"_s;
    const QVariantMap bindings = {
        {u":root"_s, root},
        {u":sex"_s, sex},
    };
    const auto newId = QueryHelper::insert(sql, bindings);
    if (newId.has_value()) {
        DataEventBroker::instance().notifyChanged<Schema::People>(newId.value());
    }
    return newId;
}

bool PersonRepository::updatePerson(IntegerPrimaryKey id, const QString& sex, bool root) const {
    const auto sql = u"UPDATE people SET root = :root, sex = :sex WHERE id = :id"_s;
    const QVariantMap bindings = {
        {u":root"_s, root},
        {u":sex"_s, sex},
        {u":id"_s, id},
    };
    const bool ok = QueryHelper::execute(sql, bindings);
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::People>(id);
    }
    return ok;
}

bool PersonRepository::deletePerson(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM people WHERE id = :id"_s;
    const bool ok = QueryHelper::execute(sql, {{u":id"_s, id}});
    if (ok) {
        DataEventBroker::instance().notifyChanged<Schema::People>(id);
    }
    return ok;
}
