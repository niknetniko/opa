/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "database/database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

// The old pre-migration schema for event_relations (composite PK, no surrogate id).
static const QString OLD_EVENT_RELATIONS_DDL = QStringLiteral(
    "CREATE TABLE event_relations ("
    "  event_id INTEGER NOT NULL REFERENCES events (id) ON DELETE CASCADE,"
    "  person_id INTEGER NOT NULL REFERENCES people (id) ON DELETE CASCADE,"
    "  role_id INTEGER NOT NULL REFERENCES event_roles (id) ON DELETE RESTRICT,"
    "  PRIMARY KEY (event_id, person_id, role_id)"
    ")"
);

// The old pre-migration schema for event_relation_citations (keyed by composite FK).
static const QString OLD_EVENT_RELATION_CITATIONS_DDL = QStringLiteral(
    "CREATE TABLE event_relation_citations ("
    "  event_id INTEGER NOT NULL,"
    "  person_id INTEGER NOT NULL,"
    "  role_id INTEGER NOT NULL,"
    "  source_id INTEGER NOT NULL REFERENCES sources (id) ON DELETE CASCADE,"
    "  PRIMARY KEY (event_id, person_id, role_id, source_id),"
    "  FOREIGN KEY (event_id, person_id, role_id)"
    "    REFERENCES event_relations (event_id, person_id, role_id) ON DELETE CASCADE"
    ")"
);

static int userVersion(QSqlDatabase& db) {
    QSqlQuery q(db);
    q.exec(u"PRAGMA user_version"_s);
    q.next();
    return q.value(0).toInt();
}

static bool columnExists(QSqlDatabase& db, const QString& table, const QString& column) {
    QSqlQuery q(db);
    q.exec(QStringLiteral("PRAGMA table_info(%1)").arg(table));
    while (q.next()) {
        if (q.value(u"name"_s).toString() == column) {
            return true;
        }
    }
    return false;
}

// Opens a raw :memory: connection (no schema, no tracing) and returns it as the default connection.
static QSqlDatabase openRawDatabase() {
    auto db = QSqlDatabase::addDatabase(u"QSQLITE"_s);
    db.setDatabaseName(u":memory:"_s);
    db.open();
    return db;
}

// Creates the minimal supporting tables needed to satisfy FK constraints for migration 1.
// FK enforcement is left OFF — call this before turning it back on.
static void createSupportingTables(QSqlDatabase& db) {
    const QStringList ddl = {
        u"CREATE TABLE people (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, root BOOLEAN, sex TEXT)"_s,
        u"CREATE TABLE event_types (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
        u"CREATE TABLE event_roles (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, role TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
        u"CREATE TABLE events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type_id INTEGER NOT NULL REFERENCES event_types (id) ON DELETE RESTRICT, date TEXT, name TEXT, note TEXT)"_s,
        u"CREATE TABLE sources (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, title TEXT, type TEXT, author TEXT, publication TEXT, confidence TEXT NOT NULL, note TEXT, parent_id INTEGER REFERENCES sources (id) ON DELETE SET NULL)"_s,
    };
    for (const auto& stmt: ddl) {
        QSqlQuery q(db);
        q.exec(stmt);
    }
}

// Builds a version-0 database: supporting tables + old event_relations + old event_relation_citations.
static QSqlDatabase setupPreMigrationDatabase() {
    auto db = openRawDatabase();
    QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
    createSupportingTables(db);
    QSqlQuery(db).exec(OLD_EVENT_RELATIONS_DDL);
    QSqlQuery(db).exec(OLD_EVENT_RELATION_CITATIONS_DDL);
    // user_version stays at 0 (SQLite default).
    return db;
}

class TestDatabaseMigrations : public QObject {
    Q_OBJECT

private Q_SLOTS:

    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    // ==================== Migration system ====================

    void testFreshDatabaseIsStampedWithLatestVersion() {
        openDatabase(u":memory:"_s, false, false);
        auto db = QSqlDatabase::database();
        QCOMPARE(userVersion(db), 5);
    }

    void testRunMigrationsIsNoopOnCurrentDatabase() {
        openDatabase(u":memory:"_s, false, false);
        auto db = QSqlDatabase::database();
        QCOMPARE(userVersion(db), 5);

        runMigrations(db);

        QCOMPARE(userVersion(db), 5);
    }

    void testRunMigrationsSkipsAlreadyAppliedMigration() {
        auto db = setupPreMigrationDatabase();
        // Manually stamp as already migrated through migration 1.
        QSqlQuery(db).exec(u"PRAGMA user_version = 1"_s);
        QCOMPARE(userVersion(db), 1);

        // event_relations still lacks the id column at this point.
        QVERIFY(!columnExists(db, u"event_relations"_s, u"id"_s));

        runMigrations(db);

        // Migration 1 schema should be untouched — it was skipped.
        QVERIFY(!columnExists(db, u"event_relations"_s, u"id"_s));
        // Migrations 2 through 5 ran, bumping to version 5.
        QCOMPARE(userVersion(db), 5);
    }

    // ==================== Migration 1 ====================

