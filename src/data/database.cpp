//
// Created by niko on 7/04/2022.
//

#include "database.h"

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

const QString driver = "QSQLITE";

void open_database(const QString &file) {
    if (!QSqlDatabase::isDriverAvailable(driver)) {
        qCritical() << "SQLite driver is not available. Hu?";
        abort();
    }

    // The main database connection.
    QSqlDatabase database = QSqlDatabase::addDatabase(driver);
    database.setDatabaseName(file);

    if (!database.open()) {
        qDebug("Error occurred opening the database %s", qPrintable(file));
        qDebug("%s.", qPrintable(database.lastError().text()));
        abort();
    }

    // Run the creation script if this is a new database.
    // The script is conditional, so we don't actually need to check
    // if the database is new at the moment.
    qDebug("Running database creation script...");
    QSqlQuery initQuery(database);
    initQuery.prepare(R"SQLSNIP(
        CREATE TABLE IF NOT EXISTS person(
            id integer not null primary key autoincrement,
            given_names text not null,
            nick_name text,
            call_name text,
            suffix text
        );

        CREATE TABLE IF NOT EXISTS roots(
            id integer not null primary key autoincrement,
            person integer,
            color text not null,
            FOREIGN KEY (person) REFERENCES person(id)
        );
)SQLSNIP");

    if (!initQuery.exec()) {
        qDebug("Error occurred running the database creation script.");
        qWarning("%s", qPrintable(initQuery.lastError().text()));
    } else {
        qDebug() << "OK, database initialized!";
    }

}
