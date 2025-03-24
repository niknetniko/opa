/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QString>

using IntegerPrimaryKey = qlonglong;

namespace Schema {
    constexpr auto PeopleTable = QLatin1String("people");
    constexpr auto NamesTable = QLatin1String("names");
    constexpr auto NameOriginsTable = QLatin1String("name_origins");
    constexpr auto EventsTable = QLatin1String("events");
    constexpr auto EventRolesTable = QLatin1String("event_roles");
    constexpr auto EventTypesTable = QLatin1String("event_types");
    constexpr auto EventRelationsTable = QLatin1String("event_relations");
    constexpr auto SourcesTable = QLatin1String("sources");
    constexpr auto CitationsTable = QLatin1String("citations");
}
