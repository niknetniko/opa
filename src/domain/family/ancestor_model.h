/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "family_entities.h"
#include "model/object_table_model.h"

class AncestorModel : public ObjectTableModel<AncestorEntity> {
    Q_OBJECT
public:
    enum Columns { CHILD_ID = 0, FATHER_ID, MOTHER_ID, VISITED, LEVEL, DISPLAY_NAME };
    Q_ENUM(Columns);

    explicit AncestorModel(IntegerPrimaryKey personId, QObject* parent = nullptr);

public Q_SLOTS:
    void reload();

private:
    IntegerPrimaryKey personId;
};
