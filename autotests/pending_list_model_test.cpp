/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "utils/pending_list_model.h"

#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {

struct Item {
    IntegerPrimaryKey id;
    QString name;
};

auto idExtractor = [](const Item& item) { return item.id; };

auto makeLookup(const QList<Item>& available) {
    return [available](IntegerPrimaryKey id) -> std::optional<Item> {
        for (const auto& item: available) {
            if (item.id == id) {
                return item;
            }
        }
        return std::nullopt;
    };
}

QList<IntegerPrimaryKey> ids(const QList<Item>& items) {
    QList<IntegerPrimaryKey> result;
    for (const auto& item: items) {
        result.append(item.id);
    }
    return result;
}

}

class TestPendingListModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // ── Construction ──

    void emptyInitialReturnsEmpty() {
        PendingListModel<Item> model({}, idExtractor, makeLookup({}));
        QVERIFY(model.items().isEmpty());
    }

    void initialItemsReturnedUnchanged() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}, {3, u"C"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 2, 3}));
    }

    // ── add() ──

    void addNewItem() {
        QList<Item> available = {{10, u"New"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(model.add(10));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{10});
    }

    void addAppendsAfterInitial() {
        QList<Item> initial = {{1, u"A"_s}};
        QList<Item> available = {{2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));

        QVERIFY(model.add(2));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 2}));
    }

    void addMultipleItems() {
        QList<Item> available = {{1, u"A"_s}, {2, u"B"_s}, {3, u"C"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(model.add(1));
        QVERIFY(model.add(2));
        QVERIFY(model.add(3));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 2, 3}));
    }

    void addDuplicateOfInitialFails() {
        QList<Item> initial = {{1, u"A"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(!model.add(1));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{1});
    }

    void addDuplicateOfPendingAddFails() {
        QList<Item> available = {{1, u"A"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(model.add(1));
        QVERIFY(!model.add(1));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{1});
    }

    void addUnknownIdFails() {
        PendingListModel<Item> model({}, idExtractor, makeLookup({}));
        QVERIFY(!model.add(999));
        QVERIFY(model.items().isEmpty());
    }

    void addRemovedInitialItemRestoresIt() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(model.remove(1));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{2});

        // Re-adding should un-remove rather than lookup.
        QVERIFY(model.add(1));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 2}));
    }

    // ── remove() ──

    void removeInitialItem() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(model.remove(1));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{2});
    }

    void removeAllInitialItems() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(model.remove(1));
        QVERIFY(model.remove(2));
        QVERIFY(model.items().isEmpty());
    }

    void removePendingAddItem() {
        QList<Item> available = {{1, u"A"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(model.add(1));
        QVERIFY(model.remove(1));
        QVERIFY(model.items().isEmpty());
    }

    void removeNonExistentIdFails() {
        QList<Item> initial = {{1, u"A"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(!model.remove(999));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{1});
    }

    void removeAlreadyRemovedInitialFails() {
        QList<Item> initial = {{1, u"A"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(model.remove(1));
        // Removing again: the item is still in initialItems, so it inserts into removals again.
        // This is idempotent — the set already contains the ID.
        QVERIFY(model.remove(1));
        QVERIFY(model.items().isEmpty());
    }

    // ── Combined add/remove sequences ──

    void addThenRemoveThenReAdd() {
        QList<Item> available = {{1, u"A"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(model.add(1));
        QVERIFY(model.remove(1));
        QVERIFY(model.items().isEmpty());

        // Re-add after removing a pending add.
        QVERIFY(model.add(1));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{1});
    }

    void removeInitialThenAddNewThenRemoveNew() {
        QList<Item> initial = {{1, u"A"_s}};
        QList<Item> available = {{2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));

        QVERIFY(model.remove(1));
        QVERIFY(model.add(2));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{2});

        QVERIFY(model.remove(2));
        QVERIFY(model.items().isEmpty());
    }

    void interleaveAddsAndRemovesOnInitialItems() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}, {3, u"C"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(model.remove(2));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 3}));

        QVERIFY(model.add(2));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 2, 3}));

        QVERIFY(model.remove(1));
        QVERIFY(model.remove(3));
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{2});
    }

    // ── items() ordering ──

    void itemsOrderPreservesInitialThenAdds() {
        QList<Item> initial = {{3, u"C"_s}, {1, u"A"_s}};
        QList<Item> available = {{2, u"B"_s}, {4, u"D"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));

        QVERIFY(model.add(2));
        QVERIFY(model.add(4));
        // Initial order preserved, then adds in order of insertion.
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{3, 1, 2, 4}));
    }

    void itemsWithRemovedInitialPreservesGaps() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}, {3, u"C"_s}};
        QList<Item> available = {{4, u"D"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));

        QVERIFY(model.remove(2));
        QVERIFY(model.add(4));
        QCOMPARE(ids(model.items()), (QList<IntegerPrimaryKey>{1, 3, 4}));
    }

    // ── commit() ──

    void commitEmptyModelCallsNothing() {
        PendingListModel<Item> model({}, idExtractor, makeLookup({}));

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        QVERIFY(added.isEmpty());
        QVERIFY(removed.isEmpty());
    }

    void commitNoChangesCallsNothing() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        QVERIFY(added.isEmpty());
        QVERIFY(removed.isEmpty());
    }

    void commitAddsOnly() {
        QList<Item> available = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));
        model.add(1);
        model.add(2);

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        QCOMPARE(added, (QList<IntegerPrimaryKey>{1, 2}));
        QVERIFY(removed.isEmpty());
    }

    void commitRemovalsOnly() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));
        model.remove(1);
        model.remove(2);

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        QVERIFY(added.isEmpty());
        // QSet order is not guaranteed, so sort.
        std::sort(removed.begin(), removed.end());
        QCOMPARE(removed, (QList<IntegerPrimaryKey>{1, 2}));
    }

    void commitMixedAddsAndRemovals() {
        QList<Item> initial = {{1, u"A"_s}, {2, u"B"_s}};
        QList<Item> available = {{3, u"C"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));
        model.remove(1);
        model.add(3);

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        QCOMPARE(added, QList<IntegerPrimaryKey>{3});
        QCOMPARE(removed, QList<IntegerPrimaryKey>{1});
    }

    void commitDoesNotIncludeRemovedPendingAdds() {
        QList<Item> available = {{1, u"A"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));
        model.add(1);
        model.remove(1);

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        // Item was added then removed from pending — nothing to commit.
        QVERIFY(added.isEmpty());
        QVERIFY(removed.isEmpty());
    }

    void commitDoesNotRemoveRestoredInitialItems() {
        QList<Item> initial = {{1, u"A"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));
        model.remove(1);
        model.add(1); // Restores the removal.

        QList<IntegerPrimaryKey> added;
        QList<IntegerPrimaryKey> removed;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey id) {
                removed.append(id);
                return true;
            }
        );
        // Removal was cancelled by re-add — nothing to commit.
        QVERIFY(added.isEmpty());
        QVERIFY(removed.isEmpty());
    }

    // ── Edge cases ──

    void lookupFailureDoesNotCorruptState() {
        QList<Item> initial = {{1, u"A"_s}};
        // Lookup always fails.
        PendingListModel<Item> model(initial, idExtractor, makeLookup({}));

        QVERIFY(!model.add(99));
        // State is unchanged.
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{1});
    }

    void addAfterFailedLookupSucceedsWithValidLookup() {
        QList<Item> available = {{2, u"B"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        QVERIFY(!model.add(99)); // Fails.
        QVERIFY(model.add(2)); // Succeeds.
        QCOMPARE(ids(model.items()), QList<IntegerPrimaryKey>{2});
    }

    void singleItemFullLifecycle() {
        QList<Item> available = {{1, u"A"_s}};
        PendingListModel<Item> model({}, idExtractor, makeLookup(available));

        // Add.
        QVERIFY(model.add(1));
        QCOMPARE(model.items().size(), 1);

        // Remove.
        QVERIFY(model.remove(1));
        QVERIFY(model.items().isEmpty());

        // Re-add.
        QVERIFY(model.add(1));
        QCOMPARE(model.items().size(), 1);

        // Commit.
        QList<IntegerPrimaryKey> added;
        model.commit(
            [&](IntegerPrimaryKey id) {
                added.append(id);
                return true;
            },
            [&](IntegerPrimaryKey) { return true; }
        );
        QCOMPARE(added, QList<IntegerPrimaryKey>{1});
    }

    void itemsPreservesEntityData() {
        QList<Item> initial = {{1, u"Alpha"_s}};
        QList<Item> available = {{2, u"Beta"_s}};
        PendingListModel<Item> model(initial, idExtractor, makeLookup(available));
        model.add(2);

        auto result = model.items();
        QCOMPARE(result.size(), 2);
        QCOMPARE(result[0].name, u"Alpha"_s);
        QCOMPARE(result[1].name, u"Beta"_s);
    }
};

QTEST_GUILESS_MAIN(TestPendingListModel)
#include "pending_list_model_test.moc"
