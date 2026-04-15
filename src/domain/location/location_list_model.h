/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationListModel : public ObjectTableModel<LocationEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, NAME, TYPE_ID, PARENT_ID };
    Q_ENUM(Columns);

    explicit LocationListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
