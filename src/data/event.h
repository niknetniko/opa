//
// Created by niko on 3/09/22.
//

#ifndef OPA_PERSON_H
#define OPA_PERSON_H

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

            QString typeToDisplay(QString type) {
                if (type == Type::BIRTH) {
                    return i18n("Birth");
                }
                if (type == Type::DEATH) {
                    return i18n("Death");
                }
                if (type == Type::MARRIAGE) {
                    return i18n("Marriage");
                }
                if (type == Type::DIVORCE) {
                    return i18n("Divorce");
                }
                if (type == Type::BAPTISM) {
                    return i18n("Baptism");
                }
                if (type == Type::CONFIRMATION) {
                    return i18n("Confirmation");
                }
                if (type == Type::FIRST_COMMUNION) {
                    return i18n("First Communion");
                }
                if (type == Type::FUNERAL) {
                    return i18n("Funeral");
                }
                return type;
            }
        };
    }
}


#endif //OPA_PERSON_H
