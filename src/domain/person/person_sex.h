/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "utils/model_utils.h"

#include <KLazyLocalizedString>
#include <QString>

namespace Sex {
Q_NAMESPACE

enum class Values { Male, Female, Unknown };

Q_ENUM_NS(Values)

const QHash<Values, KLazyLocalizedString> nameOriginToString = {
    {Values::Unknown, kli18n("Unknown")},
    {Values::Male, kli18n("Male")},
    {Values::Female, kli18n("Female")},
};

const auto toDisplayString = [](const QString& databaseValue) {
    return genericToDisplayString<Values>(databaseValue, nameOriginToString);
};

QString toIcon(const QString& sex);
}
