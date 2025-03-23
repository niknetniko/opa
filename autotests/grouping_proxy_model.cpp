/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "utils/grouping_proxy_model.h"

#include "database/database.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestGroupingProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void virtualRowsModelTest() const {
        auto rootModel = QStandardItemModel(6, 2); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0-column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, value);
        }

        auto proxy = VirtualParentsModel();
        proxy.setSourceModel(&rootModel);
        proxy.setGroupedByColumn(0);

        QCOMPARE(proxy.rowCount(), rootModel.rowCount() + 3);
        QCOMPARE(proxy.columnCount(), rootModel.columnCount() + 1);

        // Check normal data.
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            for (int col = 0; col < rootModel.columnCount(); ++col) {
                QCOMPARE(proxy.index(row, col).data(), rootModel.index(row, col).data());
            }
            // The last column should be empty for these rows.
            QCOMPARE(proxy.index(row, proxy.columnCount() - 1).data(), row);
        }

        // Check virtual rows.
        for (int row = rootModel.rowCount(); row < proxy.rowCount(); ++row) {
            for (int col = 0; col < rootModel.columnCount(); ++col) {
                QCOMPARE(proxy.index(row, col).data(), QVariant());
            }
            QCOMPARE(proxy.index(row, proxy.columnCount() - 1).data(), u"group %1"_s.arg(row - rootModel.rowCount()));
        }
    }

    void modelTest() const {
        auto rootModel = QStandardItemModel(6, 2); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0-column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, value);
        }

        auto* proxy = createGroupingProxyModel(&rootModel, 0);

        QCOMPARE(proxy->rowCount(), 3);
        QCOMPARE(proxy->columnCount(), 3);

        QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_MAIN(TestGroupingProxyModel)

#include "grouping_proxy_model.moc"