    void testMigration1AddsIdColumnToEventRelations() {
        auto db = setupPreMigrationDatabase();
        QVERIFY(!columnExists(db, u"event_relations"_s, u"id"_s));

        runMigrations(db);

        QVERIFY(columnExists(db, u"event_relations"_s, u"id"_s));
        QCOMPARE(userVersion(db), 5);
    }

    void testMigration1ReplacesCompositeKeyWithUniqueConstraint() {
        auto db = setupPreMigrationDatabase();
        runMigrations(db);

        // Should be able to insert a row using only (event_id, person_id, role_id).
        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery ins(db);
        QVERIFY(ins.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (1, 1, 1)"_s));
        auto id = ins.lastInsertId();
        QVERIFY(id.isValid());
        QVERIFY(id.toInt() > 0);
    }

    void testMigration1PreservesEventRelationsRows() {
        auto db = setupPreMigrationDatabase();

        // Insert supporting rows and one event_relation.
        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO people VALUES (1, false, 'Unknown')"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_types VALUES (1, 'Birth', false)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_roles VALUES (1, 'Primary', false)"_s);
        QSqlQuery(db).exec(u"INSERT INTO events VALUES (1, 1, NULL, NULL, NULL)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (1, 1, 1)"_s);

        runMigrations(db);

        QSqlQuery q(db);
        QVERIFY(q.exec(u"SELECT event_id, person_id, role_id FROM event_relations"_s));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
        QCOMPARE(q.value(1).toInt(), 1);
        QCOMPARE(q.value(2).toInt(), 1);
        QVERIFY(!q.next());
    }

    void testMigration1AssignsNewSurrogateIds() {
        auto db = setupPreMigrationDatabase();

        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (1, 1, 1)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (2, 1, 1)"_s);

        runMigrations(db);

        QSqlQuery q(db);
        QVERIFY(q.exec(u"SELECT id FROM event_relations ORDER BY event_id"_s));
        QVERIFY(q.next());
        const int id1 = q.value(0).toInt();
        QVERIFY(q.next());
        const int id2 = q.value(0).toInt();
        QVERIFY(id1 > 0);
        QVERIFY(id2 > 0);
        QVERIFY(id1 != id2);
    }

    void testMigration1RekeysEventRelationCitations() {
        auto db = setupPreMigrationDatabase();

        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO sources VALUES (1, 'Source A', NULL, NULL, NULL, 'High', NULL, NULL)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (10, 2, 3)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (10, 4, 3)"_s);
        // Citation belongs to the first relation (event_id=10, person_id=2, role_id=3).
        QSqlQuery(db).exec(u"INSERT INTO event_relation_citations VALUES (10, 2, 3, 1)"_s);

        runMigrations(db);

        // Find the new surrogate id for (event_id=10, person_id=2, role_id=3).
        QSqlQuery lookup(db);
        QVERIFY(lookup.exec(u"SELECT id FROM event_relations WHERE event_id=10 AND person_id=2 AND role_id=3"_s));
        QVERIFY(lookup.next());
        const int expectedRelationId = lookup.value(0).toInt();

        // The citation should reference that surrogate id.
        QSqlQuery cite(db);
        QVERIFY(cite.exec(u"SELECT event_relation_id, source_id FROM event_relation_citations"_s));
        QVERIFY(cite.next());
        QCOMPARE(cite.value(0).toInt(), expectedRelationId);
        QCOMPARE(cite.value(1).toInt(), 1);
        QVERIFY(!cite.next());
    }

    void testMigration1PreservesMultipleCitationsPerRelation() {
        auto db = setupPreMigrationDatabase();

        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO sources VALUES (1, 'Source A', NULL, NULL, NULL, 'High', NULL, NULL)"_s);
        QSqlQuery(db).exec(u"INSERT INTO sources VALUES (2, 'Source B', NULL, NULL, NULL, 'Low', NULL, NULL)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (1, 1, 1)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relation_citations VALUES (1, 1, 1, 1)"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relation_citations VALUES (1, 1, 1, 2)"_s);

        runMigrations(db);

        QSqlQuery q(db);
        QVERIFY(q.exec(u"SELECT COUNT(*) FROM event_relation_citations"_s));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 2);
    }

