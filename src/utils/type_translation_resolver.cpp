/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "type_translation_resolver.h"

TypeTranslationResolver::TypeTranslationResolver(
    std::function<std::optional<QString>(IntegerPrimaryKey typeId, const QString& locale)> lookupFn,
    std::function<QString(const QString&)> builtinTranslator
) :
    lookupFn(std::move(lookupFn)),
    builtinTranslator(std::move(builtinTranslator)) {
}

QString TypeTranslationResolver::resolve(
    const QString& typeString, bool builtin, IntegerPrimaryKey typeId, const QString& locale
) const {
    if (builtin) {
        return builtinTranslator(typeString);
    }

    if (auto result = lookupFn(typeId, locale)) {
        return *result;
    }

    const auto underscore = locale.indexOf(QLatin1Char('_'));
    if (underscore > 0) {
        if (auto result = lookupFn(typeId, locale.left(underscore))) {
            return *result;
        }
    }

    return typeString;
}
