//
// Created by niko on 8/10/24.
//

#include <QTest>
#include <QSqlDatabase>
#include "database/database.h"
#include "database/schema.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

class TestDatabase : public QObject {
Q_OBJECT

private Q_SLOTS:

    void databaseInitialization() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")));

        // Open the database.
        open_database(QStringLiteral(":memory:"));

        // Check that we have created the database.
        auto database = QSqlDatabase::database();
        QCOMPARE(database.isOpen(), true);
        QCOMPARE(database.isValid(), true);

        auto tables = database.tables();
        qDebug() << tables;
        tables.sort();
        QStringList expected = {
                Schema::NameOriginsTable,
                Schema::NamesTable,
                Schema::PeopleTable,
                Schema::EventRelationsTable,
                Schema::EventRolesTable,
                Schema::EventTypesTable,
                Schema::EventsTable,
                // Special SQLite tables...
                QStringLiteral("sqlite_sequence")
        };
        expected.sort();
        QCOMPARE(tables, expected);

        // TODO: test inserted data or not?
    }
};

QTEST_MAIN(TestDatabase)

#include "database.moc"

#pragma clang diagnostic pop