/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "database.h"

#include <sqlite3.h>

#include <QDebug>
#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

Q_LOGGING_CATEGORY(OPA_SQL, "opa.sql");

const static auto driver = QStringLiteral("QSQLITE"); // NOLINT(*-err58-cpp)

// NOLINTNEXTLINE(*-use-internal-linkage)
void executeScriptOrAbort(const QString& script, const QSqlDatabase& database) {
    for (auto& command: script.split(QStringLiteral(";"))) {
        command.replace(QStringLiteral("\n"), QStringLiteral(" "));
        command = command.trimmed();
        if (command.isEmpty()) {
            qDebug() << "Skipping command" << command;
            continue;
        }
        if (command.startsWith(QStringLiteral("--"))) {
            qDebug() << "Skipping comment" << command;
            continue;
        }
        qDebug() << "Executing" << command;
        QSqlQuery theQuery(database);
        theQuery.prepare(command);
        if (!theQuery.exec()) {
            qDebug("Error occurred running SQL query.");
            qWarning("%s", qPrintable(theQuery.lastError().text()));
            abort();
        }
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
// NOLINTNEXTLINE(*-use-internal-linkage)
int sql_trace_callback(unsigned int type, [[maybe_unused]] void* context, void* p, [[maybe_unused]] void* x) {
    if (type == SQLITE_TRACE_PROFILE) {
        auto* statement = static_cast<sqlite3_stmt*>(p);
        const auto* sql = sqlite3_expanded_sql(statement);
        qDebug(OPA_SQL) << sql;
    }

    return 0;
}

void open_database(const QString& file, bool seed) {
    if (!QSqlDatabase::isDriverAvailable(driver)) {
        qCritical() << "SQLite driver is not available. Hu?" << QSqlDatabase::drivers();
        abort();
    }

    // Support in-memory databases for tests.
    bool existing = false;
    if (file == QStringLiteral(":memory:")) {
        existing = false;
    } else {
        existing = QFile::exists(file);
        qDebug() << "Looking at file at " << QFileInfo(file).canonicalFilePath();
    }

    // The main database connection.
    QSqlDatabase database = QSqlDatabase::addDatabase(driver);
    database.setDatabaseName(file);

    if (!database.open()) {
        qDebug() << "Error occurred opening the database " << file;
        qDebug() << database.lastError().text();
        abort();
    }

    QVariant v = database.driver()->handle();
    if (v.isValid() && (qstrcmp(v.typeName(), "sqlite3*") == 0)) {
        // v.data() returns a pointer to the handle
        if (sqlite3* handle = *static_cast<sqlite3**>(v.data())) {
            sqlite3_trace_v2(handle, SQLITE_TRACE_PROFILE, sql_trace_callback, nullptr);
        }
    }

    // Ensure we have foreign keys...
    QSqlQuery foreignKeys(database);
    if (!foreignKeys.exec(QStringLiteral("PRAGMA foreign_keys = ON;"))) {
        qWarning() << "Could not enable foreign keys: " << foreignKeys.lastError().text();
        abort();
    }

    // If there is already a database, do nothing.
    // TODO: handle migration somehow?
    if (existing) {
        qDebug() << "Skipping initialization, as it exists already.";
        return;
    }

    // Initialize the database based on the schema.
    QFile schema_file(QStringLiteral(":/schema.sql"));
    if (!schema_file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error occurred opening database schema file";
        abort();
    }
    QTextStream schema_stream(&schema_file);
    const QString schema = schema_stream.readAll();

    // Run the creation script if this is a new database.
    qDebug() << "Running database creation script...";
    executeScriptOrAbort(schema, database);

    if (!seed) {
        qDebug() << "Not seeding database.";
        return;
    }

    // Initialize the data in the database.
    QFile init_file(QStringLiteral(":/init.sql"));
    if (!init_file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error occurred opening database init file";
        abort();
    }
    QTextStream init_stream(&init_file);
    const QString init = init_stream.readAll();
    executeScriptOrAbort(init, database);
}

void close_database() {
    QSqlDatabase::database().close();
}
