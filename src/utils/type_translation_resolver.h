/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"

#include <QString>
#include <functional>
#include <optional>

/**
 * Resolves the display string for a type (event type, location type, etc.).
 *
 * Resolution order:
 * 1. If builtin: apply the provided translator lambda (kli18n path)
 * 2. Look up the translation via lookupFn for (typeId, locale)
 * 3. If not found, try language-only prefix ("nl_BE" → "nl")
 * 4. Fall back to the raw stored type string
 *
 * lookupFn should query the repository for a translation given a type id and locale,
 * returning nullopt if none exists.
 */
class TypeTranslationResolver {
public:
    TypeTranslationResolver(
        std::function<std::optional<QString>(IntegerPrimaryKey typeId, const QString& locale)> lookupFn,
        std::function<QString(const QString&)> builtinTranslator
    );

    [[nodiscard]] QString resolve(
        const QString& typeString, bool builtin, IntegerPrimaryKey typeId, const QString& locale
    ) const;

private:
    std::function<std::optional<QString>(IntegerPrimaryKey typeId, const QString& locale)> lookupFn;
    std::function<QString(const QString&)> builtinTranslator;
};
