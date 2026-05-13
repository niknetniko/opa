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
bool hasActiveTransaction();

/**
 * Execute a lambda in the context of a database-level transaction.
 *
 * Notifications emitted by repositories during the transaction are batched
 * and flushed (deduplicated) after the transaction commits successfully.
 *
 * Supports nesting: if a transaction is already active, the operation runs
 * directly without starting a new transaction. The outermost call handles
 * commit/rollback and notification flushing.
 *
 * @param operation Returns std::nullopt if it should abort, a value otherwise.
 * @return The result of the operation, or std::nullopt on failure.
 */
template<typename Function>
auto executeInTransaction(const Function& operation) -> decltype(operation()) {
    using ReturnType = decltype(operation());

    auto guard = DataEventBroker::instance().batchNotifications();

    if (hasActiveTransaction()) {
        return operation();
    }

    auto db = QSqlDatabase::database();

    if (!db.transaction()) {
        qWarning() << "Failed to start transaction:" << db.lastError().text();
        guard.discard();
        return std::nullopt;
    }

    try {
        const ReturnType result = operation();
        if (result.has_value()) {
            if (db.commit()) {
                return result;
            } else {
                qWarning() << "Failed to commit transaction:" << db.lastError().text();
            }
        }

        if (!db.rollback()) {
            qWarning() << "Failed to rollback transaction:" << db.lastError().text();
        }

        guard.discard();
        return std::nullopt;

    } catch (...) {
        if (!db.rollback()) {
            qWarning() << "Failed to rollback transaction:" << db.lastError().text();
        }

        guard.discard();
        throw;
    }
}
