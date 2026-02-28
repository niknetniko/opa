/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "model/object_table_model.h"
#include "name_entities.h"
#include "names.h"

class PersonNamesModel : public ObjectTableModel<NameWithOriginEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, SORT, TITLES, GIVEN_NAMES, PREFIX, SURNAME, ORIGIN };
    Q_ENUM(Columns);

    explicit PersonNamesModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
