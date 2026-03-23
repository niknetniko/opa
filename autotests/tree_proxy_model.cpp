/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// ReSharper disable CppMemberFunctionMayBeConst
#include "utils/tree_proxy_model.h"

#include "utils/model_utils.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {

/**
 * Helper to build a flat QStandardItemModel with columns: ID, Name, ParentID.
 * Each entry is {id, name, parentId}. Use an empty string for no parent.
 */
QStandardItemModel* buildSourceModel(QObject* parent, const QList<std::tuple<QString, QString, QString>>& rows) {
    auto* model = new QStandardItemModel(static_cast<int>(rows.size()), 3, parent);
    for (int i = 0; i < rows.size(); ++i) {
        const auto& [id, name, parentId] = rows[i];
        model->setItem(i, 0, new QStandardItem(id));
        model->setItem(i, 1, new QStandardItem(name));
        if (!parentId.isEmpty()) {
            model->setItem(i, 2, new QStandardItem(parentId));
        }
    }
    return model;
}

/**
 * Build a TreeProxyModel with columns 0 (ID) and 2 (ParentID).
 */
TreeProxyModel* buildProxy(QAbstractItemModel* sourceModel, QObject* parent) {
    auto* proxy = new TreeProxyModel(parent);
    proxy->setSourceModel(sourceModel);
    proxy->setIdColumn(0);
    proxy->setParentIdColumn(2);
    return proxy;
}

}

class TestTreeProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testWithModelTester() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"ID 0"_s, u"Naam 0"_s, u""_s},
                {u"ID 1"_s, u"Naam 1"_s, u""_s},
                {u"ID 2"_s, u"Naam 2"_s, u""_s},
                {u"ID 3"_s, u"Naam 3"_s, u""_s},
                {u"ID 4"_s, u"Naam 4"_s, u"ID 0"_s},
                {u"ID 5"_s, u"Naam 5"_s, u"ID 4"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testEmptySourceModel() {
        auto* rootModel = buildSourceModel(this, {});
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 0);
        QCOMPARE(proxy->columnCount(), 3);
        QVERIFY(!proxy->hasChildren({}));
    }

    void testAllRootNodes() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
                {u"B"_s, u"Beta"_s, u""_s},
                {u"C"_s, u"Gamma"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 3);
        for (int i = 0; i < 3; ++i) {
            auto idx = proxy->index(i, 0, {});
            QVERIFY(idx.isValid());
            QCOMPARE(proxy->rowCount(idx), 0);
            QVERIFY(!proxy->hasChildren(idx));
            QVERIFY(!proxy->parent(idx).isValid());
        }
    }

    void testSingleChildRelationship() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 1);

        auto parentIdx = proxy->index(0, 0, {});
        QCOMPARE(parentIdx.data().toString(), u"P"_s);
        QCOMPARE(proxy->rowCount(parentIdx), 1);
        QVERIFY(proxy->hasChildren(parentIdx));

        auto childIdx = proxy->index(0, 0, parentIdx);
        QCOMPARE(childIdx.data().toString(), u"C"_s);
        QCOMPARE(proxy->rowCount(childIdx), 0);

        // parent() navigates back
        auto childParent = proxy->parent(childIdx);
        QVERIFY(childParent.isValid());
        QCOMPARE(childParent.data().toString(), u"P"_s);
    }

    void testMultipleChildrenUnderOneParent() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C1"_s, u"Child 1"_s, u"P"_s},
                {u"C2"_s, u"Child 2"_s, u"P"_s},
                {u"C3"_s, u"Child 3"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 1);
        auto parentIdx = proxy->index(0, 0, {});
        QCOMPARE(proxy->rowCount(parentIdx), 3);

        // Verify child order matches source order
        QCOMPARE(proxy->index(0, 0, parentIdx).data().toString(), u"C1"_s);
        QCOMPARE(proxy->index(1, 0, parentIdx).data().toString(), u"C2"_s);
        QCOMPARE(proxy->index(2, 0, parentIdx).data().toString(), u"C3"_s);
    }

    void testDeepNesting() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"L0"_s, u"Level 0"_s, u""_s},
                {u"L1"_s, u"Level 1"_s, u"L0"_s},
                {u"L2"_s, u"Level 2"_s, u"L1"_s},
                {u"L3"_s, u"Level 3"_s, u"L2"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 1);

        auto l0 = proxy->index(0, 0, {});
        QCOMPARE(l0.data().toString(), u"L0"_s);
        QCOMPARE(proxy->rowCount(l0), 1);

        auto l1 = proxy->index(0, 0, l0);
        QCOMPARE(l1.data().toString(), u"L1"_s);
        QCOMPARE(proxy->rowCount(l1), 1);

        auto l2 = proxy->index(0, 0, l1);
        QCOMPARE(l2.data().toString(), u"L2"_s);
        QCOMPARE(proxy->rowCount(l2), 1);

        auto l3 = proxy->index(0, 0, l2);
        QCOMPARE(l3.data().toString(), u"L3"_s);
        QCOMPARE(proxy->rowCount(l3), 0);

        // Walk back up via parent()
        QCOMPARE(proxy->parent(l3).data().toString(), u"L2"_s);
        QCOMPARE(proxy->parent(l2).data().toString(), u"L1"_s);
        QCOMPARE(proxy->parent(l1).data().toString(), u"L0"_s);
        QVERIFY(!proxy->parent(l0).isValid());
    }

    void testMultipleRootsWithChildren() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"R1"_s, u"Root 1"_s, u""_s},
                {u"R2"_s, u"Root 2"_s, u""_s},
                {u"C1"_s, u"Child of R1"_s, u"R1"_s},
                {u"C2"_s, u"Child of R2"_s, u"R2"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 2);

        auto r1 = proxy->index(0, 0, {});
        auto r2 = proxy->index(1, 0, {});
        QCOMPARE(r1.data().toString(), u"R1"_s);
        QCOMPARE(r2.data().toString(), u"R2"_s);

        QCOMPARE(proxy->rowCount(r1), 1);
        QCOMPARE(proxy->rowCount(r2), 1);
        QCOMPARE(proxy->index(0, 0, r1).data().toString(), u"C1"_s);
        QCOMPARE(proxy->index(0, 0, r2).data().toString(), u"C2"_s);
    }

    void testMultipleColumnsAccessible() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->columnCount(), 3);

        auto parentIdx = proxy->index(0, 0, {});

        // Access column 1 (name) for child
        auto childNameIdx = proxy->index(0, 1, parentIdx);
        QCOMPARE(childNameIdx.data().toString(), u"Child"_s);

        // Access column 1 (name) for root
        auto parentNameIdx = proxy->index(0, 1, {});
        QCOMPARE(parentNameIdx.data().toString(), u"Parent"_s);
    }

    void testOrphanedNodesBecomeRoots() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
                {u"B"_s, u"Beta"_s, u"NONEXISTENT"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // B's parent doesn't exist, so it becomes a root
        QCOMPARE(proxy->rowCount(), 2);
        QCOMPARE(proxy->index(0, 0, {}).data().toString(), u"A"_s);
        QCOMPARE(proxy->index(1, 0, {}).data().toString(), u"B"_s);
    }

    void testMapToSource() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Map root item
        auto proxyRoot = proxy->index(0, 0, {});
        auto sourceRoot = proxy->mapToSource(proxyRoot);
        QCOMPARE(sourceRoot.row(), 0);
        QCOMPARE(sourceRoot.column(), 0);

        // Map child item
        auto proxyChild = proxy->index(0, 0, proxyRoot);
        auto sourceChild = proxy->mapToSource(proxyChild);
        QCOMPARE(sourceChild.row(), 1);
        QCOMPARE(sourceChild.column(), 0);

        // Map child with different column
        auto proxyChildCol1 = proxy->index(0, 1, proxyRoot);
        auto sourceChildCol1 = proxy->mapToSource(proxyChildCol1);
        QCOMPARE(sourceChildCol1.row(), 1);
        QCOMPARE(sourceChildCol1.column(), 1);
    }

    void testMapFromSource() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Map source root → proxy
        auto sourceRoot = rootModel->index(0, 0);
        auto proxyRoot = proxy->mapFromSource(sourceRoot);
        QVERIFY(proxyRoot.isValid());
        QCOMPARE(proxyRoot.data().toString(), u"P"_s);
        QVERIFY(!proxy->parent(proxyRoot).isValid()); // is root

        // Map source child → proxy
        auto sourceChild = rootModel->index(1, 0);
        auto proxyChild = proxy->mapFromSource(sourceChild);
        QVERIFY(proxyChild.isValid());
        QCOMPARE(proxyChild.data().toString(), u"C"_s);
        QCOMPARE(proxy->parent(proxyChild).data().toString(), u"P"_s);
    }

    void testMapFromSourceWithColumn() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Map source index with column 1
        auto sourceIdx = rootModel->index(1, 1);
        auto proxyIdx = proxy->mapFromSource(sourceIdx);
        QVERIFY(proxyIdx.isValid());
        QCOMPARE(proxyIdx.column(), 1);
        QCOMPARE(proxyIdx.data().toString(), u"Child"_s);
    }

    void testMapInvalidIndices() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        QVERIFY(!proxy->mapFromSource({}).isValid());
        QVERIFY(!proxy->mapToSource({}).isValid());
    }

    void testMapRoundTrip() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C1"_s, u"Child 1"_s, u"P"_s},
                {u"C2"_s, u"Child 2"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // For every source row, mapFromSource → mapToSource should return the original
        for (int row = 0; row < rootModel->rowCount(); ++row) {
            for (int col = 0; col < rootModel->columnCount(); ++col) {
                auto sourceIdx = rootModel->index(row, col);
                auto proxyIdx = proxy->mapFromSource(sourceIdx);
                auto backToSource = proxy->mapToSource(proxyIdx);
                QCOMPARE(backToSource.row(), sourceIdx.row());
                QCOMPARE(backToSource.column(), sourceIdx.column());
            }
        }
    }

    void testEditing() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"ID 0"_s, u"Naam 0"_s, u""_s},
                {u"ID 1"_s, u"Naam 1"_s, u""_s},
                {u"ID 2"_s, u"Naam 2"_s, u""_s},
                {u"ID 3"_s, u"Naam 3"_s, u""_s},
                {u"ID 4"_s, u"Naam 4"_s, u"ID 0"_s},
                {u"ID 5"_s, u"Naam 5"_s, u"ID 4"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        auto firstParent = proxy->index(0, 0, {});
        auto secondParent = proxy->index(0, 0, firstParent);
        auto index = proxy->index(0, 1, secondParent);
        proxy->setData(index, u"New name"_s);

        // Now check that the data in the original model changed.
        auto originalIndex = rootModel->index(5, 1);
        QCOMPARE(originalIndex.data(), u"New name"_s);
    }

    void testEditingRootNode() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
                {u"B"_s, u"Beta"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        auto idx = proxy->index(1, 1, {});
        proxy->setData(idx, u"Modified"_s);

        QCOMPARE(rootModel->index(1, 1).data().toString(), u"Modified"_s);
    }

    void testSourceModelRowRemoval() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"ID 0"_s, u"Naam 0"_s, u""_s},
                {u"ID 1"_s, u"Naam 1"_s, u""_s},
                {u"ID 2"_s, u"Naam 2"_s, u""_s},
                {u"ID 3"_s, u"Naam 3"_s, u"ID 0"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Verify initial state: 3 top-level rows, 1 child under row 0
        QCOMPARE(proxy->rowCount(), 3);
        auto parent0 = proxy->index(0, 0, {});
        QCOMPARE(proxy->rowCount(parent0), 1);

        // Remove a row from the source model (simulates what happens during deletion)
        rootModel->removeRow(1);

        // Proxy should still be valid: 2 top-level rows, 1 child
        QCOMPARE(proxy->rowCount(), 2);
        auto newParent0 = proxy->index(0, 0, {});
        QCOMPARE(proxy->rowCount(newParent0), 1);

        // Verify the child is still accessible and correct
        auto child = proxy->index(0, 0, newParent0);
        QVERIFY(child.isValid());
        QCOMPARE(child.data().toString(), u"ID 3"_s);
    }

    void testSourceModelRowInsertion() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 1);
        QCOMPARE(proxy->rowCount(proxy->index(0, 0, {})), 0);

        // Insert a child row
        rootModel->insertRow(1);
        rootModel->setItem(1, 0, new QStandardItem(u"C"_s));
        rootModel->setItem(1, 1, new QStandardItem(u"Child"_s));
        rootModel->setItem(1, 2, new QStandardItem(u"P"_s));

        auto parentIdx = proxy->index(0, 0, {});
        QCOMPARE(proxy->rowCount(parentIdx), 1);
        QCOMPARE(proxy->index(0, 0, parentIdx).data().toString(), u"C"_s);
    }

    void testSourceModelFullReset() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"ID 0"_s, u"Naam 0"_s, u""_s},
                {u"ID 1"_s, u"Naam 1"_s, u""_s},
                {u"ID 2"_s, u"Naam 2"_s, u"ID 0"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 2); // ID 0 and ID 1 at root
        auto parent0 = proxy->index(0, 0, {});
        QCOMPARE(proxy->rowCount(parent0), 1);

        // Replace all data via clear() which emits modelReset
        rootModel->clear();
        rootModel->setRowCount(2);
        rootModel->setColumnCount(3);
        rootModel->setItem(0, 0, new QStandardItem(u"ID 10"_s));
        rootModel->setItem(0, 1, new QStandardItem(u"Alpha"_s));
        rootModel->setItem(1, 0, new QStandardItem(u"ID 11"_s));
        rootModel->setItem(1, 1, new QStandardItem(u"Beta"_s));
        rootModel->setItem(1, 2, new QStandardItem(u"ID 10"_s)); // child of ID 10

        // Proxy should reflect the new data
        QCOMPARE(proxy->rowCount(), 1); // Only ID 10 at root
        auto root = proxy->index(0, 0, {});
        QCOMPARE(root.data().toString(), u"ID 10"_s);
        QCOMPARE(proxy->rowCount(root), 1); // ID 11 is child
        auto child = proxy->index(0, 0, root);
        QCOMPARE(child.data().toString(), u"ID 11"_s);
    }

    void testRemoveParentOrphansChild() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
                {u"O"_s, u"Other"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 2); // P and O
        QCOMPARE(proxy->rowCount(proxy->index(0, 0, {})), 1); // C under P

        // Remove the parent row (row 0)
        rootModel->removeRow(0);

        // Child should now be orphaned → treated as root
        // Remaining source: C (parentId=P, but P is gone), O
        QCOMPARE(proxy->rowCount(), 2);
    }

    void testDataChangedOnNonStructuralColumnForwards() {
        // When structural columns are adjacent (e.g. 0 and 1), a change to
        // column 2 is outside the structural range and gets forwarded.
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        TreeProxyModel proxy;
        proxy.setSourceModel(rootModel);
        // Use columns 0 (ID) and 1 (parentID) so column 2 is non-structural
        proxy.setIdColumn(0);
        proxy.setParentIdColumn(1);

        QSignalSpy dataChangedSpy(&proxy, &QAbstractItemModel::dataChanged);

        rootModel->setData(rootModel->index(0, 2), u"Updated"_s);

        QVERIFY(dataChangedSpy.count() >= 1);
        auto proxyIdx = proxy.index(0, 2, {});
        QCOMPARE(proxyIdx.data().toString(), u"Updated"_s);
    }

    void testDataChangedOnColumnBetweenStructuralColumnsForwards() {
        // Column 1 is between idColumn=0 and parentIdColumn=2 but is not
        // structural, so changes should be forwarded (not trigger a rebuild).
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        QSignalSpy dataChangedSpy(proxy, &QAbstractItemModel::dataChanged);
        QSignalSpy resetSpy(proxy, &QAbstractItemModel::modelReset);

        rootModel->setData(rootModel->index(0, 1), u"Updated"_s);

        QVERIFY(dataChangedSpy.count() >= 1);
        QCOMPARE(resetSpy.count(), 0);
        auto proxyIdx = proxy->index(0, 1, {});
        QCOMPARE(proxyIdx.data().toString(), u"Updated"_s);
    }

    void testDataChangedOnStructuralColumnTriggersReset() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        QSignalSpy resetSpy(proxy, &QAbstractItemModel::modelReset);

        // Change the parent ID column (column 2) — structural
        rootModel->setItem(1, 2, new QStandardItem()); // Remove parent ref

        QVERIFY(resetSpy.count() >= 1);
        // Child should now be a root
        QCOMPARE(proxy->rowCount(), 2);
    }

    void testSibling() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"R1"_s, u"Root 1"_s, u""_s},
                {u"R2"_s, u"Root 2"_s, u""_s},
                {u"R3"_s, u"Root 3"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        auto r1 = proxy->index(0, 0, {});
        auto r2 = proxy->sibling(1, 0, r1);
        QCOMPARE(r2.data().toString(), u"R2"_s);

        // Sibling with different column
        auto r1col1 = proxy->sibling(0, 1, r1);
        QCOMPARE(r1col1.data().toString(), u"Root 1"_s);
        QCOMPARE(r1col1.column(), 1);

        // Same index returns itself
        auto same = proxy->sibling(0, 0, r1);
        QCOMPARE(same, r1);
    }

    void testSiblingAmongChildren() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C1"_s, u"Child 1"_s, u"P"_s},
                {u"C2"_s, u"Child 2"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        auto parent = proxy->index(0, 0, {});
        auto c1 = proxy->index(0, 0, parent);
        auto c2 = proxy->sibling(1, 0, c1);
        QCOMPARE(c2.data().toString(), u"C2"_s);
        QCOMPARE(proxy->parent(c2).data().toString(), u"P"_s);
    }

    void testHasChildren() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
                {u"L"_s, u"Leaf"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Root (invalid index) has children
        QVERIFY(proxy->hasChildren({}));

        auto parentIdx = proxy->index(0, 0, {});
        QVERIFY(proxy->hasChildren(parentIdx));

        auto child = proxy->index(0, 0, parentIdx);
        QVERIFY(!proxy->hasChildren(child));

        auto leafIdx = proxy->index(1, 0, {});
        QVERIFY(!proxy->hasChildren(leafIdx));
    }

    void testOutOfBoundsIndex() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        // Negative row
        QVERIFY(!proxy->index(-1, 0, {}).isValid());
        // Row beyond count
        QVERIFY(!proxy->index(1, 0, {}).isValid());
        // Negative column
        QVERIFY(!proxy->index(0, -1, {}).isValid());
        // Column beyond count
        QVERIFY(!proxy->index(0, 3, {}).isValid());
    }

    void testRowCountForNonZeroColumnParent() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"P"_s, u"Parent"_s, u""_s},
                {u"C"_s, u"Child"_s, u"P"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        // Qt convention: only column 0 can have children
        auto parentCol1 = proxy->index(0, 1, {});
        QCOMPARE(proxy->rowCount(parentCol1), 0);
    }

    void testNoSourceModel() {
        TreeProxyModel proxy;
        QCOMPARE(proxy.rowCount(), 0);
        QCOMPARE(proxy.columnCount(), 0);
        QVERIFY(!proxy.hasChildren({}));
    }

    void testReplaceSourceModel() {
        auto* model1 = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
                {u"B"_s, u"Beta"_s, u"A"_s},
            }
        );
        auto* model2 = buildSourceModel(
            this,
            {
                {u"X"_s, u"X-ray"_s, u""_s},
                {u"Y"_s, u"Yankee"_s, u""_s},
                {u"Z"_s, u"Zulu"_s, u"X"_s},
            }
        );

        auto* proxy = buildProxy(model1, this);

        QCOMPARE(proxy->rowCount(), 1);
        QCOMPARE(proxy->index(0, 0, {}).data().toString(), u"A"_s);

        // Switch source model
        proxy->setSourceModel(model2);
        proxy->setIdColumn(0);
        proxy->setParentIdColumn(2);

        QCOMPARE(proxy->rowCount(), 2);
        QCOMPARE(proxy->index(0, 0, {}).data().toString(), u"X"_s);
        QCOMPARE(proxy->index(1, 0, {}).data().toString(), u"Y"_s);
        QCOMPARE(proxy->rowCount(proxy->index(0, 0, {})), 1);
    }

    void testDuplicateIds() {
        // Two nodes with the same ID — second one wins in the hash map.
        // Children referencing that ID go to whichever node is in the map.
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"DUP"_s, u"First"_s, u""_s},
                {u"DUP"_s, u"Second"_s, u""_s},
                {u"C"_s, u"Child"_s, u"DUP"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);

        // Should not crash. Tree structure may vary, but must be consistent.
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        // Total nodes across all levels should equal source row count
        int totalNodes = 0;
        std::function<void(const QModelIndex&)> countNodes = [&](const QModelIndex& parent) {
            int rows = proxy->rowCount(parent);
            totalNodes += rows;
            for (int i = 0; i < rows; ++i) {
                countNodes(proxy->index(i, 0, parent));
            }
        };
        countNodes({});
        QCOMPARE(totalNodes, 3);
    }

    void testSelfParentBecomesRoot() {
        // A node that references itself as parent is treated as a root.
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u"A"_s},
                {u"B"_s, u"Beta"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy->rowCount(), 2);
        QCOMPARE(proxy->index(0, 0, {}).data().toString(), u"A"_s);
        QCOMPARE(proxy->index(1, 0, {}).data().toString(), u"B"_s);
        // A should have no children (it doesn't parent itself)
        QCOMPARE(proxy->rowCount(proxy->index(0, 0, {})), 0);
    }

    void testFlagsDoNotHaveNeverHasChildren() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"A"_s, u"Alpha"_s, u""_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        auto idx = proxy->index(0, 0, {});
        QVERIFY(!proxy->flags(idx).testFlag(Qt::ItemNeverHasChildren));
    }

    void testModelTesterDeepTree() {
        auto* rootModel = buildSourceModel(
            this,
            {
                {u"L0"_s, u"Level 0"_s, u""_s},
                {u"L1"_s, u"Level 1"_s, u"L0"_s},
                {u"L2"_s, u"Level 2"_s, u"L1"_s},
                {u"L3"_s, u"Level 3"_s, u"L2"_s},
                {u"S0"_s, u"Sibling"_s, u""_s},
                {u"S1"_s, u"S Child"_s, u"S0"_s},
            }
        );
        auto* proxy = buildProxy(rootModel, this);
        auto tester = QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_MAIN(TestTreeProxyModel)

#include "tree_proxy_model.moc"
