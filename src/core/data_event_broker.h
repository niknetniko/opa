/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QObject>
#include <optional>

/**
 * Event bus for database stuff.
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

        Q_EMIT entityChanged(T::table, id);
    }

Q_SIGNALS:
    void entityChanged(const QString& tableName, std::optional<IntegerPrimaryKey> id);

private:
    DataEventBroker() = default;
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
