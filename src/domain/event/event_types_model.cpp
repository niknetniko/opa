/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "event_types_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"

using namespace Qt::StringLiterals;

EventTypesModel::EventTypesModel(QObject* parent) : DistinctStringModel(parent) {
    this->setTable(Schema::Events::table, u"type"_s);

    connectToTable<Schema::Events>(this);

    reload();
}
