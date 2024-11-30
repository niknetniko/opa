/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/custom_sql_relational_model.h"
#include "utils/model_utils_find_source_model_of_type.h"

/**
 * Create a single display name from the different name parts.
 *
 * Any of the parts can be empty.
 */
QString
construct_display_name(const QString& titles, const QString& givenNames, const QString& prefix, const QString& surname);

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
    static constexpr int NOTE = 8;
    // Extra columns added by the relation.
    static constexpr int ORIGIN = 9;

    explicit NamesTableModel(QObject* parent, QSqlTableModel* originsModel);
};

namespace NameOrigins {
    Q_NAMESPACE

    enum Values { Unknown, Inherited, Patrilineal, Matrilineal, Taken, Patronymic, Matronymic, Location, Occupation };

    Q_ENUM_NS(Values)

    const QHash<Values, KLazyLocalizedString> nameOriginToString = {
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

    const std::function toDisplayString = [](const QString& databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };
}; // namespace NameOrigins


class NameOriginTableModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ORIGIN = 1;
    static constexpr int BUILTIN = 2;

    explicit NameOriginTableModel(QObject* parent = nullptr);
};

namespace PersonNamesModel {
    static constexpr int ID = 0;
    static constexpr int SORT = 1;
    static constexpr int TITLES = 2;
    static constexpr int GIVEN_NAMES = 3;
    static constexpr int PREFIX = 4;
    static constexpr int SURNAME = 5;
    static constexpr int ORIGIN = 6;
}; // namespace PersonNamesModel
