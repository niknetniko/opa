/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "event_entities.h"
#include "model/object_table_model.h"

class EventListModel : public ObjectTableModel<EventDisplayEntity> {
    Q_OBJECT

public:
    enum Columns { ID = 0, TYPE_ID, TYPE, DATE, NAME };
    Q_ENUM(Columns);

    explicit EventListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
