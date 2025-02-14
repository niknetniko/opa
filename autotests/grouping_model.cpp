/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "database/database.h"
#include "utils/grouped_items_proxy_model.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestGroupedItemsProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void modelTest() const {
        auto rootModel = QStandardItemModel(6, 2); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0, column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, value);
        }

        auto proxy = GroupedItemsProxyModel();
        proxy.setSourceModel(&rootModel);
        proxy.setGroups({0});

        // TODO: fix the bug and try again.
        // auto tester = QAbstractItemModelTester(&proxy,
        // QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy.columnCount(), rootModel.columnCount() + 1);
        proxy.setGroupColumnVisible(false);
        QCOMPARE(proxy.columnCount(), rootModel.columnCount());
    }
};

QTEST_MAIN(TestGroupedItemsProxyModel)

#include "grouping_model.moc"
