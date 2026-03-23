/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "event_entities.h"
#include "model/object_table_model.h"

class PersonEventListModel : public ObjectTableModel<PersonEventEntity> {
    Q_OBJECT
public:
    enum Columns { ROLE = 0, TYPE, DATE, NAME, ID, ROLE_ID, RELATION_ID };
    Q_ENUM(Columns);

    explicit PersonEventListModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
