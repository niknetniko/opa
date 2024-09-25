//
// Created by niko on 3/09/22.
//

#ifndef OPA_PERSON_H
#define OPA_PERSON_H

#include <QString>
#include <KExtraColumnsProxyModel>

namespace Data {
    namespace Person {
        const QString TABLE_NAME = QString::fromUtf8("person");

        namespace Table {
            const QString ID = QString::fromUtf8("id");
            const QString GIVEN_NAMES = QString::fromUtf8("given_names");
            const QString NICK_NAME = QString::fromUtf8("nick_name");
            const QString CALL_NAME = QString::fromUtf8("call_name");
            const QString SUFFIX = QString::fromUtf8("suffix");
            const QString SEX = QString::fromUtf8("sex");
        };

        namespace Sex {
            const QString MALE = QString::fromUtf8("male");
            const QString FEMALE = QString::fromUtf8("female");
            const QString UNKNOWN = QString::fromUtf8("unknown");

            QString toDisplay(const QString &sex);
            QString toIcon(const QString& sex);
        }

        namespace Order {
            const int ID = 0;
            const int GIVEN_NAMES = 1;
            const int NICK_NAME = 2;
            const int CALL_NAME = 3;
            const int SUFFIX = 4;
        }
    }
}

namespace DisplayNameModel {
    const int ID = 0;
    const int NAME = 1;
    const int ROOT = 2;
}

/**
 * Not for direct use.
 *
 * This class adds an additional column to provide a display name.
 */
class DisplayNameProxyModel: public KExtraColumnsProxyModel {
Q_OBJECT
public:
    explicit DisplayNameProxyModel(QObject *parent = nullptr);

    QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const override;
};

#endif //OPA_PERSON_H
