//
// Created by niko on 3/09/22.
//

#ifndef OPA_PERSON_H
#define OPA_PERSON_H

#include <QString>
#include <KExtraColumnsProxyModel>

namespace Data::Person {
    namespace Table {
        const auto ID = QStringLiteral("id");
        const auto GIVEN_NAMES = QStringLiteral("given_names");
        const auto NICK_NAME = QStringLiteral("nick_name");
        const auto CALL_NAME = QStringLiteral("call_name");
        const auto SUFFIX = QStringLiteral("suffix");
        const auto SEX = QStringLiteral("sex");
    };

    namespace Sex {
        const auto MALE = QStringLiteral("male");
        const auto FEMALE = QStringLiteral("female");
        const auto UNKNOWN = QStringLiteral("unknown");

        QString toDisplay(const QString &sex);

        QString toIcon(const QString &sex);
    }
}

namespace DisplayNameModel {
    constexpr int ID = 0;
    constexpr int NAME = 1;
    constexpr int ROOT = 2;
}

/**
 * Not for direct use.
 *
 * This class adds an additional column to provide a display name.
 */
class DisplayNameProxyModel : public KExtraColumnsProxyModel {
Q_OBJECT

public:
    explicit DisplayNameProxyModel(QObject *parent = nullptr);

    QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role) const override;
};

namespace PersonDetailModel {
    constexpr int ID = 0;
    constexpr int TITLES = 1;
    constexpr int GIVEN_NAMES = 2;
    constexpr int PREFIXES = 3;
    constexpr int SURNAME = 4;
    constexpr int ROOT = 5;
    constexpr int SEX = 6;
    constexpr int DISPLAY_NAME = 7;
};

#endif //OPA_PERSON_H
