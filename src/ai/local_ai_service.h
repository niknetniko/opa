/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "ai_service.h"

#include <KLocalizedString>
#include <QPromise>
#include <exception>
#include <stdexcept>

/**
 * Placeholder for a future local AI service.
 *
 * For now, use the OpenAI compatible API.
 */
class LocalAiService : public AiService {
    Q_OBJECT

public:
    using AiService::AiService;

    QFuture<QString>
    ask(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema = {}) override {
        Q_UNUSED(systemPrompt)
        Q_UNUSED(userMessage)
        Q_UNUSED(schema)
        QPromise<QString> promise;
        promise.start();
        promise.setException(
            std::make_exception_ptr(std::runtime_error(i18n("Local AI is not yet implemented.").toStdString()))
        );
        promise.finish();
        return promise.future();
    }
};
