/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "ai_service.h"

#include <QNetworkAccessManager>
#include <QString>

class ClaudeAiService : public AiService {
    Q_OBJECT

public:
    explicit ClaudeAiService(QString model, QObject* parent = nullptr);

    void complete(const QString& systemPrompt, const QString& userMessage) override;

private:
    QString model;
    QNetworkAccessManager network{this};

    void doRequest(const QString& apiKey, const QString& systemPrompt, const QString& userMessage);
};
