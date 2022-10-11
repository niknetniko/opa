//
// Created by niko on 3/09/22.
//

#ifndef OPA_PERSON_H
#define OPA_PERSON_H

#include <QString>

namespace Data {
    namespace Person {
        const QString TABLE_NAME = "person";

        namespace Table {
            const QString ID = "id";
            const QString GIVEN_NAMES = "given_names";
            const QString NICK_NAME = "nick_name";
            const QString CALL_NAME = "call_name";
            const QString SUFFIX = "suffix";
        };

        namespace Order {
            const int ID = 0;
            const int GIVEN_NAMES = 1;
            const int NICK_NAME = 2;
            const int CALL_NAME = 3;
            const int SUFFIX = 4;
        }
    }
}


#endif //OPA_PERSON_H
