/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "../src/domain/person/person_repository.h"

#include "../src/domain/name/name_repository.h"
#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestPersonRepository : public QObject {
    Q_OBJECT

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
        PersonRepository repo;
        auto id = repo.insertPerson(u"Male"_s, true);
        QVERIFY(id.has_value());

        auto person = repo.findById(*id);
        QVERIFY(person.has_value());
        QCOMPARE(person->id, *id);
        QCOMPARE(person->sex, u"Male"_s);
        QCOMPARE(person->root, true);
    }

    void testFindPeopleReturnsAll() {
        PersonRepository repo;
        repo.insertPerson(u"Male"_s);
        repo.insertPerson(u"Female"_s);

        auto people = repo.findPeople();
        QCOMPARE(people.size(), 2);
    }

    void testFindPeopleBySex() {
        PersonRepository repo;
        repo.insertPerson(u"Male"_s);
        repo.insertPerson(u"Female"_s);
        repo.insertPerson(u"Male"_s);

        PersonCriteria criteria;
        criteria.sex = u"Male"_s;
        auto males = repo.findPeople(criteria);
        QCOMPARE(males.size(), 2);
        for (const auto& p: males) {
            QCOMPARE(p.sex, u"Male"_s);
        }
    }

    void testFindPeopleRootOnly() {
        PersonRepository repo;
        repo.insertPerson(u"Male"_s, true);
        repo.insertPerson(u"Female"_s, false);

        PersonCriteria criteria;
        criteria.rootOnly = true;
        auto roots = repo.findPeople(criteria);
        QCOMPARE(roots.size(), 1);
        QCOMPARE(roots.first().root, true);
    }

    void testUpdatePerson() {
        PersonRepository repo;
        auto id = repo.insertPerson(u"Unknown"_s, false);
        QVERIFY(id.has_value());

        bool ok = repo.updatePerson(*id, u"Female"_s, true);
        QVERIFY(ok);

        auto updated = repo.findById(*id);
        QVERIFY(updated.has_value());
        QCOMPARE(updated->sex, u"Female"_s);
        QCOMPARE(updated->root, true);
    }

    void testDeletePerson() {
        PersonRepository repo;
        auto id = repo.insertPerson(u"Male"_s);
        QVERIFY(id.has_value());

        bool ok = repo.deletePerson(*id);
        QVERIFY(ok);

        auto gone = repo.findById(*id);
        QVERIFY(!gone.has_value());
    }

    void testGetNamesForPerson() {
        PersonRepository personRepo;
        auto personId = personRepo.insertPerson(u"Male"_s);
        QVERIFY(personId.has_value());

        QSqlQuery q;
        QVERIFY(q.exec(
            u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, 'John', 'Doe')"_s.arg(*personId)
        ));
        QVERIFY(q.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 2, 'Johnny', 'Doe')"_s
                           .arg(*personId)));

        NameRepository repo;
        auto names = repo.findNamesForPerson(*personId);
        QCOMPARE(names.size(), 2);
        QCOMPARE(names.at(0).givenNames, u"John"_s);
        QCOMPARE(names.at(0).sort, 1);
        QCOMPARE(names.at(1).givenNames, u"Johnny"_s);
        QCOMPARE(names.at(1).sort, 2);
    }

    void testFindPeopleWithPrimaryName() {
        PersonRepository repo;
        auto personId = repo.insertPerson(u"Male"_s);
        QVERIFY(personId.has_value());

        QSqlQuery q;
        QVERIFY(q.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, 'Jane', 'Smith')"_s
                           .arg(*personId)));

        auto people = repo.findPeopleWithPrimaryName();
        QCOMPARE(people.size(), 1);
        QCOMPARE(people.first().givenNames, u"Jane"_s);
        QCOMPARE(people.first().surname, u"Smith"_s);
    }

    void testFindDisplayById() {
        PersonRepository repo;
        auto personId = repo.insertPerson(u"Female"_s);
        QVERIFY(personId.has_value());

        QSqlQuery q;
        QVERIFY(q.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, 'Alice', 'Wonder')"_s
                           .arg(*personId)));

        auto display = repo.findDisplayById(*personId);
        QVERIFY(display.has_value());
        QCOMPARE(display->id, *personId);
        QCOMPARE(display->sex, u"Female"_s);
        QCOMPARE(display->givenNames, u"Alice"_s);
        QCOMPARE(display->surname, u"Wonder"_s);
    }

    void testFindByIdNotFound() {
        PersonRepository repo;
        auto result = repo.findById(9999);
        QVERIFY(!result.has_value());
    }
};

QTEST_MAIN(TestPersonRepository)

#include "person_repository_test.moc"
