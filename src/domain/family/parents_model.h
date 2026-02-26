/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "family_entities.h"
#include "model/object_table_model.h"

class ParentsModel : public ObjectTableModel<ParentEntity> {
    Q_OBJECT
public:
    enum Columns { ROLE = 0, PERSON_ID, DISPLAY_NAME };
    Q_ENUM(Columns);

    explicit ParentsModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
