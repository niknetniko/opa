/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "./name_repository.h"

#include "../../core/data_event_broker.h"
#include "names.h"

using namespace Qt::StringLiterals;

QStringList NameRepository::findAllSurnames() const {
    // language=sqlite
    const auto sql = QStringLiteral(
        "SELECT DISTINCT surname FROM names WHERE surname IS NOT NULL AND surname != '' ORDER BY surname"
    );
    QStringList result;
    for (const auto& entity: fetchAll<NameEntity>(sql, {})) {
        result.append(entity.surname);
    }
    return result;
}

QStringList NameRepository::findAllGivenNames() const {
    // language=sqlite
    const auto sql = QStringLiteral(
        "SELECT DISTINCT given_names FROM names WHERE given_names IS NOT NULL AND given_names != '' ORDER BY "
        "given_names"
    );
    QStringList result;
    for (const auto& entity: fetchAll<NameEntity>(sql, {})) {
        result.append(entity.givenNames);
    }
    return result;
}

QList<NameEntity> NameRepository::findNamesForPerson(IntegerPrimaryKey personId) const {
    const auto sql = QStringLiteral(
        "SELECT id, person_id, sort, titles, given_names, prefix, surname, note, origin "
        "FROM names WHERE person_id = :person_id ORDER BY sort ASC"
    );
    return fetchAll<NameEntity>(sql, {{u":person_id"_s, personId}});
}

std::optional<NameEntity> NameRepository::findById(IntegerPrimaryKey id) const {
    const auto sql = QStringLiteral(
        "SELECT id, person_id, sort, titles, given_names, prefix, surname, note, origin "
        "FROM names WHERE id = :id"
    );
    return fetchOne<NameEntity>(sql, {{u":id"_s, id}});
}

std::optional<IntegerPrimaryKey> NameRepository::insertName(IntegerPrimaryKey personId, int sort) const {
    const auto sql = u"INSERT INTO names (person_id, sort) VALUES (:person_id, :sort)"_s;
    const auto bindings = QVariantMap {
        {u":person_id"_s, personId},
        {u":sort"_s, sort},
    };
    const auto newId = QueryHelper::insert(sql, bindings);
    if (newId) {
        DataEventBroker::instance().notifyChanged<Schema::Names>(newId);
    }

    return newId;
}

bool NameRepository::updateName(
    IntegerPrimaryKey id,
    const QString& titles,
    const QString& givenNames,
    const QString& prefix,
    const QString& surname,
    const QString& note,
    const QString& origin
) const {
    const auto sql = QStringLiteral(
        "UPDATE names SET titles = :titles, given_names = :given_names, prefix = :prefix, "
        "surname = :surname, note = :note, origin = :origin WHERE id = :id"
    );
    auto bindings = QVariantMap {
        {u":titles"_s, titles},
        {u":given_names"_s, givenNames},
        {u":prefix"_s, prefix},
        {u":surname"_s, surname},
        {u":note"_s, note},
        {u":id"_s, id},
        {u":origin"_s, origin},
    };

    return QueryHelper::executeAndNotify<Schema::Names>(id, sql, bindings);
}

bool NameRepository::updateNameSort(IntegerPrimaryKey id, int sort) const {
    const auto sql = u"UPDATE names SET sort = :sort WHERE id = :id"_s;
    const auto bindings = QVariantMap {
        {u":sort"_s, sort},
        {u":id"_s, id},
    };
    return QueryHelper::executeAndNotify<Schema::Names>(id, sql, bindings);
}

bool NameRepository::deleteName(IntegerPrimaryKey id) const {
    const auto sql = u"DELETE FROM names WHERE id = :id"_s;
    return QueryHelper::executeAndNotify<Schema::Names>(id, sql, {{u":id"_s, id}});
}
