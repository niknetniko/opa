/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "event_roles_model.h"

#include "core/data_event_broker.h"
#include "database/schema.h"

using namespace Qt::StringLiterals;

EventRolesModel::EventRolesModel(QObject* parent) : DistinctStringModel(parent) {
    this->setTable(Schema::Events::table, u"role"_s);

    connectToTable<Schema::Events>(this);

    reload();
}
