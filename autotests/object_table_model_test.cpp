/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppUseInternalLinkage
#include "model/object_table_model.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {

struct Item {
    qlonglong id = -1;
    QString name;
    int age = 0;
    std::optional<QString> note;
};

enum Column { ID, NAME, AGE, NOTE };

QList<Item> threeItems() {
    return {
        {.id = 1, .name = u"Alice"_s, .age = 30, .note = u"First"_s},
        {.id = 2, .name = u"Bob"_s, .age = 40, .note = std::nullopt},
        {.id = 3, .name = u"Carol"_s, .age = 50, .note = u"Third"_s},
    };
}

/**
 * Model with the columns most tests need: ID and NAME read-only, AGE editable.
 */
class TestModel : public ObjectTableModel<Item> {
public:
    explicit TestModel(QObject* parent = nullptr) : ObjectTableModel(parent) {
        setColumn(ID, u"Id"_s, &Item::id);
        setColumn(NAME, u"Name"_s, &Item::name);
        setColumn(AGE, u"Age"_s, &Item::age, [](Item& item, const QVariant& value) {
            bool ok = false;
            const int age = value.toInt(&ok);
            if (!ok || age < 0) {
                return false;
            }
            item.age = age;
            return true;
        });
    }

    int rebuildCalls = 0;
    std::function<void()> onRebuild;

protected:
    void rebuild() override {
        ++rebuildCalls;
        if (onRebuild) {
            onRebuild();
        }
    }
};

}

class TestObjectTableModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ── Empty model ──

    void emptyModelHasNoRowsOrColumns() {
        ObjectTableModel<Item> model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 0);
        QVERIFY(model.getItems().isEmpty());
    }

    void columnsWithoutItems() {
        TestModel model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 3);
        QVERIFY(!model.index(0, 0).isValid());
    }

    // ── Columns ──

    void addColumnAppends() {
        ObjectTableModel<Item> model;
        model.addColumn(u"Name"_s, &Item::name);
        model.addColumn(u"Upper"_s, [](const Item& item) -> QVariant { return item.name.toUpper(); });
        model.setItems(threeItems());

        QCOMPARE(model.columnCount(), 2);
        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), u"Name"_s);
        QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), u"Upper"_s);
        QCOMPARE(model.index(0, 0).data().toString(), u"Alice"_s);
        QCOMPARE(model.index(0, 1).data().toString(), u"ALICE"_s);
    }

    void addColumnAfterSetColumnAppendsAtEnd() {
        ObjectTableModel<Item> model;
        model.setColumn(1, u"Name"_s, &Item::name);
        model.addColumn(u"Age"_s, &Item::age);
        QCOMPARE(model.columnCount(), 3);
        QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), u"Age"_s);
    }

    void setColumnReplacesExisting() {
        ObjectTableModel<Item> model;
        model.setColumn(0, u"Name"_s, &Item::name);
        model.setColumn(0, u"Age"_s, &Item::age);
        model.setItems(threeItems());

        QCOMPARE(model.columnCount(), 1);
        QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), u"Age"_s);
        QCOMPARE(model.index(0, 0).data().toInt(), 30);
    }

    void setColumnOutOfOrder() {
        ObjectTableModel<Item> model;
        model.setColumn(2, u"Age"_s, &Item::age);
        model.setColumn(0, u"Id"_s, &Item::id);
        model.setColumn(1, u"Name"_s, &Item::name);
        model.setItems(threeItems());

        QCOMPARE(model.columnCount(), 3);
        QCOMPARE(model.index(1, 0).data().toLongLong(), 2LL);
        QCOMPARE(model.index(1, 1).data().toString(), u"Bob"_s);
        QCOMPARE(model.index(1, 2).data().toInt(), 40);
    }

    void sparseColumnsAreEmpty() {
        ObjectTableModel<Item> model;
        model.setColumn(2, u"Age"_s, &Item::age);
        model.setItems(threeItems());

        QCOMPARE(model.columnCount(), 3);
        QVERIFY(!model.index(0, 0).data().isValid());
        QVERIFY(!model.index(0, 1).data().isValid());
        QVERIFY(model.headerData(0, Qt::Horizontal).toString().isEmpty());
        QVERIFY(!model.setData(model.index(0, 0), 5));
        QVERIFY(!model.flags(model.index(0, 0)).testFlag(Qt::ItemIsEditable));
        QCOMPARE(model.index(0, 2).data().toInt(), 30);
    }

    void extractorColumn() {
        ObjectTableModel<Item> model;
        model.setColumn(0, u"Label"_s, [](const Item& item) -> QVariant {
            return u"%1 (%2)"_s.arg(item.name).arg(item.age);
        });
        model.setItems(threeItems());
        QCOMPARE(model.index(2, 0).data().toString(), u"Carol (50)"_s);
    }

    void memberPointerColumnTypes() {
        TestModel model;
        model.setItems(threeItems());

        const auto id = model.index(0, ID).data();
        QCOMPARE(id.typeId(), QMetaType::LongLong);
        QCOMPARE(id.toLongLong(), 1LL);

        const auto name = model.index(0, NAME).data();
        QCOMPARE(name.typeId(), QMetaType::QString);

        const auto age = model.index(0, AGE).data();
        QCOMPARE(age.typeId(), QMetaType::Int);
    }

    /**
     * Nullable columns are std::optional<QString> (see ai/database_design.md).
     * A member-pointer column on such a field should behave like a string column.
     */
    void memberPointerColumnOnOptionalField() {
        ObjectTableModel<Item> model;
        model.setColumn(0, u"Note"_s, &Item::note);
        model.setItems(threeItems());

        QCOMPARE(model.index(0, 0).data().toString(), u"First"_s);
        QVERIFY(model.index(1, 0).data().isNull());
    }

    // ── Header data ──

    void headerDataHorizontal() {
        TestModel model;
        QCOMPARE(model.headerData(ID, Qt::Horizontal).toString(), u"Id"_s);
        QCOMPARE(model.headerData(NAME, Qt::Horizontal).toString(), u"Name"_s);
        QCOMPARE(model.headerData(AGE, Qt::Horizontal).toString(), u"Age"_s);
    }

    void headerDataOtherRolesAreEmpty() {
        TestModel model;
        QVERIFY(!model.headerData(ID, Qt::Horizontal, Qt::EditRole).isValid());
        QVERIFY(!model.headerData(ID, Qt::Horizontal, Qt::ToolTipRole).isValid());
        QVERIFY(!model.headerData(ID, Qt::Horizontal, Qt::DecorationRole).isValid());
    }

    void headerDataVerticalIsEmpty() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(!model.headerData(0, Qt::Vertical).isValid());
    }

    void headerDataOutOfRange() {
        TestModel model;
        QVERIFY(!model.headerData(3, Qt::Horizontal).isValid());
        QVERIFY(!model.headerData(100, Qt::Horizontal).isValid());
    }

    void headerDataNegativeSection() {
        TestModel model;
        QVERIFY(!model.headerData(-1, Qt::Horizontal).isValid());
    }

    // ── Data ──

    void dataDisplayAndEditRole() {
        TestModel model;
        model.setItems(threeItems());

        for (int row = 0; row < 3; ++row) {
            const auto& expected = threeItems()[row];
            QCOMPARE(model.index(row, ID).data(Qt::DisplayRole).toLongLong(), expected.id);
            QCOMPARE(model.index(row, NAME).data(Qt::DisplayRole).toString(), expected.name);
            QCOMPARE(model.index(row, AGE).data(Qt::DisplayRole).toInt(), expected.age);
            QCOMPARE(model.index(row, NAME).data(Qt::EditRole).toString(), expected.name);
        }
    }

    void dataOtherRolesAreEmpty() {
        TestModel model;
        model.setItems(threeItems());
        const auto index = model.index(0, NAME);
        QVERIFY(!index.data(Qt::DecorationRole).isValid());
        QVERIFY(!index.data(Qt::ToolTipRole).isValid());
        QVERIFY(!index.data(Qt::TextAlignmentRole).isValid());
        QVERIFY(!index.data(Qt::UserRole).isValid());
    }

    void dataInvalidIndex() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(!model.data(QModelIndex()).isValid());
    }

    void indexOutOfRangeIsInvalid() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(!model.index(3, 0).isValid());
        QVERIFY(!model.index(0, 3).isValid());
        QVERIFY(!model.index(-1, 0).isValid());
        QVERIFY(!model.index(0, -1).isValid());
    }

    void dataStaleIndexAfterShrink() {
        TestModel model;
        model.setItems(threeItems());
        const auto stale = model.index(2, NAME);
        model.setItems({threeItems().first()});
        QVERIFY(!model.data(stale).isValid());
    }

    // ── Structure: this is a flat model ──

    void flatStructure() {
        TestModel model;
        model.setItems(threeItems());
        const auto index = model.index(0, 0);
        // QAbstractTableModel hides parent() and hasChildren(); views call them through the base class.
        const QAbstractItemModel& base = model;

        QCOMPARE(model.rowCount(index), 0);
        QCOMPARE(model.columnCount(index), 0);
        QVERIFY(!model.index(0, 0, index).isValid());
        QVERIFY(!base.parent(index).isValid());
        QVERIFY(base.hasChildren());
        QVERIFY(!base.hasChildren(index));
        QVERIFY(model.flags(index).testFlag(Qt::ItemNeverHasChildren));
    }

    void sibling() {
        TestModel model;
        model.setItems(threeItems());
        const auto sibling = model.index(1, ID).siblingAtColumn(NAME);
        QCOMPARE(sibling.data().toString(), u"Bob"_s);
    }

    // ── setItems ──

    void setItemsReplacesItems() {
        TestModel model;
        model.setItems(threeItems());
        QCOMPARE(model.rowCount(), 3);

        model.setItems({{.id = 9, .name = u"Zed"_s, .age = 1, .note = std::nullopt}});
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.getItems().size(), 1);
        QCOMPARE(model.index(0, NAME).data().toString(), u"Zed"_s);
    }

    void setItemsEmpty() {
        TestModel model;
        model.setItems(threeItems());
        model.setItems({});
        QCOMPARE(model.rowCount(), 0);
        QVERIFY(!static_cast<const QAbstractItemModel&>(model).hasChildren());
    }

    void setItemsEmitsOneReset() {
        TestModel model;
        QSignalSpy aboutToReset(&model, &QAbstractItemModel::modelAboutToBeReset);
        QSignalSpy reset(&model, &QAbstractItemModel::modelReset);
        QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);
        QSignalSpy removed(&model, &QAbstractItemModel::rowsRemoved);

        model.setItems(threeItems());
        model.setItems({});

        QCOMPARE(aboutToReset.count(), 2);
        QCOMPARE(reset.count(), 2);
        QCOMPARE(inserted.count(), 0);
        QCOMPARE(removed.count(), 0);
    }

    void setItemsCallsRebuildInsideReset() {
        TestModel model;
        QSignalSpy aboutToReset(&model, &QAbstractItemModel::modelAboutToBeReset);
        QSignalSpy reset(&model, &QAbstractItemModel::modelReset);

        int rowsSeenByRebuild = -1;
        model.onRebuild = [&] {
            QCOMPARE(aboutToReset.count(), 1);
            QCOMPARE(reset.count(), 0);
            rowsSeenByRebuild = static_cast<int>(model.getItems().size());
        };

        model.setItems(threeItems());
        QCOMPARE(model.rebuildCalls, 1);
        QCOMPARE(rowsSeenByRebuild, 3);
    }

    void getItemsReturnsItemsInOrder() {
        TestModel model;
        model.setItems(threeItems());
        const auto& items = model.getItems();
        QCOMPARE(items.size(), 3);
        QCOMPARE(items[0].name, u"Alice"_s);
        QCOMPARE(items[1].name, u"Bob"_s);
        QCOMPARE(items[2].name, u"Carol"_s);
    }

    // ── flags ──

    void flagsEditableOnlyWithSetter() {
        TestModel model;
        model.setItems(threeItems());

        for (int column: {ID, NAME}) {
            const auto flags = model.flags(model.index(0, column));
            QVERIFY(flags.testFlag(Qt::ItemIsEnabled));
            QVERIFY(flags.testFlag(Qt::ItemIsSelectable));
            QVERIFY(!flags.testFlag(Qt::ItemIsEditable));
        }

        const auto flags = model.flags(model.index(0, AGE));
        QVERIFY(flags.testFlag(Qt::ItemIsEnabled));
        QVERIFY(flags.testFlag(Qt::ItemIsSelectable));
        QVERIFY(flags.testFlag(Qt::ItemIsEditable));
    }

    void flagsInvalidIndex() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(!model.flags(QModelIndex()).testFlag(Qt::ItemIsEditable));
    }

    void flagsStaleIndex() {
        TestModel model;
        model.setItems(threeItems());
        const auto stale = model.index(2, AGE);
        model.setItems({threeItems().first()});
        QVERIFY(!model.flags(stale).testFlag(Qt::ItemIsEditable));
    }

    // ── setData ──

    void setDataWithSetter() {
        TestModel model;
        model.setItems(threeItems());
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

        const auto index = model.index(1, AGE);
        QVERIFY(model.setData(index, 41));

        QCOMPARE(model.getItems()[1].age, 41);
        QCOMPARE(index.data().toInt(), 41);
        QCOMPARE(model.getItems()[0].age, 30);
        QCOMPARE(model.getItems()[2].age, 50);

        QCOMPARE(changed.count(), 1);
        const auto arguments = changed.first();
        QCOMPARE(arguments[0].toModelIndex(), index);
        QCOMPARE(arguments[1].toModelIndex(), index);
        QCOMPARE(arguments[2].value<QList<int>>(), (QList<int> {Qt::DisplayRole, Qt::EditRole}));
    }

    void setDataRejectedBySetter() {
        TestModel model;
        model.setItems(threeItems());
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

        QVERIFY(!model.setData(model.index(0, AGE), -5));
        QVERIFY(!model.setData(model.index(0, AGE), u"not a number"_s));
        QCOMPARE(model.getItems()[0].age, 30);
        QCOMPARE(changed.count(), 0);
    }

    void setDataWithoutSetter() {
        TestModel model;
        model.setItems(threeItems());
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

        QVERIFY(!model.setData(model.index(0, NAME), u"Changed"_s));
        QCOMPARE(model.getItems()[0].name, u"Alice"_s);
        QCOMPARE(changed.count(), 0);
    }

    void setDataWrongRole() {
        TestModel model;
        model.setItems(threeItems());
        QSignalSpy changed(&model, &QAbstractItemModel::dataChanged);

        QVERIFY(!model.setData(model.index(0, AGE), 99, Qt::DisplayRole));
        QVERIFY(!model.setData(model.index(0, AGE), 99, Qt::UserRole));
        QCOMPARE(model.getItems()[0].age, 30);
        QCOMPARE(changed.count(), 0);
    }

    void setDataInvalidIndex() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(!model.setData(QModelIndex(), 99));
    }

    void setDataStaleIndex() {
        TestModel model;
        model.setItems(threeItems());
        const auto stale = model.index(2, AGE);
        model.setItems({threeItems().first()});
        QVERIFY(!model.setData(stale, 99));
        QCOMPARE(model.getItems()[0].age, 30);
    }

    void setterReceivesValue() {
        ObjectTableModel<Item> model;
        QVariant received;
        model.setColumn(0, u"Name"_s, &Item::name, [&received](Item& item, const QVariant& value) {
            received = value;
            item.name = value.toString();
            return true;
        });
        model.setItems(threeItems());

        QVERIFY(model.setData(model.index(0, 0), u"Alicia"_s));
        QCOMPARE(received.toString(), u"Alicia"_s);
        QCOMPARE(model.getItems()[0].name, u"Alicia"_s);
    }

    void setterWithExtractorOverload() {
        ObjectTableModel<Item> model;
        model.setColumn(
            0,
            u"Upper"_s,
            [](const Item& item) -> QVariant { return item.name.toUpper(); },
            [](Item& item, const QVariant& value) {
                item.name = value.toString().toLower();
                return true;
            }
        );
        model.setItems(threeItems());

        QVERIFY(model.flags(model.index(0, 0)).testFlag(Qt::ItemIsEditable));
        QVERIFY(model.setData(model.index(0, 0), u"ALICIA"_s));
        QCOMPARE(model.getItems()[0].name, u"alicia"_s);
        QCOMPARE(model.index(0, 0).data().toString(), u"ALICIA"_s);
    }

    // ── itemAt ──

    void itemAtReturnsItem() {
        TestModel model;
        model.setItems(threeItems());

        for (int row = 0; row < 3; ++row) {
            for (int column: {ID, NAME, AGE}) {
                const auto* item = model.itemAt(model.index(row, column));
                QVERIFY(item != nullptr);
                QCOMPARE(item, &model.getItems()[row]);
            }
        }
        QCOMPARE(model.itemAt(model.index(1, NAME))->name, u"Bob"_s);
    }

    void itemAtInvalidIndex() {
        TestModel model;
        model.setItems(threeItems());
        QCOMPARE(model.itemAt(QModelIndex()), nullptr);
    }

    void itemAtStaleIndex() {
        TestModel model;
        model.setItems(threeItems());
        const auto stale = model.index(2, NAME);
        model.setItems({threeItems().first()});
        QCOMPARE(model.itemAt(stale), nullptr);
    }

    void itemAtReflectsSetData() {
        TestModel model;
        model.setItems(threeItems());
        QVERIFY(model.setData(model.index(0, AGE), 31));
        QCOMPARE(model.itemAt(model.index(0, ID))->age, 31);
    }

    // Columns must be defined before items are set; this is enforced with a Q_ASSERT, which QTest cannot check.

    // ── Qt model contract ──

    void modelTesterOnLifecycle() {
        TestModel model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);

        model.setItems(threeItems());
        QVERIFY(model.setData(model.index(1, AGE), 42));
        QVERIFY(!model.setData(model.index(1, NAME), u"x"_s));
        model.setItems({threeItems().first()});
        model.setItems({});
        model.setItems(threeItems());
    }

    void modelTesterEmptyColumns() {
        ObjectTableModel<Item> model;
        QAbstractItemModelTester tester(&model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        model.setItems(threeItems());
    }
};

QTEST_GUILESS_MAIN(TestObjectTableModel)
#include "object_table_model_test.moc"
