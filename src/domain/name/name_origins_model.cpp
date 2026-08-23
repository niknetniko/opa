/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "name_origins_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"

using namespace Qt::StringLiterals;

NameOriginsModel::NameOriginsModel(QObject* parent) : DistinctStringModel(parent) {
    this->setTable(Schema::Names::table, u"origin"_s);

    connectToTable<Schema::Names>(this);

    reload();
}
