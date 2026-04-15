/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationTypesListModel : public ObjectTableModel<LocationTypeEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, TYPE, BUILTIN };
    Q_ENUM(Columns);

    explicit LocationTypesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
