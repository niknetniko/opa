/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "location_entities.h"
#include "model/object_table_model.h"

class LocationPathsModel : public ObjectTableModel<LocationDisplayEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, FULL_PATH };
    Q_ENUM(Columns);

    explicit LocationPathsModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
