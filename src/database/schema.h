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

struct EventCitations : TableTag {
    static constexpr auto table = QLatin1String("event_citations");
};
inline constexpr auto EventCitationsTable = EventCitations::table;

struct EventRelationCitations : TableTag {
    static constexpr auto table = QLatin1String("event_relation_citations");
};
inline constexpr auto EventRelationCitationsTable = EventRelationCitations::table;

struct NameCitations : TableTag {
    static constexpr auto table = QLatin1String("name_citations");
};
inline constexpr auto NameCitationsTable = NameCitations::table;

struct PersonCitations : TableTag {
    static constexpr auto table = QLatin1String("person_citations");
};
inline constexpr auto PersonCitationsTable = PersonCitations::table;

struct LocationTypes : TableTag {
    static constexpr auto table = QLatin1String("location_types");
};
inline constexpr auto LocationTypesTable = LocationTypes::table;

struct Locations : TableTag {
    static constexpr auto table = QLatin1String("locations");
};
inline constexpr auto LocationsTable = Locations::table;

struct EventRoleTranslations : TableTag {
    static constexpr auto table = QLatin1String("event_role_translations");
};
inline constexpr auto EventRoleTranslationsTable = EventRoleTranslations::table;

struct NameOriginTranslations : TableTag {
    static constexpr auto table = QLatin1String("name_origin_translations");
};
inline constexpr auto NameOriginTranslationsTable = NameOriginTranslations::table;

struct SourceTypes : TableTag {
    static constexpr auto table = QLatin1String("source_types");
};
inline constexpr auto SourceTypesTable = SourceTypes::table;

struct SourceTypeTranslations : TableTag {
    static constexpr auto table = QLatin1String("source_type_translations");
};
inline constexpr auto SourceTypeTranslationsTable = SourceTypeTranslations::table;

struct EventTypeTranslations : TableTag {
    static constexpr auto table = QLatin1String("event_type_translations");
};
inline constexpr auto EventTypeTranslationsTable = EventTypeTranslations::table;

struct LocationTypeTranslations : TableTag {
    static constexpr auto table = QLatin1String("location_type_translations");
};
inline constexpr auto LocationTypeTranslationsTable = LocationTypeTranslations::table;

struct Media : TableTag {
    static constexpr auto table = QLatin1String("media");
};
inline constexpr auto MediaTable = Media::table;

struct PersonMedia : TableTag {
    static constexpr auto table = QLatin1String("person_media");
};
inline constexpr auto PersonMediaTable = PersonMedia::table;

struct NameMedia : TableTag {
    static constexpr auto table = QLatin1String("name_media");
};
inline constexpr auto NameMediaTable = NameMedia::table;

struct EventMedia : TableTag {
    static constexpr auto table = QLatin1String("event_media");
};
inline constexpr auto EventMediaTable = EventMedia::table;

struct EventRelationMedia : TableTag {
    static constexpr auto table = QLatin1String("event_relation_media");
};
inline constexpr auto EventRelationMediaTable = EventRelationMedia::table;

struct SourceMedia : TableTag {
    static constexpr auto table = QLatin1String("source_media");
};
inline constexpr auto SourceMediaTable = SourceMedia::table;

struct LocationMedia : TableTag {
    static constexpr auto table = QLatin1String("location_media");
};
inline constexpr auto LocationMediaTable = LocationMedia::table;

template<typename T>
inline constexpr bool is_table_tag = std::is_base_of_v<TableTag, T>;
}
