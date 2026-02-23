/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "../../core/base_repository.h"
#include "../../core/query_helper.h"
#include "database/schema.h"
#include "person_entities.h"

#include <QList>
#include <optional>

struct PersonCriteria : QueryHelper::QueryCriteria {
    std::optional<bool> rootOnly;
    std::optional<QString> sex;
};

class PersonRepository : public BaseRepository {
public:
    [[nodiscard]] QList<PersonEntity> findPeople(const PersonCriteria& criteria = {}) const;

    [[nodiscard]] std::optional<PersonEntity> findById(IntegerPrimaryKey id) const;

    [[nodiscard]] QList<PersonDisplayEntity> findPeopleWithPrimaryName(const PersonCriteria& criteria = {}) const;

    [[nodiscard]] std::optional<PersonDisplayEntity> findDisplayById(IntegerPrimaryKey id) const;

    std::optional<IntegerPrimaryKey> insertPerson(const QString& sex, bool root = false) const;

    bool updatePerson(IntegerPrimaryKey id, const QString& sex, bool root) const;

    bool deletePerson(IntegerPrimaryKey id) const;
};
