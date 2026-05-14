/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "openai_compatible_service.h"

#include "../core/keychain_keys.h"
#include "logging.h"
#include <qt6keychain/keychain.h>

#include <KLocalizedString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <exception>
#include <memory>
#include <stdexcept>
#include <utility>

using namespace Qt::StringLiterals;

OpenAiCompatibleService::OpenAiCompatibleService(QString endpoint, QString model, QObject* parent) :
    AiService(parent),
    endpoint(std::move(endpoint)),
    model(std::move(model)) {
}

QFuture<QString>
OpenAiCompatibleService::ask(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema) {
    qDebug() << "OpenAI-compatible request starting: endpoint=" << endpoint << "model=" << model
             << "userMessage length=" << userMessage.size();

    auto promise = std::make_unique<QPromise<QString>>();
    promise->start();
    auto future = promise->future();

    auto* job = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    job->setKey(KeychainKeys::OpenAiCompatibleApiKey);
    job->setAutoDelete(true);

    connect(
        job,
        &QKeychain::ReadPasswordJob::finished,
        this,
        [this, job, systemPrompt, userMessage, schema, promise = std::move(promise)]() mutable {
            const QString apiKey = (job->error() == QKeychain::NoError) ? job->textData() : QString{};
            doRequest(apiKey, systemPrompt, userMessage, schema, std::move(promise));
        }
    );

    job->start();
    return future;
}

void OpenAiCompatibleService::doRequest(
    const QString& apiKey,
    const QString& systemPrompt,
    const QString& userMessage,
    const QJsonObject& schema,
    std::unique_ptr<QPromise<QString>> promise
) {
    const QUrl url(endpoint + u"/chat/completions"_s);
    qDebug() << "OpenAI-compatible: sending request to" << url << "schema empty=" << schema.isEmpty();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_s);
    if (!apiKey.isEmpty()) {
        request.setRawHeader("Authorization", (u"Bearer "_s + apiKey).toUtf8());
    }

    QJsonObject body;
    body[u"model"_s] = model;
    if (!schema.isEmpty()) {
        body[u"response_format"_s] = QJsonObject{
            {u"type"_s, u"json_schema"_s},
            {u"json_schema"_s,
             QJsonObject{
                 {u"name"_s, u"extract_source"_s},
                 {u"strict"_s, true},
                 {u"schema"_s, schema},
             }},
        };
    }
    body[u"messages"_s] = QJsonArray{
        QJsonObject{
            {u"role"_s, u"system"_s},
            {u"content"_s, systemPrompt},
        },
        QJsonObject{
            {u"role"_s, u"user"_s},
            {u"content"_s, userMessage},
        },
    };

    auto* reply = network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [reply, promise = std::move(promise)]() mutable {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "OpenAI-compatible: network error:" << reply->errorString();
            promise->setException(std::make_exception_ptr(std::runtime_error(reply->errorString().toStdString())));
            promise->finish();
            return;
        }

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();

        if (status < 200 || status >= 300) {
            qWarning() << "OpenAI-compatible: HTTP error" << status << data;
            promise->setException(
                std::make_exception_ptr(
                    std::runtime_error(i18n("HTTP %1: %2").arg(status).arg(QString::fromUtf8(data)).toStdString())
                )
            );
            promise->finish();
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray choices = doc[u"choices"_s].toArray();
        if (choices.isEmpty()) {
            qWarning() << "OpenAI-compatible: empty choices array in response";
            promise->setException(
                std::make_exception_ptr(
                    std::runtime_error(i18n("Empty response from OpenAI-compatible API").toStdString())
                )
            );
            promise->finish();
            return;
        }

        const QString text = choices.first()[u"message"_s][u"content"_s].toString();
        qDebug() << "OpenAI-compatible: response received" << text;
        promise->addResult(text);
        promise->finish();
    });
}
