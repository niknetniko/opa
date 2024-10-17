//
// Created by niko on 17/10/24.
//

// ReSharper disable CppMemberFunctionMayBeStatic
#include <QTest>
#include <QStandardItemModel>
#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "database/database.h"
#include "utils/custom_sql_relational_model.h"



#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

using namespace Qt::Literals::StringLiterals;

class TestCustomSqlRelationalModel : public QObject {
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
        QVERIFY(query.exec(u"CREATE TABLE names (id INTEGER PRIMARY KEY, name TEXT)"_s));
        QVERIFY(query.exec(u"CREATE TABLE people (id INTEGER PRIMARY KEY, name_id INTEGER REFERENCES names(id), age INTEGER)"_s));
        QVERIFY(query.exec(u"INSERT INTO names (name) VALUES ('Alice'), ('Bob'), ('Chad')"_s));
        QVERIFY(query.exec(u"INSERT INTO people (name_id, age) VALUES (1, 30), (2, 25)"_s));
    }

    void cleanup() {
        QSqlQuery query;
        QVERIFY(query.exec(u"DROP TABLE people"_s));
        QVERIFY(query.exec(u"DROP TABLE names"_s));
    }

    void testWithModelTesterWithoutForeignKeys() {
        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        QVERIFY(model.select());
        auto tester = QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testWithModelTesterWithForeignKeys() {
        QSqlTableModel nameModel;
        nameModel.setTable(u"names"_s);
        QVERIFY(nameModel.select());

        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        model.setRelation(1, &nameModel, 1, 0);
        QVERIFY(model.select());
        auto tester = QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
    }

    void testDataWithoutForeignKeys() {
        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        QVERIFY(model.select());

        QCOMPARE(model.columnCount(), 3);
        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, 0).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 1).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 2).data().toLongLong(), 30);

        QCOMPARE(model.index(1, 0).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 1).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 2).data().toLongLong(), 25);
    }

    void testDataWithForeignKeys() {
        QSqlTableModel nameModel;
        nameModel.setTable(u"names"_s);
        QVERIFY(nameModel.select());

        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        model.setRelation(1, &nameModel, 1, 0);
        QVERIFY(model.select());

        QCOMPARE(model.columnCount(), 4);
        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, 0).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 1).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 2).data().toLongLong(), 30);
        QCOMPARE(model.index(0, 3).data(), u"Alice"_s);

        QCOMPARE(model.index(1, 0).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 1).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 2).data().toLongLong(), 25);
        QCOMPARE(model.index(1, 3).data(), u"Bob"_s);
    }

    void testInsertData() {
        QSqlTableModel nameModel;
        nameModel.setTable(u"names"_s);
        QVERIFY(nameModel.select());

        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        model.setRelation(1, &nameModel, 1, 0);
        QVERIFY(model.select());
        QCOMPARE(model.rowCount(), 2);

        auto record = model.record();
        record.setGenerated(0, false); // Do not generate the ID
        record.setValue(1, 3);
        record.setValue(u"age"_s, 7);
        QVERIFY(model.insertRecord(model.rowCount(), record));
        QVERIFY(model.submitAll());

        // Check if the model returns the data.
        QCOMPARE(model.rowCount(), 3);

        QCOMPARE(model.index(0, 0).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 1).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 2).data().toLongLong(), 30);
        QCOMPARE(model.index(0, 3).data(), u"Alice"_s);

        QCOMPARE(model.index(1, 0).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 1).data().toLongLong(), 2);
        QCOMPARE(model.index(1, 2).data().toLongLong(), 25);
        QCOMPARE(model.index(1, 3).data(), u"Bob"_s);

        QCOMPARE(model.index(2, 0).data().toLongLong(), 3);
        QCOMPARE(model.index(2, 1).data().toLongLong(), 3);
        QCOMPARE(model.index(2, 2).data().toLongLong(), 7);
        QCOMPARE(model.index(2, 3).data(), u"Chad"_s);

        // Check the actual table.
        QSqlQueryModel rawModel;
        rawModel.setQuery(u"SELECT COUNT(*) AS co FROM people"_s);
        QVERIFY2(!rawModel.lastError().isValid(),rawModel.lastError().text().toLocal8Bit().data());
        QCOMPARE(rawModel.record(0).value(u"co"_s).toInt(), 3);
    }

    void testChangesInForeignTablePropagate() {
        QSqlTableModel nameModel;
        nameModel.setTable(u"names"_s);
        QVERIFY(nameModel.select());

        CustomSqlRelationalModel model;
        model.setTable(u"people"_s);
        model.setRelation(1, &nameModel, 1, 0);
        QVERIFY(model.select());
        QCOMPARE(model.rowCount(), 2);

        QCOMPARE(model.index(0, 0).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 1).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 2).data().toLongLong(), 30);
        QCOMPARE(model.index(0, 3).data(), u"Alice"_s);

        QSignalSpy spy(&model, &CustomSqlRelationalModel::dataChanged);

        auto alice = nameModel.record(0);
        alice.setValue(u"name"_s, u"Annelies"_s);
        QVERIFY2(nameModel.setRecord(0, alice),nameModel.lastError().text().toLocal8Bit().data());
        QVERIFY(nameModel.submitAll());
        QVERIFY(spy.count() >= 1);

        QCOMPARE(model.index(0, 0).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 1).data().toLongLong(), 1);
        QCOMPARE(model.index(0, 2).data().toLongLong(), 30);
        QCOMPARE(model.index(0, 3).data(), u"Annelies"_s);
    }
};

QTEST_MAIN(TestCustomSqlRelationalModel)

#include "custom_sql_relational_model.moc"

#pragma clang diagnostic pop
