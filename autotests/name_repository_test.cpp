/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "../src/domain/name/name_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestNameRepository : public QObject {
    Q_OBJECT

    IntegerPrimaryKey insertPerson() {
        return insertQuery(u"INSERT INTO people (root, sex) VALUES (false, 'Unknown')"_s);
    }

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testInsertAndFindById() {
        auto personId = insertPerson();
        NameRepository repo;
        auto nameId = repo.insertName(personId, 1);
        QVERIFY(nameId.has_value());

        auto name = repo.findById(*nameId);
        QVERIFY(name.has_value());
        QCOMPARE(name->id, *nameId);
        QCOMPARE(name->personId, personId);
        QCOMPARE(name->sort, 1);
        QVERIFY(name->givenNames.isEmpty());
        QVERIFY(name->surname.isEmpty());
    }

    void testFindNamesForPerson() {
        auto personId = insertPerson();
        NameRepository repo;
        repo.insertName(personId, 1);
        repo.insertName(personId, 2);

        auto names = repo.findNamesForPerson(personId);
        QCOMPARE(names.size(), 2);
        QCOMPARE(names.at(0).sort, 1);
        QCOMPARE(names.at(1).sort, 2);
    }

    void testFindNamesForPersonReturnsOnlyForThatPerson() {
        auto person1 = insertPerson();
        auto person2 = insertPerson();
        NameRepository repo;
        repo.insertName(person1, 1);
        repo.insertName(person2, 1);

        auto names = repo.findNamesForPerson(person1);
        QCOMPARE(names.size(), 1);
        QCOMPARE(names.first().personId, person1);
    }

    void testUpdateName() {
        auto personId = insertPerson();
        NameRepository repo;
        auto nameId = repo.insertName(personId, 1);
        QVERIFY(nameId.has_value());

        bool ok = repo.updateName(*nameId, u"Dr."_s, u"John"_s, u"van"_s, u"Doe"_s, u"A note"_s, -1);
        QVERIFY(ok);

        auto updated = repo.findById(*nameId);
        QVERIFY(updated.has_value());
        QCOMPARE(updated->titles, u"Dr."_s);
        QCOMPARE(updated->givenNames, u"John"_s);
        QCOMPARE(updated->prefix, u"van"_s);
        QCOMPARE(updated->surname, u"Doe"_s);
        QCOMPARE(updated->note, u"A note"_s);
        QVERIFY(!updated->originId.has_value());
    }

    void testUpdateNameSort() {
        auto personId = insertPerson();
        NameRepository repo;
        auto nameId = repo.insertName(personId, 1);
        QVERIFY(nameId.has_value());

        bool ok = repo.updateNameSort(*nameId, 5);
        QVERIFY(ok);

        auto updated = repo.findById(*nameId);
        QVERIFY(updated.has_value());
        QCOMPARE(updated->sort, 5);
    }

    void testDeleteName() {
        auto personId = insertPerson();
        NameRepository repo;
        auto nameId = repo.insertName(personId, 1);
        QVERIFY(nameId.has_value());

        bool ok = repo.deleteName(*nameId);
        QVERIFY(ok);

        auto gone = repo.findById(*nameId);
        QVERIFY(!gone.has_value());
    }

    void testFindByIdNotFound() {
        NameRepository repo;
        auto result = repo.findById(9999);
        QVERIFY(!result.has_value());
    }

    void testFindWithOriginById() {
        auto personId = insertPerson();
        // Get an existing origin ID from the seeded data (database.cpp opens with seed=false, so insert one).
        auto originId = insertQuery(u"INSERT INTO name_origins (origin, builtin) VALUES ('Inherited', false)"_s);

        NameRepository repo;
        auto nameId = repo.insertName(personId, 1);
        QVERIFY(nameId.has_value());
        repo.updateName(*nameId, QString(), u"Alice"_s, QString(), u"Smith"_s, QString(), originId);

        auto result = repo.findWithOriginById(*nameId);
        QVERIFY(result.has_value());
        QCOMPARE(result->givenNames, u"Alice"_s);
        QCOMPARE(result->origin, u"Inherited"_s);
    }

    void testInsertMultipleNamesForOnePerson() {
        auto personId = insertPerson();
        NameRepository repo;
        auto id1 = repo.insertName(personId, 1);
        auto id2 = repo.insertName(personId, 2);
        auto id3 = repo.insertName(personId, 3);

        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());
        QVERIFY(id3.has_value());
        QVERIFY(*id1 != *id2);
        QVERIFY(*id2 != *id3);

        auto names = repo.findNamesForPerson(personId);
        QCOMPARE(names.size(), 3);
    }
};

QTEST_MAIN(TestNameRepository)

#include "name_repository_test.moc"
