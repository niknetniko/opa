/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "utils/grouping_proxy_model.h"

#include "database/database.h"
#include "utils/model_utils.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {
    // ReSharper disable once CppDFAUnreachableFunctionCall
    QStandardItemModel* createRootModel() {
        auto* rootModel = new QStandardItemModel(6, 2); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel->rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0-column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            rootModel->setItem(row, 0, group);
            rootModel->setItem(row, 1, value);
        }

        return rootModel;
    }
}

class TestGroupingProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void virtualRowsManualTest() const {
        auto* rootModel = createRootModel();

        VirtualParentsModel proxy;
        proxy.setSourceModel(rootModel);
        proxy.setGroupedByColumn(0);
        proxy.setIdColumn(1);

        QCOMPARE(proxy.rowCount(), rootModel->rowCount() + 3);
        QCOMPARE(proxy.columnCount(), rootModel->columnCount() + 1);

        // Check normal data.
        for (int row = 0; row < rootModel->rowCount(); ++row) {
            for (int col = 0; col < rootModel->columnCount(); ++col) {
                QCOMPARE(proxy.index(row, col).data(), rootModel->index(row, col).data());
            }
            // The last column should be empty for these rows.
            QCOMPARE(proxy.index(row, rootModel->columnCount()).data(), u"row %1-column 1"_s.arg(row));
        }

        // Check virtual rows.
        for (int row = rootModel->rowCount(); row < proxy.rowCount(); ++row) {
            for (int col = 0; col < rootModel->columnCount(); ++col) {
                QCOMPARE(proxy.index(row, col).data(), QVariant::fromValue(nullptr));
            }
            QCOMPARE(proxy.index(row, rootModel->columnCount()).data(), u"group %1"_s.arg(row - rootModel->rowCount()));
        }
    }

    void virtualRowsModelTest() const {
        auto* rootModel = createRootModel();
        VirtualParentsModel proxy;
        proxy.setSourceModel(rootModel);
        proxy.setGroupedByColumn(0);
        proxy.setIdColumn(1);

        QAbstractItemModelTester(&proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void groupingModelTest() const {
        auto* rootModel = createRootModel();
        auto* proxy = createGroupingProxyModel(rootModel, 0, 1);

        QCOMPARE(proxy->rowCount(), 3);
        QCOMPARE(proxy->columnCount(), 3);

        QAbstractItemModelTester(proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }
};

QTEST_MAIN(TestGroupingProxyModel)

#include "grouping_proxy_model.moc"
