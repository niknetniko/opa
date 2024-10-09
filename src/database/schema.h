//
// Created by niko on 6/02/24.
//

#ifndef OPA_SCHEMA_H
#define OPA_SCHEMA_H

#include <QString>

using IntegerPrimaryKey = qlonglong;

namespace Schema {
    const QString PeopleTable = QStringLiteral("people");
    const QString NamesTable = QStringLiteral("names");
    const QString NameOriginsTable = QStringLiteral("name_origins");
    const QString EventsTable = QStringLiteral("events");
    const QString EventRolesTable = QStringLiteral("event_roles");
    const QString EventTypesTable = QStringLiteral("event_types");
    const QString EventRelationsTable = QStringLiteral("event_relations");
}

#endif //OPA_SCHEMA_H
