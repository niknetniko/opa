/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "database/database.h"

#include "database/schema.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestDatabase : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void databaseInitialization() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        // Open the database.
        open_database(u":memory:"_s);

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
            u"sqlite_sequence"_s
        };
        expected.sort();
        QCOMPARE(tables, expected);

        // TODO: test inserted data or not?
    }
};

QTEST_MAIN(TestDatabase)

#include "database.moc"
