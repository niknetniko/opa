/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppDFAUnreachableFunctionCall
#include "./test_utils.h"
#include "core/data_event_broker.h"
#include "database/database.h"
#include "database/schema.h"
#include "domain/event/event_repository.h"

#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class TestTransactionBatching : public QObject {
    Q_OBJECT

    IntegerPrimaryKey insertPerson() {
        return insertQuery(u"INSERT INTO people (root, sex) VALUES (false, 'Unknown')"_s);
    }

    IntegerPrimaryKey insertEventType(const QString& type = u"Birth"_s) {
        return insertQuery(u"INSERT INTO event_types (type, builtin) VALUES ('%1', false)"_s.arg(type));
    }

    IntegerPrimaryKey insertEventRole(const QString& role = u"Primary"_s) {
        return insertQuery(u"INSERT INTO event_roles (role, builtin) VALUES ('%1', false)"_s.arg(role));
    }

private Q_SLOTS:
    void init() {
        QVERIFY(QSqlDatabase::isDriverAvailable(u"QSQLITE"_s));
        openDatabase(u":memory:"_s, false);
    }

    void cleanup() {
        auto db = QSqlDatabase::database();
        db.close();
    }

    // ==================== BatchGuard basic behavior ====================

    void testNotificationsFireImmediatelyWithoutBatching() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        DataEventBroker::instance().notifyChanged<Schema::Events>(1);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), Schema::Events::table);
        QCOMPARE(spy.at(0).at(1).value<std::optional<IntegerPrimaryKey>>(), 1);
    }

    void testBatchGuardDefersNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            QCOMPARE(spy.count(), 0);
        }

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), Schema::Events::table);
    }

    void testBatchGuardFlushesOnDestruction() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::EventTypes>(2);
            DataEventBroker::instance().notifyChanged<Schema::EventRoles>(3);
            QCOMPARE(spy.count(), 0);
        }

        QCOMPARE(spy.count(), 3);
    }

    void testBatchGuardDeduplicatesNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            QCOMPARE(spy.count(), 0);
        }

        QCOMPARE(spy.count(), 1);
    }

    void testBatchGuardDeduplicatesDifferentTablesIndependently() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::EventRelations>(1);
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::EventRelations>(1);
        }

        QCOMPARE(spy.count(), 2);
    }

    void testBatchGuardDeduplicatesSameTableDifferentIds() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::Events>(2);
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
        }

        QCOMPARE(spy.count(), 2);
    }

    void testBatchGuardHandlesNulloptId() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(std::nullopt);
            DataEventBroker::instance().notifyChanged<Schema::Events>(std::nullopt);
        }

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(1).value<std::optional<IntegerPrimaryKey>>(), std::nullopt);
    }

    void testBatchGuardNulloptAndConcreteIdAreSeparate() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(std::nullopt);
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
        }

        QCOMPARE(spy.count(), 2);
    }

    void testBatchGuardEmptyFlushEmitsNothing() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
        }

        QCOMPARE(spy.count(), 0);
    }

    // ==================== Nesting ====================

    void testNestedBatchGuardsOnlyFlushOnOuterDestruction() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto outer = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);

            {
                auto inner = DataEventBroker::instance().batchNotifications();
                DataEventBroker::instance().notifyChanged<Schema::EventTypes>(2);
                QCOMPARE(spy.count(), 0);
            }

            // Inner guard destructed, but outer is still active — no flush yet.
            QCOMPARE(spy.count(), 0);
            DataEventBroker::instance().notifyChanged<Schema::EventRoles>(3);
        }

        // Outer guard destructed — all three notifications flush.
        QCOMPARE(spy.count(), 3);
    }

    void testTripleNestedBatchGuards() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto level1 = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);

            {
                auto level2 = DataEventBroker::instance().batchNotifications();
                DataEventBroker::instance().notifyChanged<Schema::Events>(2);

                {
                    auto level3 = DataEventBroker::instance().batchNotifications();
                    DataEventBroker::instance().notifyChanged<Schema::Events>(3);
                }

                QCOMPARE(spy.count(), 0);
            }

            QCOMPARE(spy.count(), 0);
        }

        QCOMPARE(spy.count(), 3);
    }

    void testNestedBatchGuardsDeduplicateAcrossLevels() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto outer = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);

            {
                auto inner = DataEventBroker::instance().batchNotifications();
                // Same notification as the outer — should be deduplicated.
                DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            }
        }

        QCOMPARE(spy.count(), 1);
    }

    // ==================== executeInTransaction + batching ====================

    void testExecuteInTransactionBatchesNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            EventRepository repo;
            auto eventId = repo.insertEvent(typeId);
            // Notification should be queued, not emitted.
            VERIFY_OR_THROW(spy.count() == 0);
            return eventId;
        });

        QVERIFY(result.has_value());
        // After transaction commits, notifications flush.
        QVERIFY(spy.count() > 0);
    }

    void testExecuteInTransactionNoNotificationsOnRollback() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            EventRepository repo;
            repo.insertEvent(typeId);
            // Signal abort — transaction will be rolled back.
            return std::nullopt;
        });

        QVERIFY(!result.has_value());
        // Notifications are discarded on rollback — views should not reload.
        QCOMPARE(spy.count(), 0);
    }

    void testNestedTransactionDoesNotStartNewTransaction() {
        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            // Inner transaction should detect active transaction and skip.
            EventRepository repo;
            auto eventId = repo.insertEventWithRelation(typeId, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());
            return true;
        });

        QVERIFY(result.has_value());

        // Verify the data was actually committed.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 1);
    }

    void testNestedTransactionNotificationsFlushOnOuterCommit() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            // insertEventWithRelation calls executeInTransaction internally.
            auto eventId = repo.insertEventWithRelation(typeId, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            // Inner transaction's guard destructed, but outer is still active.
            VERIFY_OR_THROW(spy.count() == 0);

            return true;
        });

        QVERIFY(result.has_value());
        // All notifications flush after the outer transaction commits.
        QVERIFY(spy.count() > 0);
    }

    void testInsertFullEventInOuterTransaction() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            EventRepository repo;

            auto eventId = repo.insertFullEvent(typeId, u"1900-01-01"_s, u"Test"_s, u"Note"_s, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            // Nothing should have been emitted yet.
            VERIFY_OR_THROW(spy.count() == 0);

            return eventId;
        });

        QVERIFY(result.has_value());
        QVERIFY(spy.count() > 0);

        // Verify data is committed.
        EventRepository repo;
        auto event = repo.findEventById(*result);
        QVERIFY(event.has_value());
        QCOMPARE(event->date, u"1900-01-01"_s);
        QCOMPARE(event->name, u"Test"_s);
        QCOMPARE(event->note, u"Note"_s);

        auto relations = repo.findRelationsForEvent(*result);
        QCOMPARE(relations.size(), 1);
        QCOMPARE(relations.first().personId, personId);
    }

    // ==================== executeInTransaction nesting correctness ====================

    void testHasActiveTransactionDuringTransaction() {
        QVERIFY(!hasActiveTransaction());

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            VERIFY_OR_THROW(hasActiveTransaction());
            return true;
        });

        QVERIFY(result.has_value());
        QVERIFY(!hasActiveTransaction());
    }

    void testNestedExecuteInTransactionSkipsInnerTransaction() {
        auto typeId = insertEventType();

        int outerInserts = 0;
        int innerInserts = 0;

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            auto id1 = repo.insertEvent(typeId);
            VERIFY_OR_THROW(id1.has_value());
            outerInserts++;

            // Nested transaction — should just run the lambda.
            auto innerResult = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
                auto id2 = repo.insertEvent(typeId);
                VERIFY_OR_THROW(id2.has_value());
                innerInserts++;
                return id2;
            });
            VERIFY_OR_THROW(innerResult.has_value());

            return true;
        });

        QVERIFY(result.has_value());
        QCOMPARE(outerInserts, 1);
        QCOMPARE(innerInserts, 1);

        // Both events should exist.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 2);
    }

    void testInnerTransactionFailureRollsBackOuter() {
        auto typeId = insertEventType();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            auto id1 = repo.insertEvent(typeId);
            VERIFY_OR_THROW(id1.has_value());

            // Nested call returns nullopt — signals failure.
            auto innerResult = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> { return std::nullopt; });

            if (!innerResult.has_value()) {
                return std::nullopt;
            }

            return true;
        });

        // Outer transaction should have rolled back.
        QVERIFY(!result.has_value());

        // The event created before the inner failure should be rolled back.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 0);
    }

    // ==================== Composite repository methods ====================

    void testInsertEventWithRelationBatchesNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = EventRepository().insertEventWithRelation(typeId, personId, roleId);
        QVERIFY(result.has_value());

        // Notifications should have been flushed after the transaction.
        QVERIFY(spy.count() > 0);

        // Check that both Events and EventRelations notifications were emitted.
        bool hasEventNotification = false;
        bool hasRelationNotification = false;
        for (const auto& call: spy) {
            auto table = call.at(0).toString();
            if (table == Schema::Events::table) {
                hasEventNotification = true;
            }
            if (table == Schema::EventRelations::table) {
                hasRelationNotification = true;
            }
        }
        QVERIFY(hasEventNotification);
        QVERIFY(hasRelationNotification);
    }

    void testInsertFullEventBatchesNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = EventRepository().insertFullEvent(typeId, u"1900-01-01"_s, u"Test"_s, u""_s, personId, roleId);
        QVERIFY(result.has_value());

        // Check that Events and EventRelations notifications were emitted.
        bool hasEventNotification = false;
        bool hasRelationNotification = false;
        for (const auto& call: spy) {
            auto table = call.at(0).toString();
            if (table == Schema::Events::table) {
                hasEventNotification = true;
            }
            if (table == Schema::EventRelations::table) {
                hasRelationNotification = true;
            }
        }
        QVERIFY(hasEventNotification);
        QVERIFY(hasRelationNotification);
    }

    void testInsertFullEventDataIntegrity() {
        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();

        auto eventId = EventRepository().insertFullEvent(
            typeId, u"1950-06-15"_s, u"Birth of John"_s, u"<p>Hospital</p>"_s, personId, roleId
        );
        QVERIFY(eventId.has_value());

        EventRepository repo;
        auto event = repo.findEventById(*eventId);
        QVERIFY(event.has_value());
        QCOMPARE(event->typeId, typeId);
        QCOMPARE(event->date, u"1950-06-15"_s);
        QCOMPARE(event->name, u"Birth of John"_s);
        QCOMPARE(event->note, u"<p>Hospital</p>"_s);

        auto relations = repo.findRelationsForEvent(*eventId);
        QCOMPARE(relations.size(), 1);
        QCOMPARE(relations.first().personId, personId);
        QCOMPARE(relations.first().roleId, roleId);
    }

    // ==================== Multi-operation caller transaction ====================

    void testCallerTransactionWithMultipleRepoCalls() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            // Simulate what EventEditorDialog::accept() does.
            auto eventId = repo.insertFullEvent(typeId, u"1900-01-01"_s, u"Test"_s, u""_s, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            // Additional operations that would fail without batching.
            bool ok = repo.updateEvent(*eventId, typeId, u"1900-02-02"_s, u"Updated"_s, u"New note"_s);
            VERIFY_OR_THROW(ok);

            // All notifications should still be queued.
            VERIFY_OR_THROW(spy.count() == 0);

            return true;
        });

        QVERIFY(result.has_value());
        QVERIFY(spy.count() > 0);

        // Verify final state.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 1);
    }

    void testCallerTransactionWithCitations() {
        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        auto sourceId = insertQuery(u"INSERT INTO sources (title, confidence) VALUES ('Test Source', 'High')"_s);

        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            auto eventId = repo.insertFullEvent(typeId, u"1900-01-01"_s, u"Test"_s, u""_s, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            bool ok = repo.addEventCitation(*eventId, sourceId);
            VERIFY_OR_THROW(ok);

            // Look up the relation id to add a citation.
            auto relations = repo.findRelationsForEvent(*eventId);
            VERIFY_OR_THROW(relations.size() == 1);
            auto relationId = relations.first().id;

            ok = repo.addEventRelationCitation(relationId, sourceId);
            VERIFY_OR_THROW(ok);

            VERIFY_OR_THROW(spy.count() == 0);

            return true;
        });

        QVERIFY(result.has_value());
        QVERIFY(spy.count() > 0);

        // Verify the citations were committed.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 1);

        auto eventId = events.first().id;
        auto eventCitations = repo.findCitationsForEvent(eventId);
        QCOMPARE(eventCitations.size(), 1);

        auto relations = repo.findRelationsForEvent(eventId);
        QCOMPARE(relations.size(), 1);
        auto relationCitations = repo.findCitationsForEventRelation(relations.first().id);
        QCOMPARE(relationCitations.size(), 1);
    }

    void testCallerTransactionRollbackOnCitationFailure() {
        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            auto eventId = repo.insertFullEvent(typeId, u"1900-01-01"_s, u"Test"_s, u""_s, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            // Try to add a citation with a non-existent source — fails due to FK constraint.
            bool ok = repo.addEventCitation(*eventId, 99999);
            if (!ok) {
                return std::nullopt;
            }

            return true;
        });

        QVERIFY(!result.has_value());

        // Event should have been rolled back.
        EventRepository repo;
        auto events = repo.findAllEvents();
        QCOMPARE(events.size(), 0);
    }

    // ==================== Notification ordering ====================

    void testNotificationsPreserveInsertionOrder() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::EventRelations>(2);
            DataEventBroker::instance().notifyChanged<Schema::EventTypes>(3);
        }

        QCOMPARE(spy.count(), 3);
        QCOMPARE(spy.at(0).at(0).toString(), Schema::Events::table);
        QCOMPARE(spy.at(1).at(0).toString(), Schema::EventRelations::table);
        QCOMPARE(spy.at(2).at(0).toString(), Schema::EventTypes::table);
    }

    // ==================== Multiple sequential transactions ====================

    void testSequentialTransactionsEachFlushIndependently() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        spy.clear();

        // First transaction.
        auto result1 = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            return EventRepository().insertEvent(typeId);
        });
        QVERIFY(result1.has_value());
        auto countAfterFirst = spy.count();
        QVERIFY(countAfterFirst > 0);

        // Second transaction.
        auto result2 = executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            return EventRepository().insertEvent(typeId);
        });
        QVERIFY(result2.has_value());
        QVERIFY(spy.count() > countAfterFirst);
    }

    // ==================== BatchGuard::discard ====================

    void testDiscardClearsPendingNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto guard = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);
            DataEventBroker::instance().notifyChanged<Schema::EventTypes>(2);
            QCOMPARE(spy.count(), 0);

            guard.discard();
        }

        // Notifications were discarded — nothing emitted on destruction.
        QCOMPARE(spy.count(), 0);
    }

    void testDiscardInNestedBatchGuard() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        {
            auto outer = DataEventBroker::instance().batchNotifications();
            DataEventBroker::instance().notifyChanged<Schema::Events>(1);

            {
                auto inner = DataEventBroker::instance().batchNotifications();
                DataEventBroker::instance().notifyChanged<Schema::EventTypes>(2);

                // Discard clears ALL pending notifications (shared queue).
                inner.discard();
            }

            // Inner destructed but outer still active — no flush yet.
            QCOMPARE(spy.count(), 0);
        }

        // Outer destructed — queue was cleared by inner discard, so nothing emitted.
        QCOMPARE(spy.count(), 0);
    }

    void testTransactionRollbackAfterExceptionDiscardsNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        spy.clear();

        bool exceptionCaught = false;
        try {
            executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
                EventRepository repo;
                repo.insertEvent(typeId);
                throw std::runtime_error("test failure");
            });
        } catch (const std::runtime_error&) {
            exceptionCaught = true;
        }

        QVERIFY(exceptionCaught);
        // Notifications should have been discarded on the exception rollback path.
        QCOMPARE(spy.count(), 0);
    }

    void testCallerTransactionRollbackDiscardsAllNestedNotifications() {
        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        auto typeId = insertEventType();
        auto personId = insertPerson();
        auto roleId = insertEventRole();
        spy.clear();

        auto result = executeInTransaction([&]() -> std::optional<bool> {
            EventRepository repo;

            // This nested call queues notifications for Events and EventRelations.
            auto eventId = repo.insertEventWithRelation(typeId, personId, roleId);
            VERIFY_OR_THROW(eventId.has_value());

            // Signal abort — outer transaction rolls back.
            return std::nullopt;
        });

        QVERIFY(!result.has_value());
        // All notifications (including from nested repo calls) should be discarded.
        QCOMPARE(spy.count(), 0);
    }

    void testBatchingDoesNotLeakBetweenTransactions() {
        auto typeId = insertEventType();

        QSignalSpy spy(&DataEventBroker::instance(), &DataEventBroker::entityChanged);
        QVERIFY(spy.isValid());

        // First transaction with batching.
        executeInTransaction([&]() -> std::optional<IntegerPrimaryKey> {
            return EventRepository().insertEvent(typeId);
        });

        spy.clear();

        // After the transaction, notifications should fire immediately again.
        DataEventBroker::instance().notifyChanged<Schema::Events>(999);
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TestTransactionBatching)

#include "transaction_batching_test.moc"
