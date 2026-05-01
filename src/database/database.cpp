/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppParameterMayBeConstPtrOrRef
#include "database.h"

using namespace Qt::StringLiterals;

#include <sqlite3.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

Q_LOGGING_CATEGORY(OPA_SQL, "opa.sql");

const static auto driver = u"QSQLITE"_s;

namespace {
struct Migration {
    int version = 0;
    QLatin1StringView description;
    QLatin1StringView resourcePath;
};

constexpr std::array migrations = {
    Migration{
        .version = 1,
        .description = "Add surrogate key to event_relations, rekey event_relation_citations"_L1,
        .resourcePath = ":/migrations/001_event_relations_surrogate_key.sql"_L1,
    },
    Migration{
        .version = 2,
        .description = "Add location_types and locations tables, add location_id to events"_L1,
        .resourcePath = ":/migrations/002_add_locations.sql"_L1,
    },
    Migration{
        .version = 3,
        .description = "Add event_type_translations and location_type_translations tables"_L1,
        .resourcePath = ":/migrations/003_add_type_translations.sql"_L1,
    },
    Migration{
        .version = 4,
        .description = "Add event_role_translations, name_origin_translations, source_types and source_type_translations"_L1,
        .resourcePath = ":/migrations/004_add_role_origin_translations_and_source_types.sql"_L1,
    },
    Migration{
        .version = 5,
        .description = "Add media table and junction tables for all entity types"_L1,
        .resourcePath = ":/migrations/005_add_media.sql"_L1,
    },
    Migration{
        .version = 6,
        .description = "Fix media.note to be nullable"_L1,
        .resourcePath = ":/migrations/006_fix_media_note_nullable.sql"_L1,
    },
};

void executeScriptOrAbort(const QString& script, const QSqlDatabase& database) {
    for (auto& command: script.split(u";"_s)) {
        command.replace(u"\n"_s, u" "_s);
        command = command.trimmed();
        if (command.isEmpty()) {
            qDebug() << "Skipping command" << command;
            continue;
        }
        if (command.startsWith(u"--"_s)) {
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
        theQuery.clear();
    }
}

}

void runMigrations(QSqlDatabase& database) {
    const int current = [&database]() {
        QSqlQuery vq(database);
        if (!vq.exec(u"PRAGMA user_version"_s)) {
            qCritical() << "Failed to query schema version:" << vq.lastError().text();
            abort();
        }
        if (!vq.next()) {
            qCritical() << "PRAGMA user_version returned no rows";
            abort();
        }
        return vq.value(0).toInt();
    }();

    for (const auto& m: migrations) {
        if (m.version <= current) {
            continue;
        }

        qDebug() << "Applying migration" << m.version << "-" << m.description;

        QSqlQuery fkOff(database);
        if (!fkOff.exec(u"PRAGMA foreign_keys = OFF"_s)) {
            qCritical() << "Failed to disable foreign keys before migration:" << fkOff.lastError().text();
            abort();
        }

        QFile sqlFile(m.resourcePath);
        if (!sqlFile.open(QFile::ReadOnly | QFile::Text)) {
            qCritical() << "Could not open migration file" << m.resourcePath;
            abort();
        }
        const QString sql = QTextStream(&sqlFile).readAll();

        if (!database.transaction()) {
            qCritical() << "Failed to start migration transaction:" << database.lastError().text();
            abort();
        }
        executeScriptOrAbort(sql, database);
        QSqlQuery stamp(database);
        if (!stamp.exec(u"PRAGMA user_version = %1"_s.arg(m.version))) {
            qCritical() << "Failed to stamp schema version after migration" << m.version << ":" << stamp.lastError().text();
            abort();
        }
        if (!database.commit()) {
            qCritical() << "Failed to commit migration" << m.version << ":" << database.lastError().text();
            abort();
        }

        QSqlQuery fkOn(database);
        if (!fkOn.exec(u"PRAGMA foreign_keys = ON"_s)) {
            qCritical() << "Failed to re-enable foreign keys after migration:" << fkOn.lastError().text();
            abort();
        }

        qDebug() << "Migration" << m.version << "complete.";
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
    if (file == u":memory:"_s) {
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
    if (!foreignKeys.exec(u"PRAGMA foreign_keys = ON;"_s)) {
        qWarning() << "Could not enable foreign keys: " << foreignKeys.lastError().text();
        abort();
    }

    if (existing) {
        qDebug() << "Running migrations on existing database...";
        runMigrations(database);
        return;
    }

    // Initialize the database based on the schema.
    QFile schema_file(u":/schema.sql"_s);
    if (!schema_file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error occurred opening database schema file";
        abort();
    }
    QTextStream schema_stream(&schema_file);
    const QString schema = schema_stream.readAll();

    // Run the creation script if this is a new database.
    qDebug() << "Running database creation script...";
    executeScriptOrAbort(schema, database);
    QSqlQuery initStamp(database);
    if (!initStamp.exec(u"PRAGMA user_version = %1"_s.arg(migrations.back().version))) {
        qCritical() << "Failed to stamp schema version on new database:" << initStamp.lastError().text();
        abort();
    }

    if (!initialise) {
        qDebug() << "Not initialising database.";
        return;
    }

    qDebug() << "Adding built-in data";
    QFile initFile(u":/init.sql"_s);
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
    QFile seedFile(u":/seed.sql"_s);
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
