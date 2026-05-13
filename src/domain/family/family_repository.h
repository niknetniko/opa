/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "database/schema.h"
#include "family_entities.h"

#include <QList>

class FamilyRepository : public BaseRepository {
public:
    [[nodiscard]] QList<FamilyMemberEntity> findFamilyMembersForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<FamilyOverviewRow> findAllFamiliesOverview() const;

    [[nodiscard]] QList<AncestorEntity> findAncestorsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] QList<ParentEntity> findParentsForPerson(IntegerPrimaryKey personId) const;

    [[nodiscard]] std::optional<IntegerPrimaryKey> createFamily();

    bool linkEventToFamily(IntegerPrimaryKey eventId, IntegerPrimaryKey familyId);
};
