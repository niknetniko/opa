/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "data_event_broker.h"

#include <algorithm>
#include <vector>

using namespace Qt::StringLiterals;

// Every thread uses its own guard, so notifications are managed per thread.
namespace {
struct PendingNotification {
    QString table;
    std::optional<IntegerPrimaryKey> id;
};

thread_local int batchDepth = 0;
thread_local std::vector<PendingNotification> pendingNotifications;
}

BatchGuard DataEventBroker::batchNotifications() {
    return BatchGuard(*this);
}

void DataEventBroker::enqueueNotification(const QString& table, std::optional<IntegerPrimaryKey> id) const {
    auto it = std::ranges::find_if(pendingNotifications, [&](const PendingNotification& n) {
        return n.table == table && n.id == id;
    });
    if (it == pendingNotifications.end()) {
        pendingNotifications.push_back({table, id});
    }
}

void DataEventBroker::flushNotifications() {
    auto notifications = std::move(pendingNotifications);
    discardNotifications();
    for (const auto& [table, id]: notifications) {
        Q_EMIT entityChanged(table, id);
    }
}

BatchGuard::BatchGuard(DataEventBroker& broker) : broker(broker) {
    broker.pushBatch();
}

BatchGuard::~BatchGuard() {
    broker.popBatch();
}

void BatchGuard::discard() const {
    broker.discardNotifications();
}

bool DataEventBroker::isBatching() const {
    return batchDepth > 0;
}

void DataEventBroker::pushBatch() const {
    ++batchDepth;
}

void DataEventBroker::popBatch() {
    --batchDepth;
    if (batchDepth == 0) {
        flushNotifications();
    }
    Q_ASSERT(batchDepth >= 0);
}

void DataEventBroker::discardNotifications() const {
    pendingNotifications.clear();
}
