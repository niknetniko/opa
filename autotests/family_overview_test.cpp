/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "../src/domain/family/family_repository.h"
#include "./test_utils.h"
#include "database/database.h"

#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestFamilyOverview : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, true);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    void testReturnsRowsForAllFamilies() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();
        QVERIFY(!rows.isEmpty());

        QSet<IntegerPrimaryKey> familyIds;
        for (const auto& row: rows) {
            familyIds.insert(row.familyId);
        }
        QCOMPARE(familyIds.size(), 4);
    }

    void testFamilyIdsAreFromSeed() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        QSet<IntegerPrimaryKey> familyIds;
        for (const auto& row: rows) {
            familyIds.insert(row.familyId);
        }
        QVERIFY(familyIds.contains(1));
        QVERIFY(familyIds.contains(2));
        QVERIFY(familyIds.contains(3));
        QVERIFY(familyIds.contains(4));
    }

    void testDisplayNameForTwoParentFamily() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        // Family 1: Michael Doe (person 3) + Emily Smith (person 4), sorted by person_id
        QString displayName;
        for (const auto& row: rows) {
            if (row.familyId == 1) {
                displayName = row.familyDisplayName;
                break;
            }
        }
        QVERIFY(!displayName.isEmpty());
        QVERIFY(displayName.contains(u"Doe"_s));
        QVERIFY(displayName.contains(u"Smith"_s));
        // Exactly "Doe — Smith" in person_id order (person 3 before person 4)
        QCOMPARE(displayName, u"Doe — Smith"_s);
    }

    void testDisplayNameForSingleParentFamily() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        // Family 4: only Ebenezer (person 9, surname "No name")
        QString displayName;
        for (const auto& row: rows) {
            if (row.familyId == 4) {
                displayName = row.familyDisplayName;
                break;
            }
        }
        QCOMPARE(displayName, u"No name"_s);
    }

    void testDisplayNameFallbackForFamilyWithNoParents() {
        // Create a family with a marriage event but no birth events, so no Father/Mother roles exist.
        auto marriageTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Marriage'"_s);
        auto primaryRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Primary'"_s);
        auto partnerRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Partner'"_s);
        auto p1 = insertQuery(u"INSERT INTO people (root, sex) VALUES (false, 'Male')"_s);
        auto p2 = insertQuery(u"INSERT INTO people (root, sex) VALUES (false, 'Female')"_s);
        auto familyId = insertQuery(u"INSERT INTO families DEFAULT VALUES"_s);
        auto marriageEvent =
            insertQuery(u"INSERT INTO events (type_id, family_id) VALUES (%1, %2)"_s.arg(marriageTypeId).arg(familyId));

        QSqlQuery q;
        QVERIFY(q.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(marriageEvent)
                           .arg(p1)
                           .arg(primaryRoleId)));
        QVERIFY(q.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(marriageEvent)
                           .arg(p2)
                           .arg(partnerRoleId)));

        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        QString displayName;
        for (const auto& row: rows) {
            if (row.familyId == familyId) {
                displayName = row.familyDisplayName;
                break;
            }
        }
        QCOMPARE(displayName, u"Family #%1"_s.arg(familyId));
    }

    void testOnlyBirthAndMarriageEventsReturned() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        for (const auto& row: rows) {
            QVERIFY(row.eventType == u"Birth"_s || row.eventType == u"Marriage"_s);
        }
    }

    void testDeathEventsNotReturned() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        for (const auto& row: rows) {
            QVERIFY(row.eventType != u"Death"_s);
        }
    }

    void testPersonIdsArePopulated() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        for (const auto& row: rows) {
            QVERIFY(row.personId > 0);
        }
    }

    void testEventIdsArePopulated() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        for (const auto& row: rows) {
            QVERIFY(row.eventId > 0);
        }
    }

    void testMarriageEventAppearsBeforeBirthsWithinFamily() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        // Within each family, collect event types in order.
        QHash<IntegerPrimaryKey, QStringList> eventTypesByFamily;
        for (const auto& row: rows) {
            if (!eventTypesByFamily[row.familyId].contains(row.eventType)) {
                eventTypesByFamily[row.familyId].append(row.eventType);
            }
        }

        // For families that have both marriages and births, marriages come first.
        for (auto it = eventTypesByFamily.constBegin(); it != eventTypesByFamily.constEnd(); ++it) {
            const auto& types = it.value();
            if (types.contains(u"Marriage"_s) && types.contains(u"Birth"_s)) {
                QCOMPARE(types.first(), u"Marriage"_s);
            }
        }
    }

    void testRoleIsPopulated() {
        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();

        for (const auto& row: rows) {
            QVERIFY(!row.role.isEmpty());
        }
    }

    void testEmptyDatabaseReturnsNoRows() {
        QSqlDatabase::database().close();
        openDatabase(u":memory:"_s, false);

        FamilyRepository repo;
        auto rows = repo.findAllFamiliesOverview();
        QVERIFY(rows.isEmpty());
    }
};

QTEST_MAIN(TestFamilyOverview)

#include "family_overview_test.moc"
