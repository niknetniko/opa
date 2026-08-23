/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "source_types_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"

using namespace Qt::StringLiterals;

SourceTypesModel::SourceTypesModel(QObject *parent) : DistinctStringModel(parent) {
    this->setTable(Schema::Sources::table, u"type"_s);

    connectToTable<Schema::Sources>(this);

    reload();
}
