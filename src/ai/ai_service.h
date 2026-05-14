/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QFuture>
#include <QJsonObject>
#include <QObject>
#include <QString>

/**
 * Represents an AI service implementation.
 *
 * Note that in many cases, you can just re-use the OpenAI-compatible API,
 * so no need to have an implementation per provider.
 *
 * Use `createAiService()` to obtain an instance.
 */
class AiService : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

    /**
     * Ask something of the AI provider, which returns a response string.
     *
     * @return A future with the response string, or a failed future with a std::exception.
     */
    virtual QFuture<QString>
    ask(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema = {}) = 0;
};

/**
 * Create an AI service based on the settings and config of the user.
 */
AiService* createAiService(QObject* parent = nullptr);
