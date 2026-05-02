/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "data_event_broker.h"
#include "query_helper.h"

#include <QList>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariantMap>
#include <optional>

/**
 * Provides a generic implementation of database access methods for derived repository classes.
 *
 * This class defines methods to facilitate commonly used database operations, such as fetching data,
 * executing queries, and inserting records. It is intended to be inherited by specific repository classes
 * that implement the data access logic for corresponding entities.
 */
class BaseRepository {
protected:
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
    [[nodiscard]] QList<T> fetchAll(const QString& sql, const QVariantMap& bindings = {}) const {
        auto [query, result] = QueryHelper::executeWithResult(sql, bindings);

        QList<T> results;
        if (!result) {
            return results;
        }

        if (query.driver()->hasFeature(QSqlDriver::QuerySize)) {
            results.reserve(query.size());
        }

        while (query.next()) {
            results << T::fromSql(query);
        }

        return results;
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
    [[nodiscard]] std::optional<T> fetchOne(const QString& sql, const QVariantMap& bindings = {}) const {
        auto [query, result] = QueryHelper::executeWithResult(sql, bindings);

        if (!result) {
            return std::nullopt;
        }

        if (query.next()) {
            auto value = T::fromSql(query);

            // Be strict about this, to prevent accidental errors or wrong assumptions.
            if (query.next()) {
                qCritical() << "More than one result returned for fetchOne query, refusing to return.";
                return std::nullopt;
            }

            return value;
        }

        return std::nullopt;
    }
};
