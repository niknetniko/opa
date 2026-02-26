/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "./test_utils.h"
#include "domain/family/parents_model.h"
#include "database/database.h"

#include <QAbstractItemModelTester>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestParentsModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));

        openDatabase(u":memory:"_s, true);

        QSqlQuery query;
        QVERIFY(query.exec(u"SELECT COUNT(*) FROM people"_s));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 9);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    void testRootHasParents() {
        ParentsModel model{1};

        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, ParentsModel::PERSON_ID).data(), 3);
        QCOMPARE(model.index(0, ParentsModel::ROLE).data(), u"Father"_s);

        QCOMPARE(model.index(1, ParentsModel::PERSON_ID).data(), 4);
        QCOMPARE(model.index(1, ParentsModel::ROLE).data(), u"Mother"_s);
    }

    void testSiblingHasSameParents() {
        ParentsModel model{2};

        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, ParentsModel::PERSON_ID).data(), 3);
        QCOMPARE(model.index(0, ParentsModel::ROLE).data(), u"Father"_s);

        QCOMPARE(model.index(1, ParentsModel::PERSON_ID).data(), 4);
        QCOMPARE(model.index(1, ParentsModel::ROLE).data(), u"Mother"_s);
    }

    void testParentHasParents() {
        ParentsModel model{3};

        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, ParentsModel::PERSON_ID).data(), 5);
        QCOMPARE(model.index(0, ParentsModel::ROLE).data(), u"Father"_s);

        QCOMPARE(model.index(1, ParentsModel::PERSON_ID).data(), 6);
        QCOMPARE(model.index(1, ParentsModel::ROLE).data(), u"Mother"_s);
    }

    void testGrandParentsHaveParents() {
        ParentsModel model{6};

        QCOMPARE(model.rowCount(), 1);

        QCOMPARE(model.index(0, ParentsModel::PERSON_ID).data(), 9);
        QCOMPARE(model.index(0, ParentsModel::ROLE).data(), u"Father"_s);
    }
};

QTEST_MAIN(TestParentsModel)

#include "parent_query_model_test.moc"
