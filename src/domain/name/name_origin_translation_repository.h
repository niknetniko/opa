/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "name_origin_translation_entities.h"

#include <QList>
#include <optional>

class NameOriginTranslationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<NameOriginTranslationEntity> findAllForType(IntegerPrimaryKey originId) const;
    std::optional<IntegerPrimaryKey>
    insert(IntegerPrimaryKey originId, const QString& locale, const QString& name) const;
    bool remove(IntegerPrimaryKey id) const;
    [[nodiscard]] std::optional<QString> findByTypeIdAndLocale(IntegerPrimaryKey originId, const QString& locale) const;
    [[nodiscard]] std::optional<QString> findByTypeStringAndLocale(
        const QString& originString, const QString& locale
    ) const;
};
