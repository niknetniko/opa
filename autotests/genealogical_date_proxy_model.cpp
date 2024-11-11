/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// ReSharper disable CppMemberFunctionMayBeConst
#include "dates/genealogical_date_proxy_model.h"

#include "dates/genealogical_date.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestGenealogicalDateProxyModel : public QObject {
    Q_OBJECT

    const GenealogicalDate date =
        GenealogicalDate(GenealogicalDate::BEFORE, GenealogicalDate::EXACT, QDate(2001, 7, 4), true, true, true, u""_s);

private Q_SLOTS:
    void testWithModelTester() {
        QStandardItemModel rootModel(6, 3); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0, column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            auto* dateItem = new QStandardItem(date.toDatabaseRepresentation());
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, dateItem);
            rootModel.setItem(row, 2, value);
        }
        GenealogicalDateProxyModel proxy;
        proxy.setSourceModel(&rootModel);
        proxy.setDateColumn(1);
        auto tester = QAbstractItemModelTester(&proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testDataIsReadInHumanForm() {
        QStandardItemModel rootModel(6, 3); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0, column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            auto* dateItem = new QStandardItem(date.toDatabaseRepresentation());
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 1, dateItem);
            rootModel.setItem(row, 2, value);
        }
        GenealogicalDateProxyModel proxy;
        proxy.setSourceModel(&rootModel);

        QCOMPARE(proxy.index(0, 1).data(), date.toDatabaseRepresentation());
        QCOMPARE(proxy.index(0, 1).data(GenealogicalDateProxyModel::RawDateRole), QVariant());

        proxy.setDateColumn(1);

        QCOMPARE(proxy.index(0, 1).data(), date.toDisplayText());
        QCOMPARE(proxy.index(0, 1).data(GenealogicalDateProxyModel::RawDateRole).value<GenealogicalDate>(), date);
    }

    void testDateIsSavedFromHumanRepresentation() {
        QStandardItemModel rootModel(2, 3); // NOLINT(*-avoid-magic-numbers)
        for (int row = 0; row < rootModel.rowCount(); ++row) {
            auto* value = new QStandardItem(u"row %0, column %1"_s.arg(row).arg(1));
            auto* group = new QStandardItem(u"group %1"_s.arg(row / 2));
            rootModel.setItem(row, 0, group);
            rootModel.setItem(row, 2, value);
        }

        GenealogicalDateProxyModel proxy;
        proxy.setSourceModel(&rootModel);
        proxy.setDateColumn(1);
        proxy.setData(proxy.index(0, 1), date.toDisplayText());
        QCOMPARE(rootModel.index(0, 1).data(), date.toDatabaseRepresentation());

        proxy.setData(proxy.index(1, 1), QVariant::fromValue(date), GenealogicalDateProxyModel::RawDateRole);
        QCOMPARE(rootModel.index(1, 1).data(), date.toDatabaseRepresentation());
    }
};

QTEST_MAIN(TestGenealogicalDateProxyModel)

#include "genealogical_date_proxy_model.moc"
