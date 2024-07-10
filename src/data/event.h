//
// Created by niko on 3/09/22.
//

#ifndef OPA_EVENT_H
#define OPA_EVENT_H

#include <QString>

namespace Data {
    namespace Event {
        const QString TABLE_NAME = QString::fromUtf8("event");

        namespace Type {
            const QString BIRTH = QString::fromUtf8("birth");
            const QString DEATH = QString::fromUtf8("death");
            const QString MARRIAGE = QString::fromUtf8("marriage");
            const QString DIVORCE = QString::fromUtf8("divorce");
            const QString BAPTISM = QString::fromUtf8("baptism");
            const QString CONFIRMATION = QString::fromUtf8("confirmation");
            const QString FIRST_COMMUNION = QString::fromUtf8("first_communion");
            const QString FUNERAL = QString::fromUtf8("funeral");
        }

        namespace Table {
            const QString ID = QString::fromUtf8("id");
            const QString PERSON_ID = QString::fromUtf8("person");
            const QString TYPE = QString::fromUtf8("type");
            const QString DATE = QString::fromUtf8("date");

            QString typeToDisplay(QString type);
        };
    }
}


#endif //OPA_EVENT_H
