/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "core/base_repository.h"
#include "event_role_translation_entities.h"

#include <QList>
#include <optional>

class EventRoleTranslationRepository : public BaseRepository {
public:
    [[nodiscard]] QList<EventRoleTranslationEntity> findAllForType(IntegerPrimaryKey roleId) const;
    std::optional<IntegerPrimaryKey> insert(IntegerPrimaryKey roleId, const QString& locale, const QString& name) const;
    bool remove(IntegerPrimaryKey id) const;
    [[nodiscard]] std::optional<QString> findByTypeIdAndLocale(IntegerPrimaryKey roleId, const QString& locale) const;
    [[nodiscard]] std::optional<QString>
    findByTypeStringAndLocale(const QString& roleString, const QString& locale) const;
};
