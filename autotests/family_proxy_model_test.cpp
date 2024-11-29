/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "./test_utils.h"
#include "data/data_manager.h"
#include "data/event.h"
#include "data/family.h"
#include "data/names.h"
#include "database/database.h"
#include "database/schema.h"

#include <QAbstractItemModelTester>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestFamilyProxyModel : public QObject {
    Q_OBJECT

private:
    IntegerPrimaryKey addPerson(const QString& firstName, const QString& lastName, const QString& sex) {
        QSqlQuery query;
        auto personId = insertQuery(u"INSERT INTO people (root, sex) VALUES (true, '%1')"_s.arg(sex));
        VERIFY_OR_THROW2(
            query.exec(u"INSERT INTO names (person_id, sort, given_names, surname) VALUES (%1, 1, '%2', '%3')"_s
                           .arg(personId)
                           .arg(firstName, lastName)),
            query
        );
        return personId;
    }

    void addParentAndChild() {
        QSqlQuery query;

        // Query some data we need.
        auto marriageTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Marriage'"_s);
        auto primaryRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Primary'"_s);
        auto partnerRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Partner'"_s);
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto motherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Mother'"_s);
        auto fatherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Father'"_s);

        // Add two people.
        auto fatherId = addPerson(u"Bob"_s, u"Bober"_s, u"Male"_s);
        auto motherId = addPerson(u"Alice"_s, u"English"_s, u"Female"_s);

        // Marry them.
        auto marriageEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(marriageTypeId));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(marriageEvent)
                               .arg(fatherId)
                               .arg(primaryRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(marriageEvent)
                               .arg(motherId)
                               .arg(partnerRoleId)));

        // Add a child.
        auto childId = addPerson(u"Child"_s, u"Bober"_s, u"Female"_s);
        auto birthEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));

        qDebug() << "Father has ID" << fatherId;

        // Link the child.
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(childId)
                               .arg(primaryRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(fatherId)
                               .arg(fatherRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(motherId)
                               .arg(motherRoleId)));
    }

    void addBastardChild() {
        QSqlQuery query;
        // Add a bastard child.
        auto bastardChildId = addPerson(u"Bastard"_s, u"Bober"_s, u"Female"_s);
        auto birthTypeId = selectQuery(u"SELECT id FROM event_types WHERE type = 'Birth'"_s);
        auto birthEvent = insertQuery(u"INSERT INTO events (type_id) VALUES (%1)"_s.arg(birthTypeId));
        auto fatherRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Father'"_s);
        auto primaryRoleId = selectQuery(u"SELECT id FROM event_roles WHERE role = 'Primary'"_s);
        // Link it to the parent.
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(1)
                               .arg(fatherRoleId)));
        QVERIFY(query.exec(u"INSERT INTO event_relations (event_id, person_id, role_id) VALUES (%1, %2, %3)"_s
                               .arg(birthEvent)
                               .arg(bastardChildId)
                               .arg(primaryRoleId)));
    }

private Q_SLOTS:
    void init() {
        // Without this, we cannot test anything.
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        // Open the database.
        openDatabase(u":memory:"_s, false);
        addParentAndChild();
        DataManager::initialize(this);
    }

    void cleanup() {
        DataManager::reset();
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testDefaultCaseBasics() {
        auto* model = DataManager::get().familyModelFor(this, 1);
        QCOMPARE(model->rowCount(), 1);

        auto firstParentIndex = model->index(0, FamilyProxyModel::PERSON_ID);
        QCOMPARE(firstParentIndex.data(), 2);

        // Only the first column has parents.
        QCOMPARE(model->rowCount(model->index(0, 3)), 0);
        QCOMPARE(model->rowCount(model->index(0, 0)), 1);
    }

    void testDefaultCaseWithModelTester() {
        auto* model = DataManager::get().familyModelFor(this, 1);
        new QAbstractItemModelTester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testBastardCaseBasics() {
        addBastardChild();

        auto* model = DataManager::get().familyModelFor(this, 1);
        QCOMPARE(model->rowCount(), 2);

        // The ID of the first parent.
        QCOMPARE(model->index(0, FamilyProxyModel::PERSON_ID).data(), 2);
        // The parent of the bastard children.
        QCOMPARE(model->index(1, FamilyProxyModel::PERSON_ID).data(), QVariant{});

        QCOMPARE(model->rowCount(model->index(0, 0)), 1);
        QCOMPARE(model->rowCount(model->index(1, 0)), 1);
    }

    void testBastardCaseWithModelTester() {
        addBastardChild();
        auto* model = DataManager::get().familyModelFor(this, 1);
        new QAbstractItemModelTester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_MAIN(TestFamilyProxyModel)

#include "family_proxy_model_test.moc"
