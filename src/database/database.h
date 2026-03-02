/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QLoggingCategory>
// ReSharper disable once CppUnusedIncludeDirective
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
 * Return true if the database is currently in a transaction, false otherwise.
 */
bool hasActiveTransaction();

/**
 * Execute a lambda in the context of a database-level transaction.
 *
 * If the lambda throws an exception, the transaction will be rolled back, the exception will be thrown.
 *
 * @param operation Returns false if it should abort, true otherwise.
 * @return True if the operation was successful and committed, false otherwise.
 */
template <typename Function>
auto executeInTransaction(const Function& operation) -> decltype(operation()) {
    using ReturnType = decltype(operation());

    auto db = QSqlDatabase::database();

    if (!db.transaction()) {
        qWarning() << "Failed to start transaction:" << db.lastError().text();
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

        return std::nullopt;

    } catch (...) {
        if (!db.rollback()) {
            qWarning() << "Failed to rollback transaction:" << db.lastError().text();
        }

        throw;
    }
}