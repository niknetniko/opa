/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "claude_ai_service.h"

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
#include <stdexcept>
#include <utility>

using namespace Qt::StringLiterals;

ClaudeAiService::ClaudeAiService(QString model, QObject* parent) : AiService(parent), model(std::move(model)) {
}

QFuture<QString>
ClaudeAiService::ask(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema) {
    qCDebug(OPA) << "Claude AI request starting: model=" << model << "userMessage length=" << userMessage.size();

    return spawn([this, systemPrompt, userMessage, schema]() -> QCoro::Task<QString> {
        const QString apiKey = co_await readFromKeychain(KeychainKeys::Service, KeychainKeys::ClaudeApiKey, this);

        qCDebug(OPA) << "Claude AI: sending request, model=" << model << "schema empty=" << schema.isEmpty();

        QNetworkRequest request(QUrl(u"https://api.anthropic.com/v1/messages"_s));
        request.setHeader(QNetworkRequest::ContentTypeHeader, u"application/json"_s);
        request.setRawHeader("x-api-key", apiKey.toUtf8());
        request.setRawHeader("anthropic-version", "2023-06-01");

        QJsonObject body;
        body[u"model"_s] = model;
        body[u"max_tokens"_s] = 4096;
        body[u"system"_s] = systemPrompt;
        body[u"messages"_s] = QJsonArray{
            QJsonObject{
                {u"role"_s, u"user"_s},
                {u"content"_s, userMessage},
            },
        };

        const bool useTools = !schema.isEmpty();
        if (useTools) {
            body[u"tools"_s] = QJsonArray{
                QJsonObject{
                    {u"name"_s, u"extract_source"_s},
                    {u"description"_s, u"Extract genealogical source metadata from text"_s},
                    {u"input_schema"_s, schema},
                },
            };
            body[u"tool_choice"_s] = QJsonObject{
                {u"type"_s, u"tool"_s},
                {u"name"_s, u"extract_source"_s},
            };
        }

        auto* reply = co_await network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

        const auto replyError = reply->error();
        const auto replyErrorString = reply->errorString();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();
        reply->deleteLater();

        if (replyError != QNetworkReply::NoError) {
            qCWarning(OPA) << "Claude AI: network error:" << replyErrorString;
            throw std::runtime_error(replyErrorString.toStdString());
        }

        if (status < 200 || status >= 300) {
            qCWarning(OPA) << "Claude AI: HTTP error" << status << data;
            throw std::runtime_error(i18n("HTTP %1: %2").arg(status).arg(QString::fromUtf8(data)).toStdString());
        }

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray content = doc[u"content"_s].toArray();
        if (content.isEmpty()) {
            qCWarning(OPA) << "Claude AI: empty content array in response";
            throw std::runtime_error(i18n("Empty response from Claude API").toStdString());
        }

        QString text;
        if (useTools && content.first()[u"type"_s].toString() == u"tool_use"_s) {
            const QJsonObject input = content.first()[u"input"_s].toObject();
            text = QString::fromUtf8(QJsonDocument(input).toJson(QJsonDocument::Compact));
        } else {
            text = content.first()[u"text"_s].toString();
        }

        qCDebug(OPA) << "Claude AI: response received, length=" << text.size();
        co_return text;
    }());
}
