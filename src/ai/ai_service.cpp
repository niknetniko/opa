/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ai_service.h"

#include "claude_ai_service.h"
#include "local_ai_service.h"
#include "opaSettings.h"

AiService* createAiService(QObject* parent) {
    switch (opaSettings::aiProvider()) {
        case opaSettings::EnumAiProvider::Claude:
            return new ClaudeAiService(opaSettings::claudeModel(), parent);
        case opaSettings::EnumAiProvider::Local:
            return new LocalAiService(parent);
        case opaSettings::EnumAiProvider::None:
        default:
            return nullptr;
    }
}
