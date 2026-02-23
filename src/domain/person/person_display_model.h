/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "model/object_table_model.h"
#include "person_entities.h"

class PersonDisplayModel : public ObjectTableModel<PersonDisplayEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, NAME, ROOT };
    Q_ENUM(Columns);

    explicit PersonDisplayModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
