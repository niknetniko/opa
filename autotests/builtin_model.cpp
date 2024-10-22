// ReSharper disable CppMemberFunctionMayBeStatic
#include "utils/builtin_model.h"

#include <QTest>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QSqlError>
#include <QSqlQuery>

#include "utils/builtin_model.h"

#include <QSqlTableModel>


#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

using namespace Qt::Literals::StringLiterals;

class TestBuiltinModel : public QObject {
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
        QVERIFY(
            query.exec(
                u"CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT, builtin BOOLEAN NON NULL DEFAULT FALSE)"_s
            ));
        QVERIFY(
            query.exec(
                u"INSERT INTO test_table (name, builtin) VALUES ('First', true), ('Second', true), ('Third', false)"_s))
        ;
    }

    void cleanup() {
        QSqlQuery query;
        QVERIFY(query.exec(u"DROP TABLE test_table"_s));
    }

    void testWithModelTesterWithoutForeignKeys() {
        QSqlTableModel model;
        model.setTable(u"test_table"_s);
        QVERIFY(model.select());
        BuiltinModel builtinModel;
        builtinModel.setSourceModel(&model);
        builtinModel.setColumns(2, 1);
        auto tester = QAbstractItemModelTester(&builtinModel, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testColumnIsHiddenAndShownAsIcon() {
        QSqlTableModel model;
        model.setTable(u"test_table"_s);
        QVERIFY(model.select());

        QCOMPARE(model.columnCount(), 3);
        QCOMPARE(model.index(0, 0).data(), 1);
        QCOMPARE(model.index(0, 1).data(), u"First"_s);
        QCOMPARE(model.index(0, 2).data(), true);

        QCOMPARE(model.index(0, 0).data(Qt::DecorationRole), QVariant());
        QCOMPARE(model.index(0, 1).data(Qt::DecorationRole), QVariant());
        QCOMPARE(model.index(0, 2).data(Qt::DecorationRole), QVariant());

        BuiltinModel builtinModel;
        builtinModel.setSourceModel(&model);
        builtinModel.setColumns(2, 1);

        QCOMPARE(builtinModel.columnCount(), 2);
        QCOMPARE(builtinModel.index(0, 0).data(), 1);
        QCOMPARE(builtinModel.index(0, 1).data(), u"First"_s);

        QCOMPARE(builtinModel.index(0, 0).data(Qt::DecorationRole), QVariant());
        QVERIFY(builtinModel.index(0, 1).data(Qt::DecorationRole).canConvert<QIcon>());
        QCOMPARE(builtinModel.index(0, 1).data(Qt::DecorationRole).value<QIcon>().name(), u"lock"_s);
    }

    void testBuiltinItemsAreNotEditable() {
        QSqlTableModel model;
        model.setTable(u"test_table"_s);
        QVERIFY(model.select());

        QVERIFY(model.index(0, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(0, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(0, 2).flags().testFlag(Qt::ItemIsEditable));

        QVERIFY(model.index(2, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(2, 1).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(model.index(2, 2).flags().testFlag(Qt::ItemIsEditable));

        BuiltinModel builtinModel;
        builtinModel.setSourceModel(&model);
        builtinModel.setColumns(2, 1);

        QVERIFY(!builtinModel.index(0, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(!builtinModel.index(0, 1).flags().testFlag(Qt::ItemIsEditable));

        QVERIFY(builtinModel.index(2, 0).flags().testFlag(Qt::ItemIsEditable));
        QVERIFY(builtinModel.index(2, 1).flags().testFlag(Qt::ItemIsEditable));
    }
};

QTEST_MAIN(TestBuiltinModel)

#include "builtin_model.moc"

#pragma clang diagnostic pop
