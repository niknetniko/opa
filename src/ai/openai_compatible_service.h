/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "ai_service.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QPromise>
#include <QString>
#include <memory>

class OpenAiCompatibleService : public AiService {
    Q_OBJECT

public:
    explicit OpenAiCompatibleService(QString endpoint, QString model, QObject* parent = nullptr);

    QFuture<QString>
    ask(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema = {}) override;

private:
    QString endpoint;
    QString model;
    QNetworkAccessManager network{this};

    void doRequest(
        const QString& apiKey,
        const QString& systemPrompt,
        const QString& userMessage,
        const QJsonObject& schema,
        std::unique_ptr<QPromise<QString>> promise
    );
};
