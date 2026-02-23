/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/custom_sql_relational_model.h"
#include "utils/model_utils.h"

#include <KExtraColumnsProxyModel>

/**
 * Create a single display name from the different name parts.
 *
 * Any of the parts can be empty.
 */
QString
construct_display_name(const QString& titles, const QString& givenNames, const QString& prefix, const QString& surname);

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

    const auto toDisplayString = [](const QString& databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };
}

struct NameColumns {
    int titles;
    int givenNames;
    int prefix;
    int surname;
};

class DisplayNameProxyModel : public KExtraColumnsProxyModel {
    Q_OBJECT

public:
    explicit DisplayNameProxyModel(QObject* parent = nullptr);

    void setColumns(NameColumns columns);

    [[nodiscard]] QVariant
    extraColumnData(const QModelIndex& parent, int row, int extraColumn, int role) const override;

private:
    NameColumns columns;
};
