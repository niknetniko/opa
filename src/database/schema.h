//
// Created by niko on 6/02/24.
//

#ifndef OPA_SCHEMA_H
#define OPA_SCHEMA_H

#include <QString>

// TODO: generate this with a script...
namespace Schema {
    namespace People {
        const QString TableName = "people";
        const QString id = "id";
        const QString root = "root";
        const QString sex = "sex";
    }
    namespace Names {
        const QString TableName = "names";
        const QString id = "id";
        const QString personId = "person_id";
        const QString givenNames = "given_names";
        const QString titles = "titles";
        const QString nickname = "nickname";
        const QString prefix = "prefix";
        const QString surname = "surname";
        const QString origin = "origin";
        const QString sortAs = "sort_as";
        const QString main = "main";
    }

    namespace Events {
        const QString TableName = "events";
        const QString id = "id";
        const QString type = "type";
        const QString date = "date";
    }

    namespace EventPeople {
        const QString TableName = "event_people";
        const QString personId = "person_id";
        const QString eventId = "event_id";
        const QString role = "role";
    }

    // TODO: other data...
}

#endif //OPA_SCHEMA_H
