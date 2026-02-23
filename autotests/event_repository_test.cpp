/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "../src/domain/event/event_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestEventRepository : public QObject {
    Q_OBJECT

    IntegerPrimaryKey insertPerson() {
        return insertQuery(u"INSERT INTO people (root, sex) VALUES (false, 'Unknown')"_s);
    }

    IntegerPrimaryKey insertEventType(const QString& type = u"Birth"_s) {
        return insertQuery(
            u"INSERT INTO event_types (type, builtin) VALUES ('%1', false)"_s.arg(type)
        );
    }

    IntegerPrimaryKey insertEventRole(const QString& role = u"Primary"_s) {
        return insertQuery(
            u"INSERT INTO event_roles (role, builtin) VALUES ('%1', false)"_s.arg(role)
        );
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

    // EventType tests

    void testInsertAndFindEventTypeById() {
        EventRepository repo;
        auto id = repo.insertEventType(u"Birth"_s);
        QVERIFY(id.has_value());

        auto result = repo.findEventTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->type, u"Birth"_s);
        QCOMPARE(result->builtin, false);
    }

    void testFindAllEventTypes() {
        EventRepository repo;
        const auto before = repo.findAllEventTypes().size();
        repo.insertEventType(u"TestTypeA"_s);
        repo.insertEventType(u"TestTypeB"_s);

        auto types = repo.findAllEventTypes();
        QCOMPARE(types.size(), before + 2);
    }

    void testUpdateEventType() {
        EventRepository repo;
        auto id = repo.insertEventType(u"Birth"_s);
        QVERIFY(id.has_value());

        bool ok = repo.updateEventType(*id, u"Baptism"_s);
        QVERIFY(ok);

        auto result = repo.findEventTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->type, u"Baptism"_s);
    }

    void testDeleteEventType() {
        EventRepository repo;
        auto id = repo.insertEventType(u"Birth"_s);
        QVERIFY(id.has_value());

        bool ok = repo.deleteEventType(*id);
        QVERIFY(ok);

        auto result = repo.findEventTypeById(*id);
        QVERIFY(!result.has_value());
    }

    void testFindEventTypeByIdNotFound() {
        EventRepository repo;
        auto result = repo.findEventTypeById(9999);
        QVERIFY(!result.has_value());
    }

    // EventRole tests

    void testInsertAndFindEventRoleById() {
        EventRepository repo;
        auto id = repo.insertEventRole(u"Primary"_s);
        QVERIFY(id.has_value());

        auto result = repo.findEventRoleById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->role, u"Primary"_s);
        QCOMPARE(result->builtin, false);
    }

    void testFindAllEventRoles() {
        EventRepository repo;
        const auto before = repo.findAllEventRoles().size();
        repo.insertEventRole(u"TestRoleA"_s);
        repo.insertEventRole(u"TestRoleB"_s);

        auto roles = repo.findAllEventRoles();
        QCOMPARE(roles.size(), before + 2);
    }

    void testUpdateEventRole() {
        EventRepository repo;
        auto id = repo.insertEventRole(u"Primary"_s);
        QVERIFY(id.has_value());

        bool ok = repo.updateEventRole(*id, u"Witness"_s);
        QVERIFY(ok);

        auto result = repo.findEventRoleById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->role, u"Witness"_s);
    }

    void testDeleteEventRole() {
        EventRepository repo;
        auto id = repo.insertEventRole(u"Primary"_s);
        QVERIFY(id.has_value());

        bool ok = repo.deleteEventRole(*id);
        QVERIFY(ok);

        auto result = repo.findEventRoleById(*id);
        QVERIFY(!result.has_value());
    }

    // Event tests

    void testInsertAndFindEventById() {
        auto typeId = insertEventType();
        EventRepository repo;
        auto id = repo.insertEvent(typeId);
        QVERIFY(id.has_value());

        auto result = repo.findEventById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->typeId, typeId);
        QVERIFY(result->date.isEmpty());
        QVERIFY(result->name.isEmpty());
        QVERIFY(result->note.isEmpty());
    }

    void testUpdateEvent() {
        auto typeId = insertEventType();
        EventRepository repo;
        auto id = repo.insertEvent(typeId);
        QVERIFY(id.has_value());

        bool ok = repo.updateEvent(*id, typeId, u"1900-01-01"_s, u"Test event"_s, u"A note"_s);
        QVERIFY(ok);

        auto result = repo.findEventById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->date, u"1900-01-01"_s);
        QCOMPARE(result->name, u"Test event"_s);
        QCOMPARE(result->note, u"A note"_s);
    }

    void testDeleteEvent() {
        auto typeId = insertEventType();
        EventRepository repo;
        auto id = repo.insertEvent(typeId);
        QVERIFY(id.has_value());

        bool ok = repo.deleteEvent(*id);
        QVERIFY(ok);

        auto result = repo.findEventById(*id);
        QVERIFY(!result.has_value());
    }

    void testFindEventByIdNotFound() {
        EventRepository repo;
        auto result = repo.findEventById(9999);
        QVERIFY(!result.has_value());
    }

    // EventRelation tests

    void testInsertAndFindRelationsForEvent() {
        auto personId = insertPerson();
        auto typeId = insertEventType();
        auto roleId = insertEventRole();
        EventRepository repo;
        auto eventId = repo.insertEvent(typeId);
        QVERIFY(eventId.has_value());

        bool ok = repo.insertEventRelation(*eventId, personId, roleId);
        QVERIFY(ok);

        auto relations = repo.findRelationsForEvent(*eventId);
        QCOMPARE(relations.size(), 1);
        QCOMPARE(relations.first().eventId, *eventId);
        QCOMPARE(relations.first().personId, personId);
        QCOMPARE(relations.first().roleId, roleId);
    }

    void testFindRelationsForPerson() {
        auto personId = insertPerson();
        auto typeId = insertEventType();
        auto roleId = insertEventRole();
        EventRepository repo;
        auto eventId1 = repo.insertEvent(typeId);
        auto eventId2 = repo.insertEvent(typeId);
        QVERIFY(eventId1.has_value());
        QVERIFY(eventId2.has_value());

        repo.insertEventRelation(*eventId1, personId, roleId);
        repo.insertEventRelation(*eventId2, personId, roleId);

        auto relations = repo.findRelationsForPerson(personId);
        QCOMPARE(relations.size(), 2);
    }

    void testDeleteEventRelation() {
        auto personId = insertPerson();
        auto typeId = insertEventType();
        auto roleId = insertEventRole();
        EventRepository repo;
        auto eventId = repo.insertEvent(typeId);
        QVERIFY(eventId.has_value());

        repo.insertEventRelation(*eventId, personId, roleId);
        bool ok = repo.deleteEventRelation(*eventId, personId, roleId);
        QVERIFY(ok);

        auto relations = repo.findRelationsForEvent(*eventId);
        QVERIFY(relations.isEmpty());
    }

    void testFindRelationsForEventEmpty() {
        auto typeId = insertEventType();
        EventRepository repo;
        auto eventId = repo.insertEvent(typeId);
        QVERIFY(eventId.has_value());

        auto relations = repo.findRelationsForEvent(*eventId);
        QVERIFY(relations.isEmpty());
    }
};

QTEST_MAIN(TestEventRepository)

#include "event_repository_test.moc"
