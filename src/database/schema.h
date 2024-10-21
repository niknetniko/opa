//
// Created by niko on 6/02/24.
//

#ifndef OPA_SCHEMA_H
#define OPA_SCHEMA_H

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

#endif //OPA_SCHEMA_H
