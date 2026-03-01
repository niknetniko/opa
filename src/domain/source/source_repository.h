/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
#include "source_entities.h"

#include <QList>
#include <optional>

class SourceRepository : public BaseRepository {
public:
    [[nodiscard]] QList<SourceEntity> findAll() const;
    [[nodiscard]] std::optional<SourceEntity> findById(IntegerPrimaryKey id) const;
    [[nodiscard]] QStringList findAllTypes() const;
    [[nodiscard]] QList<SourceEntity> findByTitleContaining(const QString& searchTerm) const;

    std::optional<IntegerPrimaryKey> insert(
        const QString& title,
        const QString& type,
        const QString& author,
        const QString& publication,
        const QString& confidence,
        const QString& note,
        std::optional<IntegerPrimaryKey> parentId
    ) const;
    bool update(
        IntegerPrimaryKey id,
        const QString& title,
        const QString& type,
        const QString& author,
        const QString& publication,
        const QString& confidence,
        const QString& note,
        std::optional<IntegerPrimaryKey> parentId
    ) const;
    bool remove(IntegerPrimaryKey id) const;
};
