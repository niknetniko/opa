//
// Created by niko on 14/05/2026.
//

#include "utils.h"

#include "logging.h"
#include <qt6keychain/keychain.h>

using namespace Qt::StringLiterals;

QFuture<QString> readFromKeychain(const QString& service, const QString& key, QObject* parent) {
    auto lax = readFromKeychainLax(service, key, parent);

    return lax.then([](QString value) {
        if (value.isEmpty()) {
            throw KeychainException(u"Keychain read failed"_s);
        }

        return value;
    });
}

QFuture<QString> readFromKeychainLax(const QString& service, const QString& key, QObject* parent) {
    QPromise<QString> promise;
    promise.start();

    auto future = promise.future();

    auto* job = new QKeychain::ReadPasswordJob(service, parent);
    job->setKey(key);
    job->setAutoDelete(true);

    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, [job, promise = std::move(promise)]() mutable {
        if (job->error() != QKeychain::NoError) {
            promise.addResult(QString{});
        } else {
            promise.addResult(job->textData());
        }

        promise.finish();
    });

    job->start();
    return future;
}
