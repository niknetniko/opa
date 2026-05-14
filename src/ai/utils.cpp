//
// Created by niko on 14/05/2026.
//

#include "utils.h"

#include "logging.h"
#include <qt6keychain/keychain.h>

QFuture<QString> readFromKeychain(const QString& service, const QString& key, QObject* parent) {
    QPromise<QString> promise;
    promise.start();

    auto future = promise.future();

    auto* job = new QKeychain::ReadPasswordJob(service, parent);
    job->setKey(key);
    job->setAutoDelete(true);

    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, [job, promise = std::move(promise)]() mutable {
        if (job->error() != QKeychain::NoError || job->textData().isEmpty()) {
            qCWarning(OPA) << "Keychain read failed:" << job->errorString();
            promise.setException(KeychainException(job->errorString()));
        } else {
            promise.addResult(job->textData());
        }

        promise.finish();
    });

    job->start();
    return future;
}
