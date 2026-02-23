/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QString>
#include <type_traits>

using IntegerPrimaryKey = qlonglong;

namespace Schema {
    struct TableTag {};

    struct People : TableTag {
        static constexpr auto table = QLatin1String("people");
    };
    inline constexpr auto PeopleTable = People::table;

    struct Names : TableTag {
        static constexpr auto table = QLatin1String("names");
    };
    inline constexpr auto NamesTable = Names::table;

    struct NameOrigins : TableTag {
        static constexpr auto table = QLatin1String("name_origins");
    };
    inline constexpr auto NameOriginsTable = NameOrigins::table;

    struct Events : TableTag {
        static constexpr auto table = QLatin1String("events");
    };
    inline constexpr auto EventsTable = Events::table;

    struct EventRoles : TableTag {
        static constexpr auto table = QLatin1String("event_roles");
    };
    inline constexpr auto EventRolesTable = EventRoles::table;

    struct EventTypes : TableTag {
        static constexpr auto table = QLatin1String("event_types");
    };
    inline constexpr auto EventTypesTable = EventTypes::table;

    struct EventRelations : TableTag {
        static constexpr auto table = QLatin1String("event_relations");
    };
    inline constexpr auto EventRelationsTable = EventRelations::table;

    struct Sources : TableTag {
        static constexpr auto table = QLatin1String("sources");
    };
    inline constexpr auto SourcesTable = Sources::table;

    struct Citations : TableTag {
        static constexpr auto table = QLatin1String("citations");
    };
    inline constexpr auto CitationsTable = Citations::table;

    template <typename T>
    inline constexpr bool is_table_tag = std::is_base_of_v<TableTag, T>;
}
