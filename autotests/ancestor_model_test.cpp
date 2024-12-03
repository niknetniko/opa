/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "./test_utils.h"
#include "data/family.h"
#include "data/names.h"
#include "database/database.h"

#include <QAbstractItemModelTester>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestAncestorModel : public QObject {
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

    void testRootPersonGetsAllAncestors() {
        AncestorQueryModel model{1};

        QCOMPARE(model.rowCount(), 8);

        QCOMPARE(model.index(0, AncestorQueryModel::CHILD_ID).data(), 1);
        QCOMPARE(model.index(0, AncestorQueryModel::FATHER_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorQueryModel::MOTHER_ID).data(), 4);

        QCOMPARE(model.index(1, AncestorQueryModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(1, AncestorQueryModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(1, AncestorQueryModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(2, AncestorQueryModel::CHILD_ID).data(), 4);
        QCOMPARE(model.index(2, AncestorQueryModel::FATHER_ID).data(), 7);
        QCOMPARE(model.index(2, AncestorQueryModel::MOTHER_ID).data(), 8);

        QCOMPARE(model.index(3, AncestorQueryModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(3, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(4, AncestorQueryModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(4, AncestorQueryModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(4, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(5, AncestorQueryModel::CHILD_ID).data(), 7);
        QCOMPARE(model.index(5, AncestorQueryModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(5, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(6, AncestorQueryModel::CHILD_ID).data(), 8);
        QVERIFY(model.index(6, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(6, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(7, AncestorQueryModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(7, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(7, AncestorQueryModel::MOTHER_ID).data().isNull());
    }

    void testSiblingGetsAllAncestors() {
        AncestorQueryModel model{2};

        QCOMPARE(model.rowCount(), 8);

        QCOMPARE(model.index(0, AncestorQueryModel::CHILD_ID).data(), 2);
        QCOMPARE(model.index(0, AncestorQueryModel::FATHER_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorQueryModel::MOTHER_ID).data(), 4);

        QCOMPARE(model.index(1, AncestorQueryModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(1, AncestorQueryModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(1, AncestorQueryModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(2, AncestorQueryModel::CHILD_ID).data(), 4);
        QCOMPARE(model.index(2, AncestorQueryModel::FATHER_ID).data(), 7);
        QCOMPARE(model.index(2, AncestorQueryModel::MOTHER_ID).data(), 8);

        QCOMPARE(model.index(3, AncestorQueryModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(3, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(4, AncestorQueryModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(4, AncestorQueryModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(4, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(5, AncestorQueryModel::CHILD_ID).data(), 7);
        QCOMPARE(model.index(5, AncestorQueryModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(5, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(6, AncestorQueryModel::CHILD_ID).data(), 8);
        QVERIFY(model.index(6, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(6, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(7, AncestorQueryModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(7, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(7, AncestorQueryModel::MOTHER_ID).data().isNull());
    }

    void testParentGetsLessAncestors() {
        AncestorQueryModel model{3};

        QCOMPARE(model.rowCount(), 4);

        QCOMPARE(model.index(0, AncestorQueryModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorQueryModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(0, AncestorQueryModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(1, AncestorQueryModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(1, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(1, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(2, AncestorQueryModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(2, AncestorQueryModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(2, AncestorQueryModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(3, AncestorQueryModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(3, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorQueryModel::MOTHER_ID).data().isNull());
    }

    void testGrandParentGetsNoAncestors() {
        AncestorQueryModel model{5};

        QCOMPARE(model.rowCount(), 1);

        QCOMPARE(model.index(0, AncestorQueryModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(0, AncestorQueryModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(0, AncestorQueryModel::MOTHER_ID).data().isNull());
    }
};

QTEST_MAIN(TestAncestorModel)

#include "ancestor_model_test.moc"
