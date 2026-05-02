/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "claude_ai_service.h"

#include "../core/keychain_keys.h"
#include "logging.h"
#include <qt6keychain/keychain.h>

#include <KLocalizedString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <utility>

using namespace Qt::StringLiterals;

ClaudeAiService::ClaudeAiService(QString model, QObject* parent) : AiService(parent), model(std::move(model)) {
}

void ClaudeAiService::complete(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema) {
    qCDebug(OPA) << "Claude AI request starting: model=" << model << "userMessage length=" << userMessage.size();

    auto* job = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    job->setKey(KeychainKeys::ClaudeApiKey);
    job->setAutoDelete(true);

    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job, systemPrompt, userMessage, schema] {
        if (job->error() != QKeychain::NoError || job->textData().isEmpty()) {
            qCWarning(OPA) << "Claude AI: keychain read failed:" << job->errorString();
            Q_EMIT requestFailed(i18n("No API key configured. Set one in Preferences → AI."));
            return;
        }
        doRequest(job->textData(), systemPrompt, userMessage, schema);
    });

    job->start();
}

void ClaudeAiService::doRequest(
    const QString& apiKey,
    const QString& systemPrompt,
    const QString& userMessage,
    const QJsonObject& schema
) {
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

    auto* reply = network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, useTools] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(OPA) << "Claude AI: network error:" << reply->errorString();
            Q_EMIT requestFailed(reply->errorString());
            return;
        }

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();

        if (status < 200 || status >= 300) {
            qCWarning(OPA) << "Claude AI: HTTP error" << status << data;
            Q_EMIT requestFailed(i18n("HTTP %1: %2").arg(status).arg(QString::fromUtf8(data)));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray content = doc[u"content"_s].toArray();
        if (content.isEmpty()) {
            qCWarning(OPA) << "Claude AI: empty content array in response";
            Q_EMIT requestFailed(i18n("Empty response from Claude API"));
            return;
        }

        QString text;
        if (useTools && content.first()[u"type"_s].toString() == u"tool_use"_s) {
            const QJsonObject input = content.first()[u"input"_s].toObject();
            text = QString::fromUtf8(QJsonDocument(input).toJson(QJsonDocument::Compact));
        } else {
            text = content.first()[u"text"_s].toString();
        }

        qCDebug(OPA) << "Claude AI: response received, length=" << text.size();
        Q_EMIT responseReady(text);
    });
}
