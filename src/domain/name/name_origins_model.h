/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "model/object_table_model.h"
#include "name_entities.h"

class NameOriginsModel : public ObjectTableModel<NameOriginEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, ORIGIN, BUILTIN };
    Q_ENUM(Columns);

    explicit NameOriginsModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
