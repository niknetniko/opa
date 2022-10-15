//
// Created by niko on 3/09/22.
//

#ifndef OPA_EVENT_H
#define OPA_EVENT_H

#include <QString>

namespace Data {
    namespace Event {
        const QString TABLE_NAME = "event";

        namespace Type {
            const QString BIRTH = "birth";
            const QString DEATH = "death";
            const QString MARRIAGE = "marriage";
            const QString DIVORCE = "divorce";
            const QString BAPTISM = "baptism";
            const QString CONFIRMATION = "confirmation";
            const QString FIRST_COMMUNION = "first_communion";
            const QString FUNERAL = "funeral";
        }

        namespace Table {
            const QString ID = "id";
            const QString PERSON_ID = "person";
            const QString TYPE = "type";
            const QString DATE = "date";

            QString typeToDisplay(QString type);
        };
    }
}


#endif //OPA_EVENT_H
