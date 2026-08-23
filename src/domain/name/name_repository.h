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
    [[nodiscard]] QStringList findAllSurnames() const;

    [[nodiscard]] QStringList findAllGivenNames() const;

    [[nodiscard]] QList<NameEntity> findNamesForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<NameEntity> findById(IntegerPrimaryKey id) const;

    std::optional<IntegerPrimaryKey> insertName(IntegerPrimaryKey personId, int sort) const;

    bool updateName(
        IntegerPrimaryKey id,
        const QString& titles,
        const QString& givenNames,
        const QString& prefix,
        const QString& surname,
        const QString& note,
        const QString& origin
    ) const;

    bool updateNameSort(IntegerPrimaryKey id, int sort) const;

    bool deleteName(IntegerPrimaryKey id) const;
};
