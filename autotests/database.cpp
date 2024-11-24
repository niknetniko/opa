/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "database/database.h"

#include "data/event.h"
#include "database/schema.h"
#include <data/names.h>

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestDatabase : public QObject {
    Q_OBJECT

private:
    template<typename Enum>
    void runEnumValueCheck(QString tableName, int nameColumn = 1, int builtinColumn = 2) {
        auto database = QSqlDatabase::database();
        QSqlQuery eventTypesQuery{u"SELECT * FROM %1"_s.arg(tableName)};
        QVERIFY(eventTypesQuery.exec());

        auto metaTypeObject = QMetaEnum::fromType<Enum>();
        QStringList possibilities;
        for (int i = 0; i < metaTypeObject.keyCount(); ++i) {
            auto value = metaTypeObject.key(i);
            possibilities.append(QString::fromLatin1(value));
        }

        QSet<Enum> valuesInDatabase;
        while (eventTypesQuery.next()) {
            auto databaseValue = eventTypesQuery.value(nameColumn).toString();
            auto errorMessage = qPrintable(
                u"%1 is not a valid enum, possibilities are %2"_s.arg(databaseValue).arg(possibilities.join(u", "_s))
            );
            QVERIFY2(isValidEnum<Enum>(databaseValue), errorMessage);
            valuesInDatabase.insert(enumFromString<Enum>(databaseValue));
            auto isBuiltin = eventTypesQuery.value(builtinColumn).toBool();
            QVERIFY(isBuiltin);
        }

        QCOMPARE(valuesInDatabase.size(), metaTypeObject.keyCount());
    }

private Q_SLOTS:

    void init() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        // Open the database.
        open_database(u":memory:"_s);
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
    }

    void testEventRolesAreInserted() {
        runEnumValueCheck<EventRoles::Values>(Schema::EventRolesTable);
    }

    void testEventTypesAreInserted() {
        runEnumValueCheck<EventTypes::Values>(Schema::EventTypesTable);
    }

    void testNameOriginsAreInserted() {
        runEnumValueCheck<NameOrigins::Values>(Schema::NameOriginsTable);
    }
};

QTEST_MAIN(TestDatabase)

#include "database.moc"
