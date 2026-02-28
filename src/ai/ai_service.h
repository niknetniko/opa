/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QObject>
#include <QString>

/**
 * Abstract AI service interface.
 *
 * Use `createAiService()` to obtain an instance.
 */
class AiService : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

    /**
     * Send a request to the AI provider.
     *
     * Results are delivered asynchronously via responseReady() or requestFailed().
     */
    virtual void complete(const QString& systemPrompt, const QString& userMessage) = 0;

Q_SIGNALS:
    void responseReady(const QString& response);
    void requestFailed(const QString& errorMessage);
};

AiService* createAiService(QObject* parent = nullptr);
