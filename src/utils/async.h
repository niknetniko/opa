/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <qcoro/qcorotask.h>

#include <QFuture>
#include <QPromise>
#include <memory>
#include <optional>

/**
 * Convert a QCoro::Task into a QFuture, allowing coroutines to be used
 * at the boundary of non-coroutine code.
 *
 * The task is driven to completion by Qt's event loop. Any exception thrown
 * inside the task is stored in the returned future and rethrown when the
 * caller calls result() or accesses the value via onFailed().
 */
template<typename T>
QFuture<T> spawn(QCoro::Task<T> task) {
    auto promise = std::make_shared<QPromise<T>>();
    promise->start();
    auto future = promise->future();

    auto keeper = std::make_shared<std::optional<QCoro::Task<void>>>();
    *keeper = std::move(task).then(
        [promise, keeper](T result) mutable {
            promise->addResult(std::move(result));
            promise->finish();
            keeper->reset();
        },
        [promise, keeper](const std::exception&) mutable {
            promise->setException(std::current_exception());
            promise->finish();
            keeper->reset();
        }
    );

    return future;
}
