/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "data_event_broker.h"
#include "database/schema.h"

#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariantMap>

namespace QueryHelper {

struct QueryCriteria {
    QString filterText;
    QString filterColumn;
    QString sortColumn;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    int limit = -1;
    int offset = 0;
    QVariantMap filters;
};

class SqlQueryBuilder {
    QString baseSql;
    QString tableName;
    QStringList selectColumns;
    QStringList whereClauses;
    QVariantMap bindings;
    QString orderByClause;
    int limitCount = -1;
    int offsetCount = -1;

public:
    SqlQueryBuilder() = default;

    explicit SqlQueryBuilder(const QString& sql) : baseSql(sql) {
    }

    SqlQueryBuilder& from(const QString& table);

    SqlQueryBuilder& select(const QStringList& columns);

    SqlQueryBuilder& where(const QString& condition);

    SqlQueryBuilder& bind(const QString& placeholder, const QVariant& value);

    SqlQueryBuilder& orderBy(const QString& column, Qt::SortOrder order);

    SqlQueryBuilder& limit(int limit);
    SqlQueryBuilder& offset(int offset);

    SqlQueryBuilder& applyCriteria(const QueryCriteria& criteria);

    std::tuple<QString, QVariantMap> construct() const;
};

/**
 * Execute a SQL query with bindings and return the query and result status.
 *
 * @param sql The SQL query to execute
 * @param bindings Query parameter bindings
 *
 * @return Tuple containing the executed query and result status
 */
[[nodiscard]] std::tuple<QSqlQuery, bool> executeWithResult(const QString& sql, const QVariantMap& bindings = {});

/**
 * Execute a query and return the success status.
 *
 * @param sql The SQL query to execute
 * @param bindings Optional bindings for the query parameters.
 *
 * @return True if the query executed successfully, false otherwise.
 */
[[nodiscard]] bool execute(const QString& sql, const QVariantMap& bindings = {});

/**
 * Execute a query (nominally an insert) and return the primary key of the inserted row.
 *
 * @param sql The SQL insert query to execute
 * @param bindings Optional bindings for the query parameters.
 *
 * @return The primary key of the inserted row, or std::nullopt if the query failed.
 */
[[nodiscard]] std::optional<IntegerPrimaryKey> insert(const QString& sql, const QVariantMap& bindings = {});

/**
 * Execute a query and notify the event broker if the query was successful.
 *
 * @tparam EventType The type of event to notify (e.g. which table)
 * @param id The primary key of the affected row
 * @param sql The SQL query to execute
 * @param bindings Optional bindings for the query parameters
 * @return True if the query was successful, false otherwise
 */
template<typename EventType>
[[nodiscard]] bool executeAndNotify(IntegerPrimaryKey id, const QString& sql, const QVariantMap& bindings = {}) {
    auto result = execute(sql, bindings);
    if (result) {
        DataEventBroker::instance().notifyChanged<EventType>(id);
    }
    return result;
}

template<typename T>
using QueryExtractor = std::function<T(const QSqlQuery&)>;

/**
 * Execute a query and get all results.
 *
 * @tparam T The type of the result objects.
 * @param sql The SQL query to execute.
 * @param bindings Optional bindings for the query parameters.
 *
 * @return A list of result objects.
 */
template<typename T>
[[nodiscard]] QList<T> fetchAll(const QString& sql, const QVariantMap& bindings, const QueryExtractor<T>& extractor) {
    auto [query, result] = executeWithResult(sql, bindings);

    QList<T> results;
    if (!result) {
        return results;
    }

    if (query.driver()->hasFeature(QSqlDriver::QuerySize)) {
        results.reserve(query.size());
    }

    while (query.next()) {
        results << extractor(query);
    }

    return results;
}

template<typename T>
[[nodiscard]] QList<T> fetchAll(const QString& sql, const QVariantMap& bindings = {}) {
    return fetchAll<T>(sql, bindings, &T::fromSql);
}

/**
 * Execute a query and get one result.
 *
 * This is a strict function: if there is more than one result, we will error.
 *
 * @tparam T The type of the result objects.
 * @param sql The SQL query to execute.
 * @param bindings Optional bindings for the query parameters.
 *
 * @return The resulting object.
 */
template<typename T>
[[nodiscard]] std::optional<T>
fetchOne(const QString& sql, const QVariantMap& bindings, const QueryExtractor<T>& extractor) {
    auto [query, result] = QueryHelper::executeWithResult(sql, bindings);

    if (!result) {
        return std::nullopt;
    }

    if (query.next()) {
        auto value = extractor(query);

        // Be strict about this, to prevent accidental errors or wrong assumptions.
        if (query.next()) {
            qCritical() << "More than one result returned for fetchOne query, refusing to return.";
            return std::nullopt;
        }

        return value;
    }

    return std::nullopt;
}

template<typename T>
[[nodiscard]] std::optional<T> fetchOne(const QString& sql, const QVariantMap& bindings = {}) {
    return fetchOne<T>(sql, bindings, &T::fromSql);
}

}
