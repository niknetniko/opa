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
#include "database/schema.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestFamilyRepository : public QObject {
    Q_OBJECT

    IntegerPrimaryKey addPerson(const QString& firstName, const QString& lastName, const QString& sex) {
        auto personId = insertQuery(u"INSERT INTO people (root, sex) VALUES (false, '%1')"_s.arg(sex));
        QSqlQuery query;
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, '%2', '%3')"_s
                           .arg(personId)
                           .arg(firstName, lastName)),
            query
        );
        return personId;
    }

    struct FamilySetup {
        IntegerPrimaryKey fatherId;
        IntegerPrimaryKey motherId;
        IntegerPrimaryKey childId;
    };

    FamilySetup addParentAndChild() {
        QSqlQuery query;

        auto marriageTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Marriage'"_s);
        auto primaryRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Primary'"_s);
        auto partnerRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Partner'"_s);
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto motherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Mother'"_s);
        auto fatherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Father'"_s);

        auto fatherId = addPerson(u"Bob"_s, u"Bober"_s, u"Male"_s);
        auto motherId = addPerson(u"Alice"_s, u"English"_s, u"Female"_s);

        auto marriageEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(marriageTypeId));
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(marriageEvent)
                           .arg(fatherId)
                           .arg(primaryRoleId)),
            query
        );
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(marriageEvent)
                           .arg(motherId)
                           .arg(partnerRoleId)),
            query
        );

        auto childId = addPerson(u"Child"_s, u"Bober"_s, u"Female"_s);
        auto birthEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(birthEvent)
                           .arg(childId)
                           .arg(primaryRoleId)),
            query
        );
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(birthEvent)
                           .arg(fatherId)
                           .arg(fatherRoleId)),
            query
        );
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                           .arg(birthEvent)
                           .arg(motherId)
                           .arg(motherRoleId)),
            query
        );

        auto familyId = insertQuery(u"INSERT INTO families DEFAULT VALUES"_s);
        VERIFY_OR_THROW2(
            query.exec(
                u"UPDATE events SET family_id = %1 WHERE id IN (%2, %3)"_s.arg(familyId).arg(marriageEvent).arg(birthEvent)
            ),
            query
        );

        return {fatherId, motherId, childId};
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

    void testFindFamilyMembersForPersonReturnsMembersAndChildren() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto members = repo.findFamilyMembersForPerson(fatherId);

        // Should have one marriage entry and one child entry.
        QCOMPARE(members.size(), 2);
    }

    void testFindFamilyMembersForPersonWithNoFamily() {
        auto personId = addPerson(u"Lone"_s, u"Wolf"_s, u"Unknown"_s);

        FamilyRepository repo;
        auto members = repo.findFamilyMembersForPerson(personId);

        QVERIFY(members.isEmpty());
    }

    void testFindFamilyMembersContainsPartnerPersonId() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto members = repo.findFamilyMembersForPerson(fatherId);

        bool foundPartner = false;
        for (const auto& member: members) {
            if (member.personId == motherId) {
                foundPartner = true;
                break;
            }
        }
        QVERIFY(foundPartner);
    }

    void testFindAncestorsForPersonReturnsAll() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto ancestors = repo.findAncestorsForPerson(childId);

        // Child itself plus father and mother.
        QCOMPARE(ancestors.size(), 3);
    }

    void testFindAncestorsForPersonWithNoParents() {
        auto personId = addPerson(u"Root"_s, u"Person"_s, u"Unknown"_s);

        FamilyRepository repo;
        auto ancestors = repo.findAncestorsForPerson(personId);

        QCOMPARE(ancestors.size(), 1);
        QCOMPARE(ancestors.first().childId, personId);
        QVERIFY(!ancestors.first().fatherId.has_value());
        QVERIFY(!ancestors.first().motherId.has_value());
    }

    void testFindAncestorsForPersonChildHasCorrectParents() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto ancestors = repo.findAncestorsForPerson(childId);

        // First result should be the child itself.
        QCOMPARE(ancestors.first().childId, childId);
        QVERIFY(ancestors.first().fatherId.has_value());
        QCOMPARE(*ancestors.first().fatherId, fatherId);
        QVERIFY(ancestors.first().motherId.has_value());
        QCOMPARE(*ancestors.first().motherId, motherId);
    }

    void testFindParentsForPersonReturnsBothParents() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto parents = repo.findParentsForPerson(childId);

        QCOMPARE(parents.size(), 2);
    }

    void testFindParentsForPersonWithNoParents() {
        auto personId = addPerson(u"Orphan"_s, u"Child"_s, u"Unknown"_s);

        FamilyRepository repo;
        auto parents = repo.findParentsForPerson(personId);

        QVERIFY(parents.isEmpty());
    }

    void testFindParentsForPersonContainsFatherAndMother() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto parents = repo.findParentsForPerson(childId);

        QList<IntegerPrimaryKey> parentIds;
        for (const auto& parent: parents) {
            parentIds.append(parent.personId);
        }
        QVERIFY(parentIds.contains(fatherId));
        QVERIFY(parentIds.contains(motherId));
    }

    void testFindParentsRolesAreSet() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto parents = repo.findParentsForPerson(childId);

        QStringList roles;
        for (const auto& parent: parents) {
            roles.append(parent.role);
        }
        QVERIFY(roles.contains(u"Father"_s));
        QVERIFY(roles.contains(u"Mother"_s));
    }

    void testFindParentsNameDataIsPopulated() {
        auto [fatherId, motherId, childId] = addParentAndChild();

        FamilyRepository repo;
        auto parents = repo.findParentsForPerson(childId);

        for (const auto& parent: parents) {
            QVERIFY(!parent.givenNames.isEmpty());
            QVERIFY(!parent.surname.isEmpty());
        }
    }

    void testCreateFamilyReturnsId() {
        FamilyRepository repo;
        auto id = repo.createFamily();
        QVERIFY(id.has_value());
        QVERIFY(*id > 0);
    }

    void testCreateFamilyCreatesRecord() {
        FamilyRepository repo;
        auto id = repo.createFamily();
        QVERIFY(id.has_value());

        QSqlQuery q;
        QVERIFY(q.exec(u"SELECT COUNT(*) FROM families WHERE id = %1"_s.arg(*id)));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
    }

    void testCreateFamilyIdsAreUnique() {
        FamilyRepository repo;
        auto id1 = repo.createFamily();
        auto id2 = repo.createFamily();
        QVERIFY(id1.has_value());
        QVERIFY(id2.has_value());
        QVERIFY(*id1 != *id2);
    }

    void testLinkEventToFamilyReturnsTrueOnSuccess() {
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto eventId = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));
        FamilyRepository repo;
        auto familyId = repo.createFamily();
        QVERIFY(familyId.has_value());

        QVERIFY(repo.linkEventToFamily(eventId, *familyId));
    }

    void testLinkEventToFamilyUpdatesEvent() {
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto eventId = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));
        FamilyRepository repo;
        auto familyId = repo.createFamily();
        QVERIFY(familyId.has_value());
        repo.linkEventToFamily(eventId, *familyId);

        QSqlQuery q;
        QVERIFY(q.exec(u"SELECT family_id FROM events WHERE id = %1"_s.arg(eventId)));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toLongLong(), *familyId);
    }
};

QTEST_MAIN(TestFamilyRepository)

#include "family_repository_test.moc"
