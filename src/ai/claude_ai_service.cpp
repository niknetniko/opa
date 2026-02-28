/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "claude_ai_service.h"

#include "../core/keychain_keys.h"
#include <qt6keychain/keychain.h>

#include <KLocalizedString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <utility>

using namespace Qt::StringLiterals;

ClaudeAiService::ClaudeAiService(QString model, QObject* parent) :
    AiService(parent),
    model(std::move(model)) {
}

void ClaudeAiService::complete(const QString& systemPrompt, const QString& userMessage) {
    auto* job = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    job->setKey(KeychainKeys::ClaudeApiKey);
    job->setAutoDelete(true);

    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job, systemPrompt, userMessage] {
        if (job->error() != QKeychain::NoError || job->textData().isEmpty()) {
            Q_EMIT requestFailed(i18n("No API key configured. Set one in Preferences → AI."));
            return;
        }
        doRequest(job->textData(), systemPrompt, userMessage);
    });

    job->start();
}

// TODO: This is AI generated, to review later.
void ClaudeAiService::doRequest(const QString& apiKey, const QString& systemPrompt, const QString& userMessage) {
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

    auto* reply = network.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            Q_EMIT requestFailed(reply->errorString());
            return;
        }

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray data = reply->readAll();

        if (status < 200 || status >= 300) {
            Q_EMIT requestFailed(i18n("HTTP %1: %2").arg(status).arg(QString::fromUtf8(data)));
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonArray content = doc[u"content"_s].toArray();
        if (content.isEmpty()) {
            Q_EMIT requestFailed(i18n("Empty response from Claude API"));
            return;
        }

        const QString text = content.first()[u"text"_s].toString();
        Q_EMIT responseReady(text);
    });
}
