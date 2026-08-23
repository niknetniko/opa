/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "database/database.h"

#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestDatabase : public QObject {
    Q_OBJECT

private Q_SLOTS:

    void init() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        // Open the database.
        openDatabase(u":memory:"_s);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testDatabaseSeedIsValid() {
        // Check that we have created the database.
        auto database = QSqlDatabase::database();
        QCOMPARE(database.isOpen(), true);
        QCOMPARE(database.isValid(), true);

        auto tables = database.tables();
        qDebug() << tables;
        tables.sort();
        QStringList expected = {
            Schema::NamesTable,
            Schema::PeopleTable,
            Schema::EventRelationsTable,
            Schema::EventsTable,
            Schema::FamiliesTable,
            Schema::SourcesTable,
            Schema::EventCitationsTable,
            Schema::EventRelationCitationsTable,
            Schema::NameCitationsTable,
            Schema::PersonCitationsTable,
            Schema::LocationsTable,
            Schema::MediaTable,
            Schema::PersonMediaTable,
            Schema::NameMediaTable,
            Schema::EventMediaTable,
            Schema::EventRelationMediaTable,
            Schema::SourceMediaTable,
            Schema::LocationMediaTable,
            u"person_external_ids"_s,
            u"location_external_ids"_s,
            u"family_external_ids"_s,
            u"event_external_ids"_s,
            u"source_external_ids"_s,
            u"media_external_ids"_s,
            // Special SQLite tables...
            u"sqlite_sequence"_s
        };
        expected.sort();
        QCOMPARE(tables, expected);
    }
};

QTEST_MAIN(TestDatabase)

#include "database.moc"
