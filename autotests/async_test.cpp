/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "utils/async.h"

#include <qcoro/qcorotask.h>
#include <qcoro/qcorotimer.h>

#include <QTest>
#include <stdexcept>

using namespace Qt::Literals::StringLiterals;

namespace {

QCoro::Task<int> immediateSuccess() {
    co_return 42;
}

QCoro::Task<int> immediateFailure() {
    throw std::runtime_error("immediate error");
    co_return 0;
}

QCoro::Task<int> delayedSuccess() {
    co_await QCoro::sleepFor(std::chrono::milliseconds(10));
    co_return 42;
}

QCoro::Task<int> delayedFailure() {
    co_await QCoro::sleepFor(std::chrono::milliseconds(10));
    throw std::runtime_error("delayed error");
    co_return 0;
}

}

class TestSpawn : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testImmediateSuccessSettlesWithResult() {
        auto future = spawn(immediateSuccess());
        QTRY_VERIFY(future.isFinished());
        QCOMPARE(future.result(), 42);
    }

    void testImmediateFailureSettlesWithException() {
        auto future = spawn(immediateFailure());
        QTRY_VERIFY(future.isFinished());
        bool threw = false;
        try {
            future.result();
        } catch (const std::runtime_error& e) {
            threw = true;
            QCOMPARE(e.what(), "immediate error");
        }
        QVERIFY(threw);
    }

    void testDelayedSuccessRequiresEventLoop() {
        auto future = spawn(delayedSuccess());
        QVERIFY(!future.isFinished()); // not done until event loop processes the timer
        QTRY_VERIFY(future.isFinished());
        QCOMPARE(future.result(), 42);
    }

    void testDelayedFailureRequiresEventLoop() {
        auto future = spawn(delayedFailure());
        QVERIFY(!future.isFinished());
        QTRY_VERIFY(future.isFinished());
        bool threw = false;
        try {
            future.result();
        } catch (const std::runtime_error& e) {
            threw = true;
            QCOMPARE(e.what(), "delayed error");
        }
        QVERIFY(threw);
    }

    void testConcurrentSpawnsAreIndependent() {
        // Both tasks are in flight simultaneously; results must not cross.
        auto future1 = spawn(delayedSuccess());
        auto future2 = spawn([]() -> QCoro::Task<int> {
            co_await QCoro::sleepFor(std::chrono::milliseconds(10));
            co_return 99;
        }());

        QTRY_VERIFY(future1.isFinished() && future2.isFinished());
        QCOMPARE(future1.result(), 42);
        QCOMPARE(future2.result(), 99);
    }
};

QTEST_MAIN(TestSpawn)
#include "async_test.moc"
