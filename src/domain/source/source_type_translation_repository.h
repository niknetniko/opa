/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "source_type_translation_entities.h"

#include <QList>
#include <optional>

class SourceTypeTranslationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<SourceTypeTranslationEntity> findAllForType(IntegerPrimaryKey typeId) const;
    std::optional<IntegerPrimaryKey> insert(IntegerPrimaryKey typeId, const QString& locale, const QString& name) const;
    bool remove(IntegerPrimaryKey id) const;
    [[nodiscard]] std::optional<QString> findByTypeIdAndLocale(IntegerPrimaryKey typeId, const QString& locale) const;
    [[nodiscard]] std::optional<QString> findByTypeStringAndLocale(
        const QString& typeString, const QString& locale
    ) const;
};
