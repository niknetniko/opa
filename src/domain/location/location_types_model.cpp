/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "location_types_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"

using namespace Qt::StringLiterals;

LocationTypesModel::LocationTypesModel(QObject *parent) : DistinctStringModel(parent) {
    this->setTable(Schema::Locations::table, u"type"_s);

    connectToTable<Schema::Locations>(this);

    reload();
}
