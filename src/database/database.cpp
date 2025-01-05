/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "database.h"

#include <sqlite3.h>

#include <QDebug>
#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

Q_LOGGING_CATEGORY(OPA_SQL, "opa.sql");

const static auto driver = QStringLiteral("QSQLITE");

namespace {
    void executeScriptOrAbort(const QString& script, const QSqlDatabase& database) {
        for (auto& command: script.split(QStringLiteral(";"))) {
            command.replace(QStringLiteral("\n"), QStringLiteral(" "));
            command = command.trimmed();
            if (command.isEmpty()) {
                qDebug() << "Skipping command" << command;
                continue;
            }
            if (command.startsWith(QStringLiteral("--"))) {
                qCritical() << "Comments are not supported at the moment.";
                abort();
            }
            qDebug() << "Executing" << command;
            QSqlQuery theQuery(database);
            theQuery.prepare(command);
            if (!theQuery.exec()) {
                qCritical() << "Error occurred running SQL query.";
                qCritical() << theQuery.lastError().text();
                abort();
            }
        }
    }
}

// NOLINTNEXTLINE(*-use-internal-linkage)
int sql_trace_callback(unsigned int type, void* context, void* p, void* x) {
    Q_UNUSED(context);
    Q_UNUSED(x);
    if (type == SQLITE_TRACE_PROFILE) {
        auto* statement = static_cast<sqlite3_stmt*>(p);
        auto* sql = sqlite3_expanded_sql(statement);
        qDebug(OPA_SQL) << sql;
        sqlite3_free(sql);
    }

    return 0;
}

void openDatabase(const QString& file, bool seed, bool initialise) {
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

    if (!initialise) {
        qDebug() << "Not initialising database.";
        return;
    }

    qDebug() << "Adding built-in data";
    QFile initFile(QStringLiteral(":/init.sql"));
    if (!initFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error occurred opening database init file";
        abort();
    }
    QTextStream initStream(&initFile);
    QString init = initStream.readAll();
    executeScriptOrAbort(init, database);

    if (!seed) {
        qDebug() << "Not seeding database.";
        return;
    }

    qDebug() << "Seeding with sample data";

    // Initialize the data in the database.
    QFile seedFile(QStringLiteral(":/seed.sql"));
    if (!seedFile.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error occurred opening database seed file";
        abort();
    }
    QTextStream seedStream(&seedFile);
    QString seedQuery = seedStream.readAll();
    executeScriptOrAbort(seedQuery, database);
}

void closeDatabase() {
    QSqlDatabase::database().close();
}

bool hasActiveTransaction() {
    auto database = QSqlDatabase::database();
    auto qtHandle = database.driver()->handle();
    if (qtHandle.isValid() && qstrcmp(qtHandle.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        if (sqlite3* handle = *static_cast<sqlite3**>(qtHandle.data())) {
            return sqlite3_get_autocommit(handle) == 0;
        }
    }

    return false;
}
