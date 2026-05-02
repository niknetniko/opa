/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "ai_service.h"

#include <KLocalizedString>

/**
 * Placeholder for a future local AI service.
 *
 * For now, use the OpenAI compatible API.
 */
class LocalAiService : public AiService {
    Q_OBJECT

public:
    using AiService::AiService;

    void complete(const QString& systemPrompt, const QString& userMessage, const QJsonObject& schema = {}) override {
        Q_EMIT requestFailed(i18n("Local AI is not yet implemented."));
    }
};
