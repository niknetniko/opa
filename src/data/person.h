/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <QString>

namespace Sex {
    Q_NAMESPACE

    enum Values { Male, Female, Unknown };

    Q_ENUM_NS(Values)

    const QHash<Values, KLazyLocalizedString> nameOriginToString = {
        {Unknown, kli18n("Unknown")},
        {Male, kli18n("Male")},
        {Female, kli18n("Female")},
    };

    const std::function toDisplayString = [](const QString& databaseValue) {
        return genericToDisplayString<Values>(databaseValue, nameOriginToString);
    };

    QString toIcon(const QString& sex);
}

namespace SexesModel {
    constexpr int SEX = 0;
}

namespace DisplayNameModel {
    constexpr int ID = 0;
    constexpr int NAME = 1;
    constexpr int ROOT = 2;
}

namespace PersonDetailModel {
    constexpr int ID = 0;
    constexpr int TITLES = 1;
    constexpr int GIVEN_NAMES = 2;
    constexpr int PREFIXES = 3;
    constexpr int SURNAME = 4;
    constexpr int ROOT = 5;
    constexpr int SEX = 6;
    constexpr int DISPLAY_NAME = 7;
}

class PeopleTableModel : public QSqlTableModel {
    Q_OBJECT

public:
    static constexpr int ID = 0;
    static constexpr int ROOT = 1;
    static constexpr int SEX = 2;

    explicit PeopleTableModel(QObject* parent = nullptr);
};
