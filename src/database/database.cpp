#include "database.h"

#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <qsqldriver.h>
#include <sqlite3.h>

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

void open_database(const QString& file) {
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
