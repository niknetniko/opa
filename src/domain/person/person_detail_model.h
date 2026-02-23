/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "model/object_table_model.h"
#include "person_entities.h"

class PersonDetailModel : public ObjectTableModel<PersonDisplayEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, TITLES, GIVEN_NAMES, PREFIXES, SURNAME, ROOT, SEX, DISPLAY_NAME };
    Q_ENUM(Columns);

    explicit PersonDetailModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
