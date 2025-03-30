/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// ReSharper disable CppMemberFunctionMayBeConst
#include "utils/tree_proxy_model.h"

#include "utils/model_utils.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestTreeProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testWithModelTester() {
        QStandardItemModel rootModel(6, 3); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* id = new QStandardItem(u"ID %0"_s.arg(row));
            auto* name = new QStandardItem(u"Naam %0"_s.arg(row));
            rootModel.setItem(row, 0, id);
            rootModel.setItem(row, 1, name);
            // Ugly I know
            if (row == 4) {
                auto* parentId = new QStandardItem(u"ID 0"_s);
                rootModel.setItem(row, 2, parentId);
            } else if (row == 5) {
                auto* parentId = new QStandardItem(u"ID 4"_s);
                rootModel.setItem(row, 2, parentId);
            }
        }
        TreeProxyModel proxy;
        proxy.setSourceModel(&rootModel);
        proxy.setIdColumn(0);
        proxy.setParentIdColumn(2);
        auto tester = QAbstractItemModelTester(&proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testEditing() {
        QStandardItemModel rootModel(6, 3); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* id = new QStandardItem(u"ID %0"_s.arg(row));
            auto* name = new QStandardItem(u"Naam %0"_s.arg(row));
            rootModel.setItem(row, 0, id);
            rootModel.setItem(row, 1, name);
            // Ugly I know
            if (row == 4) {
                auto* parentId = new QStandardItem(u"ID 0"_s);
                rootModel.setItem(row, 2, parentId);
            }
            if (row == 5) {
                auto* parentId = new QStandardItem(u"ID 4"_s);
                rootModel.setItem(row, 2, parentId);
            }
        }
        TreeProxyModel proxy;
        proxy.setSourceModel(&rootModel);
        proxy.setIdColumn(0);
        proxy.setParentIdColumn(2);

        auto firstParent = proxy.index(0, 0, {});
        auto secondParent = proxy.index(0, 0, firstParent);
        auto index = proxy.index(0, 1, secondParent);
        proxy.setData(index, u"New name"_s);

        // Now check that the data in the original model changed.
        auto originalIndex = rootModel.index(5, 1);
        QCOMPARE(originalIndex.data(), u"New name"_s);
    }
};

QTEST_MAIN(TestTreeProxyModel)

#include "tree_proxy_model.moc"
