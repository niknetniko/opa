/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "domain/family/family_list_model.h"

#include "./test_utils.h"
#include "database/database.h"

#include <QAbstractItemModelTester>
#include <QSqlDatabase>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestFamilyListModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, true);
    }

    void cleanup() {
        QSqlDatabase::database().close();
    }

    void testPassesModelTester() {
        FamilyListModel model;
        new QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testTopLevelRowCountEqualsFamilyCount() {
        FamilyListModel model;
        // Seed data has 4 families.
        QCOMPARE(model.rowCount(), 4);
    }

    void testTopLevelRowCountIsZeroWithNoFamilies() {
        QSqlDatabase::database().close();
        openDatabase(u":memory:"_s, false);

        FamilyListModel model;
        QCOMPARE(model.rowCount(), 0);
    }

    void testTopLevelParentIsInvalid() {
        FamilyListModel model;
        for (int i = 0; i < model.rowCount(); ++i) {
            QVERIFY(!model.index(i, 0).parent().isValid());
        }
    }

    void testFamilyIdColumnHasCorrectId() {
        FamilyListModel model;
        // All four seed family IDs should appear as top-level FAMILY_ID values.
        QSet<IntegerPrimaryKey> ids;
        for (int i = 0; i < model.rowCount(); ++i) {
            auto id = model.index(i, FamilyListModel::FAMILY_ID).data().toLongLong();
            QVERIFY(id > 0);
            ids.insert(id);
        }
        QCOMPARE(ids.size(), 4);
        QVERIFY(ids.contains(1));
        QVERIFY(ids.contains(2));
        QVERIFY(ids.contains(3));
        QVERIFY(ids.contains(4));
    }

    void testFamilyDisplayNameIsShownAtTopLevel() {
        FamilyListModel model;
        // Every top-level row must have a non-empty display name.
        for (int i = 0; i < model.rowCount(); ++i) {
            auto name = model.index(i, FamilyListModel::DISPLAY_NAME).data().toString();
            QVERIFY(!name.isEmpty());
        }
    }

    void testChildRowsExistUnderFamilies() {
        FamilyListModel model;
        // Every family in the seed has at least one child row.
        for (int i = 0; i < model.rowCount(); ++i) {
            auto parent = model.index(i, 0);
            QVERIFY(model.rowCount(parent) > 0);
        }
    }

    void testChildRowCountsMatchSeedData() {
        FamilyListModel model;

        // Build family_id -> top-level row index map.
        QHash<IntegerPrimaryKey, int> familyRow;
        for (int i = 0; i < model.rowCount(); ++i) {
            auto id = model.index(i, FamilyListModel::FAMILY_ID).data().toLongLong();
            familyRow[id] = i;
        }

        // Family 1: marriage (Michael, Emily) + births (John, Jane) = 4 child rows
        QCOMPARE(model.rowCount(model.index(familyRow[1], 0)), 4);
        // Family 2: marriage (William, Elizabeth) + birth (Michael) = 3 child rows
        QCOMPARE(model.rowCount(model.index(familyRow[2], 0)), 3);
        // Family 3: marriage (George, Mary) + birth (Emily) = 3 child rows
        QCOMPARE(model.rowCount(model.index(familyRow[3], 0)), 3);
        // Family 4: births (Elizabeth, George) only = 2 child rows
        QCOMPARE(model.rowCount(model.index(familyRow[4], 0)), 2);
    }

    void testChildRowHasDisplayName() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        for (int i = 0; i < model.rowCount(parent); ++i) {
            auto name = model.index(i, FamilyListModel::DISPLAY_NAME, parent).data().toString();
            QVERIFY(!name.isEmpty());
        }
    }

    void testChildRowHasEventType() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        for (int i = 0; i < model.rowCount(parent); ++i) {
            auto type = model.index(i, FamilyListModel::TYPE, parent).data().toString();
            QVERIFY(!type.isEmpty());
        }
    }

    void testChildRowHasPersonId() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        for (int i = 0; i < model.rowCount(parent); ++i) {
            auto personId = model.index(i, FamilyListModel::PERSON_ID, parent).data().toLongLong();
            QVERIFY(personId > 0);
        }
    }

    void testChildRowHasEventId() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        for (int i = 0; i < model.rowCount(parent); ++i) {
            auto eventId = model.index(i, FamilyListModel::EVENT_ID, parent).data().toLongLong();
            QVERIFY(eventId > 0);
        }
    }

    void testChildFamilyIdColumnIsEmpty() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        for (int i = 0; i < model.rowCount(parent); ++i) {
            auto familyId = model.index(i, FamilyListModel::FAMILY_ID, parent).data();
            QVERIFY(!familyId.isValid() || familyId.isNull());
        }
    }

    void testParentOfChildIsTopLevelRow() {
        FamilyListModel model;
        auto topLevel = model.index(0, 0);
        auto child = model.index(0, 0, topLevel);
        QVERIFY(child.isValid());
        auto childParent = model.parent(child);
        QVERIFY(childParent.isValid());
        QCOMPARE(childParent.row(), 0);
        QCOMPARE(childParent.column(), 0);
    }

    void testColumnCount() {
        FamilyListModel model;
        QCOMPARE(model.columnCount(), 7);
    }

    void testHeaderDataIsSet() {
        FamilyListModel model;
        for (int col = 0; col < model.columnCount(); ++col) {
            auto header = model.headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
            QVERIFY(!header.isEmpty());
        }
    }

    void testTopLevelFlagsAreSelectableAndEnabled() {
        FamilyListModel model;
        auto flags = model.flags(model.index(0, 0));
        QVERIFY(flags.testFlag(Qt::ItemIsSelectable));
        QVERIFY(flags.testFlag(Qt::ItemIsEnabled));
    }

    void testChildFlagsAreSelectableAndEnabled() {
        FamilyListModel model;
        auto parent = model.index(0, 0);
        auto flags = model.flags(model.index(0, 0, parent));
        QVERIFY(flags.testFlag(Qt::ItemIsSelectable));
        QVERIFY(flags.testFlag(Qt::ItemIsEnabled));
    }
};

QTEST_MAIN(TestFamilyListModel)

#include "family_list_model_test.moc"
