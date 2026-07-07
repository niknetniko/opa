/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/data_event_broker.h"

#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(OPA_SQL);

/**
 * Open the database, or error.
 */
void openDatabase(const QString& file, bool seed = true, bool initialise = true);

void closeDatabase();

/**
 * Apply any pending migrations to the database.
 * Reads the current schema version from PRAGMA user_version and runs each
 * migration whose version number exceeds it, in order.
 */
void runMigrations(QSqlDatabase& database);

/**
 * Return true if the database is currently in a transaction, false otherwise.
 */
bool hasActiveTransaction(const QSqlDatabase& database);

/**
 * Execute a lambda in the context of a database-level transaction.
 *
 * In most cases, you should use executeInTransaction() instead of this function.
 * It uses the standard connection and has support for the data event broker.
 *
 * Supports nesting: if a transaction is already active, the operation runs
 * directly without starting a new transaction. The outermost call handles
 * commit/rollback.
 *
 * @param database The database to use.
 * @param operation Returns std::nullopt if it should abort, a value otherwise.
 *
 * @return The result of the operation, or std::nullopt on failure.
 */
template<typename Function>
auto rawExecuteInTransaction(QSqlDatabase& database, const Function& operation) -> decltype(operation()) {
    using ReturnType = decltype(operation());

    if (hasActiveTransaction(database)) {
        return operation();
    }

    if (!database.transaction()) {
        qWarning() << "Failed to start transaction:" << database.lastError().text();
        return std::nullopt;
    }

    try {
        const ReturnType result = operation();
        if (result.has_value()) {
            if (database.commit()) {
                return result;
            } else {
                qWarning() << "Failed to commit transaction:" << database.lastError().text();
            }
        }

        if (!database.rollback()) {
            qWarning() << "Failed to rollback transaction:" << database.lastError().text();
        }

        return std::nullopt;

    } catch (...) {
        if (!database.rollback()) {
            qWarning() << "Failed to rollback transaction:" << database.lastError().text();
        }

        throw;
    }
}

/**
 * Execute a lambda in the context of a database-level transaction.
 *
 * Notifications emitted by repositories during the transaction are batched
 * and flushed (deduplicated) after the transaction commits successfully.
 *
 * @param operation Returns std::nullopt if it should abort, a value otherwise.
 * @return The result of the operation, or std::nullopt on failure.
 */
template<typename Function>
auto executeInTransaction(const Function& operation) -> decltype(operation()) {
    auto guard = DataEventBroker::instance().batchNotifications();

    auto db = QSqlDatabase::database();
    auto result = rawExecuteInTransaction(db, operation);

    if (!result.has_value()) {
        guard.discard();
    }

    return result;
}
