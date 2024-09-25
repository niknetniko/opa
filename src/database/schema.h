//
// Created by niko on 6/02/24.
//

#ifndef OPA_SCHEMA_H
#define OPA_SCHEMA_H

#include <QString>

using IntegerPrimaryKey = qlonglong;

// TODO: generate this with a script...
namespace Schema {
    namespace People {
        const QString TableName = QString::fromUtf8("people");
        const QString id = QString::fromUtf8("id");
        const QString root = QString::fromUtf8("root");
        const QString sex = QString::fromUtf8("sex");
    }
    namespace Names {
        const QString TableName = QString::fromUtf8("names");
        const QString id = QString::fromUtf8("id");
        const QString personId = QString::fromUtf8("person_id");
        const QString givenNames = QString::fromUtf8("given_names");
        const QString titles = QString::fromUtf8("titles");
        const QString prefix = QString::fromUtf8("prefix");
        const QString surname = QString::fromUtf8("surname");
        const QString origin = QString::fromUtf8("origin");
        const QString main = QString::fromUtf8("main");
    }

    namespace NameOrigins {
        const QString TableName = QStringLiteral("name_origins");
        const QString id = QStringLiteral("id");
        const QString origin = QStringLiteral("origin");
    }

    namespace Events {
        const QString TableName = QString::fromUtf8("events");
        const QString id = QString::fromUtf8("id");
        const QString type = QString::fromUtf8("type");
        const QString date = QString::fromUtf8("date");
    }

    namespace EventPeople {
        const QString TableName = QString::fromUtf8("event_people");
        const QString personId = QString::fromUtf8("person_id");
        const QString eventId = QString::fromUtf8("event_id");
        const QString role = QString::fromUtf8("role");
    }

    // TODO: other data...
}

#endif //OPA_SCHEMA_H