    void testMigration1LeavesEventRelationCitationsEmptyWhenNoneExisted() {
        auto db = setupPreMigrationDatabase();

        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_relations VALUES (1, 1, 1)"_s);
        // No citations.

        runMigrations(db);

        QSqlQuery q(db);
        QVERIFY(q.exec(u"SELECT COUNT(*) FROM event_relation_citations"_s));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 0);
    }

    // ==================== Migration 2 ====================

    static QSqlDatabase setupVersion1Database() {
        auto db = openRawDatabase();
        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        const QStringList ddl = {
            u"CREATE TABLE name_origins (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, origin TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
            u"CREATE TABLE people (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, root BOOLEAN, sex TEXT)"_s,
            u"CREATE TABLE names (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, person_id INTEGER NOT NULL, sort INTEGER NOT NULL, titles TEXT, given_names TEXT, prefix TEXT, surname TEXT, note TEXT, origin_id INTEGER NULL DEFAULT NULL)"_s,
            u"CREATE TABLE event_types (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
            u"CREATE TABLE event_roles (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, role TEXT, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s,
            u"CREATE TABLE events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type_id INTEGER NOT NULL, date TEXT, name TEXT, note TEXT)"_s,
            u"CREATE TABLE event_relations (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, event_id INTEGER NOT NULL, person_id INTEGER NOT NULL, role_id INTEGER NOT NULL, UNIQUE (event_id, person_id, role_id))"_s,
            u"CREATE TABLE sources (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, title TEXT, type TEXT, author TEXT, publication TEXT, confidence TEXT NOT NULL, note TEXT, parent_id INTEGER)"_s,
            u"CREATE TABLE event_citations (event_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (event_id, source_id))"_s,
            u"CREATE TABLE event_relation_citations (event_relation_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (event_relation_id, source_id))"_s,
            u"CREATE TABLE name_citations (name_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (name_id, source_id))"_s,
            u"CREATE TABLE person_citations (person_id INTEGER NOT NULL, source_id INTEGER NOT NULL, PRIMARY KEY (person_id, source_id))"_s,
        };
        for (const auto& stmt: ddl) {
            QSqlQuery(db).exec(stmt);
        }
        QSqlQuery(db).exec(u"PRAGMA user_version = 1"_s);
        return db;
    }

    void testMigration2AddsLocationTypesTable() {
        auto db = setupVersion1Database();
        QVERIFY(!columnExists(db, u"location_types"_s, u"id"_s));

        runMigrations(db);

        QVERIFY(columnExists(db, u"location_types"_s, u"id"_s));
        QCOMPARE(userVersion(db), 5);
    }

    // ==================== Migration 3 ====================

    static QSqlDatabase setupVersion2Database() {
        auto db = setupVersion1Database();
        QSqlQuery(db).exec(
            u"CREATE TABLE location_types (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, type TEXT NOT NULL, builtin BOOLEAN NOT NULL DEFAULT FALSE)"_s
        );
        QSqlQuery(db).exec(
            u"CREATE TABLE locations (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, type_id INTEGER NULL, parent_id INTEGER NULL, note TEXT, latitude REAL, longitude REAL, date_start TEXT, date_end TEXT)"_s
        );
        QSqlQuery(db).exec(u"ALTER TABLE events ADD COLUMN location_id INTEGER NULL"_s);
        QSqlQuery(db).exec(u"PRAGMA user_version = 2"_s);
        return db;
    }

    void testMigration3AddsEventTypeTranslationsTable() {
        auto db = setupVersion2Database();
        QVERIFY(!columnExists(db, u"event_type_translations"_s, u"id"_s));

        runMigrations(db);

        QVERIFY(columnExists(db, u"event_type_translations"_s, u"id"_s));
        QVERIFY(columnExists(db, u"event_type_translations"_s, u"type_id"_s));
        QVERIFY(columnExists(db, u"event_type_translations"_s, u"locale"_s));
        QVERIFY(columnExists(db, u"event_type_translations"_s, u"name"_s));
        QCOMPARE(userVersion(db), 5);
    }

    void testMigration3AddsLocationTypeTranslationsTable() {
        auto db = setupVersion2Database();
        QVERIFY(!columnExists(db, u"location_type_translations"_s, u"id"_s));

        runMigrations(db);

        QVERIFY(columnExists(db, u"location_type_translations"_s, u"id"_s));
        QVERIFY(columnExists(db, u"location_type_translations"_s, u"type_id"_s));
        QVERIFY(columnExists(db, u"location_type_translations"_s, u"locale"_s));
        QVERIFY(columnExists(db, u"location_type_translations"_s, u"name"_s));
    }

    void testMigration2AddsLocationsTable() {
        auto db = setupVersion1Database();

        runMigrations(db);

        QVERIFY(columnExists(db, u"locations"_s, u"id"_s));
        QVERIFY(columnExists(db, u"locations"_s, u"parent_id"_s));
        QVERIFY(columnExists(db, u"locations"_s, u"type_id"_s));
    }

    void testMigration2AddsLocationIdToEvents() {
        auto db = setupVersion1Database();
        QVERIFY(!columnExists(db, u"events"_s, u"location_id"_s));

        runMigrations(db);

        QVERIFY(columnExists(db, u"events"_s, u"location_id"_s));
    }

    void testMigration2PreservesExistingEvents() {
        auto db = setupVersion1Database();
        QSqlQuery(db).exec(u"PRAGMA foreign_keys = OFF"_s);
        QSqlQuery(db).exec(u"INSERT INTO event_types VALUES (1, 'Birth', false)"_s);
        QSqlQuery(db).exec(u"INSERT INTO events VALUES (1, 1, '1900', 'Test', 'Note')"_s);

        runMigrations(db);

        QSqlQuery q(db);
        QVERIFY(q.exec(u"SELECT id, name FROM events"_s));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
        QCOMPARE(q.value(1).toString(), u"Test"_s);
    }
};

QTEST_MAIN(TestDatabaseMigrations)

#include "database_migration_test.moc"
