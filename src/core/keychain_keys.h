/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QString>

using namespace Qt::StringLiterals;

namespace KeychainKeys {
/**
 * General service name for this application.
 */
constexpr auto Service = "opa"_L1;
/**
 * API key for Claude AI service.
 */
constexpr auto ClaudeApiKey = "claudeApiKey"_L1;
/**
 * API key for OpenAI-compatible AI service.
 */
constexpr auto OpenAiCompatibleApiKey = "openAiCompatibleApiKey"_L1;
}
