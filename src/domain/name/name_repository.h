/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
#include "name_entities.h"

#include <QList>
#include <optional>

class NameRepository : public BaseRepository {
public:
    [[nodiscard]] QList<NameOriginEntity> findAllOrigins() const;

    [[nodiscard]] QStringList findAllSurnames() const;

    [[nodiscard]] QStringList findAllGivenNames() const;

    [[nodiscard]] QList<NameEntity> findNamesForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<NameWithOriginEntity> findNamesWithOriginForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<NameEntity> findById(IntegerPrimaryKey id) const;

    [[nodiscard]] std::optional<NameWithOriginEntity> findWithOriginById(IntegerPrimaryKey id) const;

    std::optional<IntegerPrimaryKey> insertName(IntegerPrimaryKey personId, int sort) const;

    bool updateName(
        IntegerPrimaryKey id,
        const QString& titles,
        const QString& givenNames,
        const QString& prefix,
        const QString& surname,
        const QString& note,
        std::optional<IntegerPrimaryKey> originId
    ) const;

    bool updateNameSort(IntegerPrimaryKey id, int sort) const;

    bool deleteName(IntegerPrimaryKey id) const;
};
