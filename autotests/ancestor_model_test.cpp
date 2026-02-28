/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "domain/family/ancestor_model.h"

#include "../src/domain/name/names.h"
#include "./test_utils.h"
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
        AncestorModel model{1};

        QCOMPARE(model.rowCount(), 8);

        QCOMPARE(model.index(0, AncestorModel::CHILD_ID).data(), 1);
        QCOMPARE(model.index(0, AncestorModel::FATHER_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorModel::MOTHER_ID).data(), 4);

        QCOMPARE(model.index(1, AncestorModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(1, AncestorModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(1, AncestorModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(2, AncestorModel::CHILD_ID).data(), 4);
        QCOMPARE(model.index(2, AncestorModel::FATHER_ID).data(), 7);
        QCOMPARE(model.index(2, AncestorModel::MOTHER_ID).data(), 8);

        QCOMPARE(model.index(3, AncestorModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(3, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(4, AncestorModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(4, AncestorModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(4, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(5, AncestorModel::CHILD_ID).data(), 7);
        QCOMPARE(model.index(5, AncestorModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(5, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(6, AncestorModel::CHILD_ID).data(), 8);
        QVERIFY(model.index(6, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(6, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(7, AncestorModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(7, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(7, AncestorModel::MOTHER_ID).data().isNull());
    }

    void testSiblingGetsAllAncestors() {
        AncestorModel model{2};

        QCOMPARE(model.rowCount(), 8);

        QCOMPARE(model.index(0, AncestorModel::CHILD_ID).data(), 2);
        QCOMPARE(model.index(0, AncestorModel::FATHER_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorModel::MOTHER_ID).data(), 4);

        QCOMPARE(model.index(1, AncestorModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(1, AncestorModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(1, AncestorModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(2, AncestorModel::CHILD_ID).data(), 4);
        QCOMPARE(model.index(2, AncestorModel::FATHER_ID).data(), 7);
        QCOMPARE(model.index(2, AncestorModel::MOTHER_ID).data(), 8);

        QCOMPARE(model.index(3, AncestorModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(3, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(4, AncestorModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(4, AncestorModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(4, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(5, AncestorModel::CHILD_ID).data(), 7);
        QCOMPARE(model.index(5, AncestorModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(5, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(6, AncestorModel::CHILD_ID).data(), 8);
        QVERIFY(model.index(6, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(6, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(7, AncestorModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(7, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(7, AncestorModel::MOTHER_ID).data().isNull());
    }

    void testParentGetsLessAncestors() {
        AncestorModel model{3};

        QCOMPARE(model.rowCount(), 4);

        QCOMPARE(model.index(0, AncestorModel::CHILD_ID).data(), 3);
        QCOMPARE(model.index(0, AncestorModel::FATHER_ID).data(), 5);
        QCOMPARE(model.index(0, AncestorModel::MOTHER_ID).data(), 6);

        QCOMPARE(model.index(1, AncestorModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(1, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(1, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(2, AncestorModel::CHILD_ID).data(), 6);
        QCOMPARE(model.index(2, AncestorModel::FATHER_ID).data(), 9);
        QVERIFY(model.index(2, AncestorModel::MOTHER_ID).data().isNull());

        QCOMPARE(model.index(3, AncestorModel::CHILD_ID).data(), 9);
        QVERIFY(model.index(3, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(3, AncestorModel::MOTHER_ID).data().isNull());
    }

    void testGrandParentGetsNoAncestors() {
        AncestorModel model{5};

        QCOMPARE(model.rowCount(), 1);

        QCOMPARE(model.index(0, AncestorModel::CHILD_ID).data(), 5);
        QVERIFY(model.index(0, AncestorModel::FATHER_ID).data().isNull());
        QVERIFY(model.index(0, AncestorModel::MOTHER_ID).data().isNull());
    }
};

QTEST_MAIN(TestAncestorModel)

#include "ancestor_model_test.moc"
