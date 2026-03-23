/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QObject>
#include <optional>

class BatchGuard;

/**
 * Event bus for database stuff.
 *
 * Thread-safe: This class uses thread-local storage for batching.
 * It is safe to call `notifyChanged` and `batchNotifications` from background threads.
 */
class DataEventBroker : public QObject {
    Q_OBJECT
public:
    DataEventBroker(const DataEventBroker&) = delete;
    DataEventBroker& operator=(const DataEventBroker&) = delete;

    static DataEventBroker& instance() {
        static DataEventBroker _instance;
        return _instance;
    }

    ~DataEventBroker() override = default;

    template<typename T>
    void notifyChanged(std::optional<IntegerPrimaryKey> id) {
        static_assert(Schema::is_table_tag<T>, "notifyChanged must be called with a type from the Schema namespace.");

        if (isBatching()) {
            enqueueNotification(T::table, id);
        } else {
            Q_EMIT entityChanged(T::table, id);
        }
    }

    /**
     * Get a guard to start RAII batching of notifications.
     *
     * In most cases, you should use `executeInTransaction` instead, so the database is also rolled back
     * if something goes wrong, which should also cancel the notifications.
     */
    [[nodiscard]] BatchGuard batchNotifications();

Q_SIGNALS:
    void entityChanged(const QString& tableName, std::optional<IntegerPrimaryKey> id);

private:
    friend class BatchGuard;

    DataEventBroker() = default;

    void enqueueNotification(const QString& table, std::optional<IntegerPrimaryKey> id) const;
    void flushNotifications();
    void discardNotifications() const;

    bool isBatching() const;
    void pushBatch() const;
    void popBatch();
};

class BatchGuard {
public:
    explicit BatchGuard(DataEventBroker& broker);
    ~BatchGuard();
    BatchGuard(const BatchGuard&) = delete;
    BatchGuard& operator=(const BatchGuard&) = delete;
    BatchGuard(BatchGuard&&) = delete;
    BatchGuard& operator=(BatchGuard&&) = delete;

    /**
     * Discard all pending notifications queued during this batch.
     *
     * Call this when a transaction is rolled back so that views are not
     * notified about changes that were never committed.
     */
    void discard() const;

private:
    DataEventBroker& broker;
};

template<typename T>
static void connectToTable(QObject* receiver, const std::function<void(std::optional<IntegerPrimaryKey>)>& callback) {
    auto* broker = &DataEventBroker::instance();
    QObject::connect(
        broker,
        &DataEventBroker::entityChanged,
        receiver,
        [callback](const QString& table, std::optional<IntegerPrimaryKey> id) {
            if (table == T::table) {
                callback(id);
            }
        }
    );
}

template<typename T>
static void connectToTable(QObject* receiver, const std::function<void()>& callback) {
    connectToTable<T>(receiver, [callback](std::optional<IntegerPrimaryKey>) { callback(); });
}

template<typename T>
static void connectToTable(QObject* receiver, IntegerPrimaryKey targetId, const std::function<void()>& callback) {
    connectToTable<T>(receiver, [callback, targetId](std::optional<IntegerPrimaryKey> id) {
        if (id == targetId || !id) {
            callback();
        }
    });
}

template<typename T, typename Receiver>
static void connectToTable(Receiver* receiver, IntegerPrimaryKey targetId) {
    connectToTable<T>(receiver, targetId, [receiver] { receiver->reload(); });
}

template<typename T, typename Receiver>
static void connectToTable(Receiver* receiver) {
    connectToTable<T>(receiver, [receiver] { receiver->reload(); });
}
