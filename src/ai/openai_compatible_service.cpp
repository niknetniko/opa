/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "openai_compatible_service.h"

#include "../core/keychain_keys.h"
#include "logging.h"
#include "utils.h"
#include "utils/async.h"

#include <KLocalizedString>
#include <QCoro/QCoroFuture>
#include <QCoro/QCoroNetworkReply>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
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
    qCDebug(OPA) << "OpenAI request starting: endpoint=" << endpoint << "model=" << model
                 << "userMessage length=" << userMessage.size();

    // Pre-compute the request data outside the coroutine. The work below runs in a lambda
    // coroutine that is invoked as a temporary, so its closure object is destroyed at the end of
    // this function while the coroutine is still suspended on its first co_await. Anything the
    // coroutine needs must therefore be passed as a *parameter* (which is copied into the
    // coroutine frame and lives for the coroutine's duration), never as a capture (which lives in
    // the now-destroyed closure and would dangle across suspension points).
    struct RequestData {
        QUrl url;
        QByteArray body;
    };
    auto request = std::make_shared<RequestData>();
    request->url = QUrl(endpoint + u"/chat/completions"_s);

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
    request->body = QJsonDocument(body).toJson(QJsonDocument::Compact);

    qCDebug(OPA) << "OpenAI: sending request to" << request->url << "schema empty=" << schema.isEmpty();

    auto coroutine = [](OpenAiCompatibleService* self,
                        std::shared_ptr<RequestData> request) -> QCoro::Task<QString> {
        const QString apiKey =
            co_await readFromKeychainLax(KeychainKeys::Service, KeychainKeys::OpenAiCompatibleApiKey, self);

        QNetworkRequest req(request->url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_s);
        if (!apiKey.isEmpty()) {
            req.setRawHeader("Authorization", (u"Bearer "_s + apiKey).toUtf8());
        }

        auto* reply = co_await self->network.post(req, request->body);
        const auto replyError = reply->error();
        const auto replyErrorString = reply->errorString();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();
        reply->deleteLater();

        if (replyError != QNetworkReply::NoError) {
            qCWarning(OPA) << "OpenAI: network error:" << replyErrorString;
            throw std::runtime_error(replyErrorString.toStdString());
        }

        if (status < 200 || status >= 300) {
            qCWarning(OPA) << "OpenAI: HTTP error" << status << data;
            throw std::runtime_error(i18n("HTTP %1: %2").arg(status).arg(QString::fromUtf8(data)).toStdString());
        }

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray choices = doc[u"choices"_s].toArray();
        if (choices.isEmpty()) {
            qCWarning(OPA) << "OpenAI: empty choices array in response";
            throw std::runtime_error(i18n("Empty response from OpenAI API").toStdString());
        }

        const QString text = choices.first()[u"message"_s][u"content"_s].toString();
        qCDebug(OPA) << "OpenAI: response received, length=" << text.size();

        co_return text;
    };

    return spawn(coroutine(this, request));
}
