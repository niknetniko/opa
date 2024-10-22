//
// Created by niko on 25/09/24.
//

#ifndef OPA_DATA_NAMES_H
#define OPA_DATA_NAMES_H

#include <QSqlTableModel>
#include <KLazyLocalizedString>

#include "database/schema.h"
#include "utils/custom_sql_relational_model.h"

/**
 * Base model for the names table.
 */
class NamesTableModel : public CustomSqlRelationalModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int PERSON_ID = 1;
    static constexpr int SORT = 2;
    static constexpr int TITLES = 3;
    static constexpr int GIVEN_NAMES = 4;
    static constexpr int PREFIX = 5;
    static constexpr int SURNAME = 6;
    static constexpr int ORIGIN_ID = 7;
    static constexpr int ORIGIN = 8;

    explicit NamesTableModel(QObject *parent, QSqlTableModel *originsModel);
};

// namespace NameOrigins {
//     // TODO: can this be done with less repetition?
//     constexpr auto Unknown = kli18n("Unknown");
//     constexpr auto Inherited = kli18n("Inherited");
//     constexpr auto Patrilineal = kli18n("Patrilineal");
//     constexpr auto Matrilineal = kli18n("Matrilineal");
//     constexpr auto Taken = kli18n("Taken");
//     constexpr auto Patronymic = kli18n("Patronymic");
//     constexpr auto Matronymic = kli18n("Matronymic");
//     constexpr auto Location = kli18n("Location");
//     constexpr auto Occupation = kli18n("Occupation");
//
//     QString asDisplayString(const QString& databaseValue);
// }

namespace NameOrigins {
    Q_NAMESPACE

    enum Values {
        Unknown,
        Inherited,
        Patrilineal,
        Matrilineal,
        Taken,
        Patronymic,
        Matronymic,
        Location,
        Occupation
    };

    Q_ENUM_NS(Values)

    static const QHash<Values, KLazyLocalizedString> nameOriginToString = {
        {Unknown, kli18n("Unknown")},
        {Inherited, kli18n("Inherited")},
        {Patrilineal, kli18n("Patrilineal")},
        {Matrilineal, kli18n("Matrilineal")},
        {Taken, kli18n("Taken")},
        {Patronymic, kli18n("Patronymic")},
        {Matronymic, kli18n("Matronymic")},
        {Location, kli18n("Location")},
        {Occupation, kli18n("Occupation")}
    };

    QString toDisplayString(const QString &databaseValue);
};


class NameOriginTableModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ORIGIN = 1;
    static constexpr int BUILTIN = 2;

    explicit NameOriginTableModel(QObject *parent = nullptr);
};

#endif //OPA_DATA_NAMES_H
