/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/domain/location/location_repository.h"

#include "./test_utils.h"
#include "database/database.h"
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestLocationRepository : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    // LocationType tests

    void testInsertAndFindLocationTypeById() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        auto result = repo.findLocationTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->type, u"City"_s);
        QCOMPARE(result->builtin, false);
    }

    void testFindAllLocationTypes() {
        LocationRepository repo;
        const auto before = repo.findAllLocationTypes().size();
        repo.insertLocationType(u"TypeA"_s);
        repo.insertLocationType(u"TypeB"_s);

        auto types = repo.findAllLocationTypes();
        QCOMPARE(types.size(), before + 2);
    }

    void testUpdateLocationType() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        QVERIFY(repo.updateLocationType(*id, u"Town"_s));

        auto result = repo.findLocationTypeById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->type, u"Town"_s);
    }

    void testDeleteLocationType() {
        LocationRepository repo;
        auto id = repo.insertLocationType(u"City"_s);
        QVERIFY(id.has_value());

        QVERIFY(repo.deleteLocationType(*id));
        QVERIFY(!repo.findLocationTypeById(*id).has_value());
    }

    void testIsLocationTypeUsed() {
        LocationRepository repo;
        auto typeId = repo.insertLocationType(u"City"_s);
        QVERIFY(typeId.has_value());

        QVERIFY(!repo.isLocationTypeUsed(*typeId));

        auto locId = repo.insert(u"Amsterdam"_s, *typeId, std::nullopt);
        QVERIFY(locId.has_value());

        QVERIFY(repo.isLocationTypeUsed(*typeId));
    }

    // Location tests

    void testInsertAndFindById() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        auto result = repo.findById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->id, *id);
        QCOMPARE(result->name, u"Netherlands"_s);
        QVERIFY(!result->typeId.has_value());
        QVERIFY(!result->parentId.has_value());
    }

    void testInsertWithParent() {
        LocationRepository repo;
        auto parentId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(parentId.has_value());

        auto childId = repo.insert(u"Groningen"_s, std::nullopt, *parentId);
        QVERIFY(childId.has_value());

        auto result = repo.findById(*childId);
        QVERIFY(result.has_value());
        QVERIFY(result->parentId.has_value());
        QCOMPARE(*result->parentId, *parentId);
    }

    void testUpdate() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        QVERIFY(repo.update(*id, u"The Netherlands"_s, std::nullopt, std::nullopt,
                            u"A note"_s, Coordinates{52.3, 4.9}, u"1815"_s, QString{}));

        auto result = repo.findById(*id);
        QVERIFY(result.has_value());
        QCOMPARE(result->name, u"The Netherlands"_s);
        QCOMPARE(result->note, u"A note"_s);
        QVERIFY(result->coordinates.has_value());
        QCOMPARE(result->coordinates->latitude, 52.3);
        QCOMPARE(result->coordinates->longitude, 4.9);
        QCOMPARE(result->dateStart, u"1815"_s);
        QVERIFY(result->dateEnd.isEmpty());
    }

    void testDeleteLocation() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        QVERIFY(repo.deleteLocation(*id));
        QVERIFY(!repo.findById(*id).has_value());
    }

    void testIsUsed() {
        LocationRepository repo;
        auto locId = repo.insert(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(locId.has_value());
        QVERIFY(!repo.isUsed(*locId));

        auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Birth', false)"_s);
        QSqlQuery q;
        q.prepare(u"INSERT INTO events (type_id, location_id) VALUES (:type_id, :location_id)"_s);
        q.bindValue(u":type_id"_s, typeId);
        q.bindValue(u":location_id"_s, *locId);
        VERIFY_OR_THROW2(q.exec(), q);

        QVERIFY(repo.isUsed(*locId));
    }

    void testFindAllWithPaths_singleRoot() {
        LocationRepository repo;
        auto id = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());

        auto paths = repo.findAllWithPaths();
        auto it = std::find_if(paths.begin(), paths.end(), [&](const auto& e) { return e.id == *id; });
        QVERIFY(it != paths.end());
        QCOMPARE(it->fullPath, u"Netherlands"_s);
    }

    void testFindAllWithPaths_childPath() {
        LocationRepository repo;
        auto nlId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(nlId.has_value());
        auto grId = repo.insert(u"Groningen"_s, std::nullopt, *nlId);
        QVERIFY(grId.has_value());

        auto paths = repo.findAllWithPaths();
        auto it = std::find_if(paths.begin(), paths.end(), [&](const auto& e) { return e.id == *grId; });
        QVERIFY(it != paths.end());
        QCOMPARE(it->fullPath, u"Netherlands > Groningen"_s);
    }

    void testFindOrCreate_createsWhenMissing() {
        LocationRepository repo;
        auto id = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(id.has_value());
        QVERIFY(repo.findById(*id).has_value());
    }

    void testFindOrCreate_findsExisting() {
        LocationRepository repo;
        auto id1 = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        auto id2 = repo.findOrCreate(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());
        QCOMPARE(*id1, *id2);
    }

    void testFindOrCreate_typeIdIgnored() {
        // findOrCreate matches on (name, parent_id) only — typeId is intentionally not used.
        LocationRepository repo;
        auto typeId1 = repo.insertLocationType(u"City"_s);
        auto typeId2 = repo.insertLocationType(u"Village"_s);
        QVERIFY(typeId1.has_value());
        QVERIFY(typeId2.has_value());

        auto id1 = repo.findOrCreate(u"Amsterdam"_s, typeId1, std::nullopt);
        auto id2 = repo.findOrCreate(u"Amsterdam"_s, typeId2, std::nullopt);
        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());
        QCOMPARE(*id1, *id2);
    }

    void testDeleteLocationUsedByEvent_setsNullOnEvent() {
        // Verifies ON DELETE SET NULL: deleting a location nullifies event.location_id.
        LocationRepository repo;
        auto locId = repo.insert(u"Amsterdam"_s, std::nullopt, std::nullopt);
        QVERIFY(locId.has_value());

        auto typeId = insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('Birth', false)"_s);
        QSqlQuery insertEvent;
        insertEvent.prepare(u"INSERT INTO events (type_id, location_id) VALUES (:type_id, :location_id)"_s);
        insertEvent.bindValue(u":type_id"_s, typeId);
        insertEvent.bindValue(u":location_id"_s, *locId);
        VERIFY_OR_THROW2(insertEvent.exec(), insertEvent);
        auto eventId = insertEvent.lastInsertId().toInt();

        QVERIFY(repo.isUsed(*locId));
        QVERIFY(repo.deleteLocation(*locId));

        QSqlQuery selectEvent;
        selectEvent.prepare(u"SELECT COALESCE(location_id, -1) FROM events WHERE id = :id"_s);
        selectEvent.bindValue(u":id"_s, eventId);
        VERIFY_OR_THROW2(selectEvent.exec(), selectEvent);
        VERIFY_OR_THROW(selectEvent.next());
        const auto nulledLocation = selectEvent.value(0).toInt();
        QCOMPARE(nulledLocation, -1);
    }

    void testDeleteParentLocation_childGetsNullParent() {
        // Verifies ON DELETE SET NULL: deleting a parent location makes children parentless.
        LocationRepository repo;
        auto parentId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(parentId.has_value());
        auto childId = repo.insert(u"Groningen"_s, std::nullopt, *parentId);
        QVERIFY(childId.has_value());

        QVERIFY(repo.deleteLocation(*parentId));

        auto child = repo.findById(*childId);
        QVERIFY(child.has_value());
        QVERIFY(!child->parentId.has_value());
    }

    void testFindAllWithPaths_deepHierarchy() {
        LocationRepository repo;
        auto nlId = repo.insert(u"Netherlands"_s, std::nullopt, std::nullopt);
        QVERIFY(nlId.has_value());
        auto grId = repo.insert(u"Groningen"_s, std::nullopt, *nlId);
        QVERIFY(grId.has_value());
        auto ccId = repo.insert(u"City Centre"_s, std::nullopt, *grId);
        QVERIFY(ccId.has_value());

        auto paths = repo.findAllWithPaths();
        auto it = std::find_if(paths.begin(), paths.end(), [&](const auto& e) { return e.id == *ccId; });
        QVERIFY(it != paths.end());
        QCOMPARE(it->fullPath, u"Netherlands > Groningen > City Centre"_s);
    }

    void testFindLocationTypeByIdNotFound() {
        LocationRepository repo;
        QVERIFY(!repo.findLocationTypeById(9999).has_value());
    }

    void testFindByIdNotFound() {
        LocationRepository repo;
        QVERIFY(!repo.findById(9999).has_value());
    }
};

QTEST_MAIN(TestLocationRepository)

#include "location_repository_test.moc"
