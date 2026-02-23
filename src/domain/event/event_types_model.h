/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "event_entities.h"
#include "model/object_table_model.h"

class EventTypesListModel : public ObjectTableModel<EventTypeEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, TYPE, BUILTIN };
    Q_ENUM(Columns);

    explicit EventTypesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
