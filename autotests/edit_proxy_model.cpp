/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
#include "utils/edit_proxy_model.h"

#include "utils/builtin_model.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStandardItemModel>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestEditProxyModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() {
        QSqlDatabase db = QSqlDatabase::addDatabase(u"QSQLITE"_s);
        db.setDatabaseName(u":memory:"_s);
        QVERIFY(db.open());
    }

    void cleanupTestCase() {
        QSqlDatabase::database().close();
    }

    void init() {
        QSqlQuery query;
        QVERIFY(query.exec(
            u"CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT, builtin BOOLEAN NON NULL DEFAULT FALSE)"_s
        ));
        QVERIFY(query.exec(
            u"INSERT INTO test_table (name, builtin) VALUES ('First', true), ('Second', true), ('Third', false)"_s
        ));
    }

    void cleanup() {
        QSqlQuery query;
        QVERIFY(query.exec(u"DROP TABLE test_table"_s));
    }

    void testWithModelTester() {
        QSqlTableModel model;
        model.setTable(u"test_table"_s);
        QVERIFY(model.select());
        EditProxyModel builtinModel;
        builtinModel.setSourceModel(&model);
        builtinModel.addReadOnlyColumns({1, 2});
        auto tester = QAbstractItemModelTester(&builtinModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testIndicatedColumnsAreReadOnly() {
        QSqlTableModel model;
        model.setTable(u"test_table"_s);
        QVERIFY(model.select());

        QVERIFY(model.index(0, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(0, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(0, 2).flags().testFlag(Qt::ItemIsEditable));

        QVERIFY(model.index(2, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(2, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(2, 2).flags().testFlag(Qt::ItemIsEditable));

        EditProxyModel builtinModel;
        builtinModel.setSourceModel(&model);
        builtinModel.addReadOnlyColumns({0, 2});

        QVERIFY(!builtinModel.index(0, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(builtinModel.index(0, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(!builtinModel.index(0, 2).flags().testFlag(Qt::ItemIsEditable));

        QVERIFY(!builtinModel.index(2, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(builtinModel.index(2, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(!builtinModel.index(2, 2).flags().testFlag(Qt::ItemIsEditable));
    }
};

QTEST_MAIN(TestEditProxyModel)

#include "edit_proxy_model.moc"
