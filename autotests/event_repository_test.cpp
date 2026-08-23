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

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    // Event tests

    void testInsertAndFindEventById() {
        EventRepository repo;
        auto id = repo.insertEvent(u"Birth"_s);
        QVERIFY(id.has_value());

        auto result = repo.findEventById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->type, u"Birth"_s);
        QVERIFY(result->date.isEmpty());
        QVERIFY(result->name.isEmpty());
        QVERIFY(result->note.isEmpty());
    }

    void testUpdateEvent() {
        EventRepository repo;
        auto id = repo.insertEvent(u"Birth"_s);
        QVERIFY(id.has_value());

        bool ok = repo.updateEvent(*id, u"Birth"_s, u"1900-01-01"_s, u"Test event"_s, u"A note"_s);
        QVERIFY(ok);

        auto result = repo.findEventById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->date, u"1900-01-01"_s);
        QCOMPARE(result->name, u"Test event"_s);
        QCOMPARE(result->note, u"A note"_s);
    }

    void testDeleteEvent() {
        EventRepository repo;
        auto id = repo.insertEvent(u"Birth"_s);
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
        EventRepository repo;
        auto eventId = repo.insertEvent(u"Birth"_s);
        QVERIFY(eventId.has_value());

        auto relationId = repo.insertEventRelation(*eventId, personId, u"Primary"_s);
        QVERIFY(relationId.has_value());

        auto relations = repo.findRelationsForEvent(*eventId);
        QCOMPARE(relations.size(), 1);
        QCOMPARE(relations.first().id, *relationId);
        QCOMPARE(relations.first().eventId, *eventId);
        QCOMPARE(relations.first().personId, personId);
        QCOMPARE(relations.first().role, u"Primary"_s);
    }

    void testFindRelationsForPerson() {
        auto personId = insertPerson();
        EventRepository repo;
        auto eventId1 = repo.insertEvent(u"Birth"_s);
        auto eventId2 = repo.insertEvent(u"Death"_s);
        QVERIFY(eventId1.has_value());
        QVERIFY(eventId2.has_value());

        repo.insertEventRelation(*eventId1, personId, u"Primary"_s);
        repo.insertEventRelation(*eventId2, personId, u"Primary"_s);

        auto relations = repo.findRelationsForPerson(personId);
        QCOMPARE(relations.size(), 2);
    }

    void testDeleteEventRelation() {
        auto personId = insertPerson();
        EventRepository repo;
        auto eventId = repo.insertEvent(u"Birth"_s);
        QVERIFY(eventId.has_value());

        auto relationId = repo.insertEventRelation(*eventId, personId, u"Primary"_s);
        QVERIFY(relationId.has_value());
        bool ok = repo.deleteEventRelation(*relationId);
        QVERIFY(ok);

        auto relations = repo.findRelationsForEvent(*eventId);
        QVERIFY(relations.isEmpty());
    }

    void testFindRelationsForEventEmpty() {
        EventRepository repo;
        auto eventId = repo.insertEvent(u"Birth"_s);
        QVERIFY(eventId.has_value());

        auto relations = repo.findRelationsForEvent(*eventId);
        QVERIFY(relations.isEmpty());
    }

    void testUpdateEventRelationRole() {
        auto personId = insertPerson();
        EventRepository repo;
        auto eventId = repo.insertEvent(u"Birth"_s);
        QVERIFY(eventId.has_value());

        auto relationId = repo.insertEventRelation(*eventId, personId, u"Primary"_s);
        QVERIFY(relationId.has_value());

        bool ok = repo.updateEventRelationRole(*relationId, u"Witness"_s);
        QVERIFY(ok);

        auto relations = repo.findRelationsForEvent(*eventId);
        QCOMPARE(relations.size(), 1);
        QCOMPARE(relations.first().role, u"Witness"_s);
        QCOMPARE(relations.first().id, *relationId);
    }

    void testCitationsSurviveRoleChange() {
        auto personId = insertPerson();
        auto sourceId = insertQuery(u"INSERT INTO sources (title, type, confidence) VALUES ('Test', '', 3)"_s);

        EventRepository repo;
        auto eventId = repo.insertEvent(u"Birth"_s);
        QVERIFY(eventId.has_value());

        auto relationId = repo.insertEventRelation(*eventId, personId, u"Primary"_s);
        QVERIFY(relationId.has_value());

        QVERIFY(repo.addEventRelationCitation(*relationId, sourceId));

        // Change role.
        QVERIFY(repo.updateEventRelationRole(*relationId, u"Witness"_s));

        // Citations should still be there.
        auto citations = repo.findCitationsForEventRelation(*relationId);
        QCOMPARE(citations.size(), 1);
        QCOMPARE(citations.first().id, sourceId);
    }
};

QTEST_MAIN(TestEventRepository)

#include "event_repository_test.moc"
