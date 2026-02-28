/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "model/object_table_model.h"
#include "source_entities.h"

class SourcesListModel : public ObjectTableModel<SourceEntity> {
    Q_OBJECT

public:
    enum Columns { ID = 0, TITLE, TYPE, AUTHOR, PUBLICATION, CONFIDENCE, NOTE, PARENT_ID };
    Q_ENUM(Columns)

    explicit SourcesListModel(QObject* parent = nullptr);

public Q_SLOTS:
    void reload();
};
