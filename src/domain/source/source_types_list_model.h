/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "model/object_table_model.h"
#include "source_type_entities.h"

class SourceTypesListModel : public ObjectTableModel<SourceTypeEntity> {
    Q_OBJECT
public:
    enum Columns { ID = 0, TYPE, BUILTIN };
    Q_ENUM(Columns);

    explicit SourceTypesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
