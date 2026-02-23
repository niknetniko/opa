/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "event_entities.h"
#include "model/object_table_model.h"

class PersonDeathEventsModel : public ObjectTableModel<PersonEventEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, ROLE_ID, ROLE, TYPE, DATE, NAME, DATE_RAW };
    Q_ENUM(Columns);

    explicit PersonDeathEventsModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
