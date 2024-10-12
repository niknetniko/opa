//
// Created by niko on 8/10/24.
//

#include <QTest>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>
#include "database/database.h"
#include "utils/grouped_items_proxy_model.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

class TestGroupedItemsProxyModel : public QObject {
Q_OBJECT

private Q_SLOTS:

    void modelTest() {
        auto rootModel = QStandardItemModel(6, 2);
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            QStandardItem *value = new QStandardItem(QStringLiteral("row %0, column %1").arg(row).arg(1));
            QStandardItem *group = new QStandardItem(QStringLiteral("group %1").arg(row / 2));
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, value);
        }

        auto proxy = GroupedItemsProxyModel();
        proxy.setSourceModel(&rootModel);
        proxy.setGroups({0});

        auto tester = QAbstractItemModelTester(&proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);

        QCOMPARE(proxy.columnCount(), rootModel.columnCount() + 1);
        proxy.setGroupColumnVisible(false);
        QCOMPARE(proxy.columnCount(), rootModel.columnCount());
    }
};

QTEST_MAIN(TestGroupedItemsProxyModel)

#include "grouping_model.moc"

#pragma clang diagnostic pop