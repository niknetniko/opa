/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QString>

using IntegerPrimaryKey = qlonglong;

namespace Schema {
    const auto PeopleTable = QStringLiteral("people");
    const auto NamesTable = QStringLiteral("names");
    const auto NameOriginsTable = QStringLiteral("name_origins");
    const auto EventsTable = QStringLiteral("events");
    const auto EventRolesTable = QStringLiteral("event_roles");
    const auto EventTypesTable = QStringLiteral("event_types");
    const auto EventRelationsTable = QStringLiteral("event_relations");
}
